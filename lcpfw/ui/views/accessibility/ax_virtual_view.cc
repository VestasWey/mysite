// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/accessibility/ax_virtual_view.h"

#include <stdint.h>

#include <algorithm>
#include <map>
#include <utility>

#include "base/callback.h"
#include "base/containers/adapters.h"
#include "base/no_destructor.h"
#include "build/build_config.h"
#include "ui/accessibility/ax_action_data.h"
#include "ui/accessibility/ax_tree_data.h"
#include "ui/accessibility/platform/ax_platform_node.h"
#include "ui/base/layout.h"
#include "ui/base/ui_base_types.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/accessibility/view_ax_platform_node_delegate.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#if OS_WIN
#include "ui/views/win/hwnd_util.h"
#endif

namespace views {

// Tracks all virtual ax views.
std::map<int32_t, AXVirtualView*>& GetIdMap() {
  static base::NoDestructor<std::map<int32_t, AXVirtualView*>> id_to_obj_map;
  return *id_to_obj_map;
}

// static
const char AXVirtualView::kViewClassName[] = "AXVirtualView";

// static
AXVirtualView* AXVirtualView::GetFromId(int32_t id) {
  auto& id_map = GetIdMap();
  const auto& it = id_map.find(id);
  return it != id_map.end() ? it->second : nullptr;
}

AXVirtualView::AXVirtualView() {
  GetIdMap()[unique_id_.Get()] = this;
  ax_platform_node_ = ui::AXPlatformNode::Create(this);
  DCHECK(ax_platform_node_);
  custom_data_.AddStringAttribute(ax::mojom::StringAttribute::kClassName,
                                  GetViewClassName());
}

AXVirtualView::~AXVirtualView() {
  GetIdMap().erase(unique_id_.Get());
  DCHECK(!parent_view_ || !virtual_parent_view_)
      << "Either |parent_view_| or |virtual_parent_view_| could be set but "
         "not both.";

  if (ax_platform_node_) {
    ax_platform_node_->Destroy();
    ax_platform_node_ = nullptr;
  }
}

void AXVirtualView::AddChildView(std::unique_ptr<AXVirtualView> view) {
  DCHECK(view);
  if (view->virtual_parent_view_ == this)
    return;  // Already a child of this virtual view.
  AddChildViewAt(std::move(view), int{(int)children_.size()});
}

void AXVirtualView::AddChildViewAt(std::unique_ptr<AXVirtualView> view,
                                   int index) {
  DCHECK(view);
  CHECK_NE(view.get(), this)
      << "You cannot add an AXVirtualView as its own child.";
  DCHECK(!view->parent_view_) << "This |view| already has a View "
                                 "parent. Call RemoveVirtualChildView first.";
  DCHECK(!view->virtual_parent_view_) << "This |view| already has an "
                                         "AXVirtualView parent. Call "
                                         "RemoveChildView first.";
  DCHECK_GE(index, 0);
  DCHECK_LE(index, int{(int)children_.size()});

  view->virtual_parent_view_ = this;
  children_.insert(children_.begin() + index, std::move(view));
  if (GetOwnerView()) {
    GetOwnerView()->NotifyAccessibilityEvent(ax::mojom::Event::kChildrenChanged,
                                             true);
  }
}

void AXVirtualView::ReorderChildView(AXVirtualView* view, int index) {
  DCHECK(view);
  if (index >= int{(int)children_.size()})
    return;
  if (index < 0)
    index = int{ (int)children_.size()} - 1;

  DCHECK_EQ(view->virtual_parent_view_, this);
  if (children_[index].get() == view)
    return;

  int cur_index = GetIndexOf(view);
  if (cur_index < 0)
    return;

  std::unique_ptr<AXVirtualView> child = std::move(children_[cur_index]);
  children_.erase(children_.begin() + cur_index);
  children_.insert(children_.begin() + index, std::move(child));

  GetOwnerView()->NotifyAccessibilityEvent(ax::mojom::Event::kChildrenChanged,
                                           true);
}

std::unique_ptr<AXVirtualView> AXVirtualView::RemoveFromParentView() {
  if (parent_view_)
    return parent_view_->RemoveVirtualChildView(this);

  if (virtual_parent_view_)
    return virtual_parent_view_->RemoveChildView(this);

  // This virtual view hasn't been added to a parent view yet.
  NOTREACHED() << "Cannot remove from parent view if there is no parent.";
  return {};
}

std::unique_ptr<AXVirtualView> AXVirtualView::RemoveChildView(
    AXVirtualView* view) {
  DCHECK(view);
  int cur_index = GetIndexOf(view);
  if (cur_index < 0)
    return {};

  bool focus_changed = false;
  if (GetOwnerView()) {
    ViewAccessibility& view_accessibility =
        GetOwnerView()->GetViewAccessibility();
    if (view_accessibility.FocusedVirtualChild() &&
        Contains(view_accessibility.FocusedVirtualChild())) {
      focus_changed = true;
    }
  }

  std::unique_ptr<AXVirtualView> child = std::move(children_[cur_index]);
  children_.erase(children_.begin() + cur_index);
  child->virtual_parent_view_ = nullptr;
  child->populate_data_callback_.Reset();

  if (GetOwnerView()) {
    if (focus_changed)
      GetOwnerView()->GetViewAccessibility().OverrideFocus(nullptr);
    GetOwnerView()->NotifyAccessibilityEvent(ax::mojom::Event::kChildrenChanged,
                                             true);
  }

  return child;
}

void AXVirtualView::RemoveAllChildViews() {
  while (!children_.empty())
    RemoveChildView(children_.back().get());
}

bool AXVirtualView::Contains(const AXVirtualView* view) const {
  DCHECK(view);
  for (const AXVirtualView* v = view; v; v = v->virtual_parent_view_) {
    if (v == this)
      return true;
  }
  return false;
}

int AXVirtualView::GetIndexOf(const AXVirtualView* view) const {
  DCHECK(view);
  const auto iter =
      std::find_if(children_.begin(), children_.end(),
                   [view](const auto& child) { return child.get() == view; });
  return iter != children_.end() ? static_cast<int>(iter - children_.begin())
                                 : -1;
}

const char* AXVirtualView::GetViewClassName() const {
  return kViewClassName;
}

gfx::NativeViewAccessible AXVirtualView::GetNativeObject() const {
  DCHECK(ax_platform_node_);
  return ax_platform_node_->GetNativeViewAccessible();
}

void AXVirtualView::NotifyAccessibilityEvent(ax::mojom::Event event_type) {
  DCHECK(ax_platform_node_);
  if (GetOwnerView()) {
    const ViewAccessibility::AccessibilityEventsCallback& events_callback =
        GetOwnerView()->GetViewAccessibility().accessibility_events_callback();
    if (events_callback)
      events_callback.Run(this, event_type);
  }
  ax_platform_node_->NotifyAccessibilityEvent(event_type);
}

ui::AXNodeData& AXVirtualView::GetCustomData() {
  return custom_data_;
}

void AXVirtualView::SetPopulateDataCallback(
    base::RepeatingCallback<void(ui::AXNodeData*)> callback) {
  populate_data_callback_ = std::move(callback);
}

void AXVirtualView::UnsetPopulateDataCallback() {
  populate_data_callback_.Reset();
}

// ui::AXPlatformNodeDelegate

const ui::AXNodeData& AXVirtualView::GetData() const {
  // Make a copy of our |custom_data_| so that any modifications will not be
  // made to the data that users of this class will be manipulating.
  static ui::AXNodeData node_data;
  node_data = custom_data_;

  node_data.id = GetUniqueId().Get();

  if (!GetOwnerView() || !GetOwnerView()->GetEnabled())
    node_data.SetRestriction(ax::mojom::Restriction::kDisabled);

  if (!GetOwnerView() || !GetOwnerView()->IsDrawn())
    node_data.AddState(ax::mojom::State::kInvisible);

  if (GetOwnerView() && GetOwnerView()->context_menu_controller())
    node_data.AddAction(ax::mojom::Action::kShowContextMenu);

  if (populate_data_callback_ && GetOwnerView())
    populate_data_callback_.Run(&node_data);

  // According to the ARIA spec, the node should not be ignored if it is
  // focusable. This is to ensure that the focusable node is both understandable
  // and operable.
  if (node_data.HasState(ax::mojom::State::kIgnored) &&
      node_data.HasState(ax::mojom::State::kFocusable)) {
    node_data.RemoveState(ax::mojom::State::kIgnored);
  }

  return node_data;
}

int AXVirtualView::GetChildCount() const {
  int count = 0;
  for (const std::unique_ptr<AXVirtualView>& child : children_) {
    if (child->IsIgnored()) {
      count += child->GetChildCount();
    } else {
      ++count;
    }
  }
  return count;
}

gfx::NativeViewAccessible AXVirtualView::ChildAtIndex(int index) {
  DCHECK_GE(index, 0) << "|index| should be greater or equal to 0.";
  DCHECK_LT(index, GetChildCount())
      << "|index| should be less than the child count.";

  for (const std::unique_ptr<AXVirtualView>& child : children_) {
    if (child->IsIgnored()) {
      int child_count = child->GetChildCount();
      if (index < child_count)
        return child->ChildAtIndex(index);
      index -= child_count;
    } else {
      if (index == 0)
        return child->GetNativeObject();
      --index;
    }

    DCHECK_GE(index, 0) << "|index| should be less than the child count.";
  }

  NOTREACHED() << "|index| should be less than the child count.";
  return nullptr;
}

#if !defined(OS_APPLE)
gfx::NativeViewAccessible AXVirtualView::GetNSWindow() {
  NOTREACHED();
  return nullptr;
}
#endif

gfx::NativeViewAccessible AXVirtualView::GetNativeViewAccessible() {
  return GetNativeObject();
}

gfx::NativeViewAccessible AXVirtualView::GetParent() {
  if (parent_view_) {
    if (!parent_view_->IsIgnored())
      return parent_view_->GetNativeObject();
    return GetDelegate()->GetParent();
  }

  if (virtual_parent_view_) {
    if (virtual_parent_view_->IsIgnored())
      return virtual_parent_view_->GetParent();
    return virtual_parent_view_->GetNativeObject();
  }

  // This virtual view hasn't been added to a parent view yet.
  return nullptr;
}

gfx::Rect AXVirtualView::GetBoundsRect(
    const ui::AXCoordinateSystem coordinate_system,
    const ui::AXClippingBehavior clipping_behavior,
    ui::AXOffscreenResult* offscreen_result) const {
  // We could optionally add clipping here if ever needed.
  // TODO(nektar): Implement bounds that are relative to the parent.
  gfx::Rect bounds = gfx::ToEnclosingRect(GetData().relative_bounds.bounds);
  View* owner_view = GetOwnerView();
  if (owner_view && owner_view->GetWidget())
    View::ConvertRectToScreen(owner_view, &bounds);
  switch (coordinate_system) {
    case ui::AXCoordinateSystem::kScreenDIPs:
      return bounds;
    case ui::AXCoordinateSystem::kScreenPhysicalPixels: {
      float scale_factor = 1.0;
      if (owner_view && owner_view->GetWidget()) {
        gfx::NativeView native_view = owner_view->GetWidget()->GetNativeView();
        if (native_view)
          scale_factor = ui::GetScaleFactorForNativeView(native_view);
      }
      return gfx::ScaleToEnclosingRect(bounds, scale_factor);
    }
    case ui::AXCoordinateSystem::kRootFrame:
    case ui::AXCoordinateSystem::kFrame:
      NOTIMPLEMENTED();
      return gfx::Rect();
  }
}

gfx::NativeViewAccessible AXVirtualView::HitTestSync(
    int screen_physical_pixel_x,
    int screen_physical_pixel_y) const {
  const ui::AXNodeData& node_data = GetData();
  if (node_data.HasState(ax::mojom::State::kInvisible))
    return nullptr;

  // Check if the point is within any of the virtual children of this view.
  // AXVirtualView's HitTestSync is a recursive function that will return the
  // deepest child, since it does not support relative bounds.
  // Search the greater indices first, since they're on top in the z-order.
  for (const std::unique_ptr<AXVirtualView>& child :
       base::Reversed(children_)) {
    gfx::NativeViewAccessible result =
        child->HitTestSync(screen_physical_pixel_x, screen_physical_pixel_y);
    if (result)
      return result;
  }

  // If it's not inside any of our virtual children, and it's inside the bounds
  // of this virtual view, then it's inside this virtual view.
  gfx::Rect bounds_in_screen_physical_pixels =
      GetBoundsRect(ui::AXCoordinateSystem::kScreenPhysicalPixels,
                    ui::AXClippingBehavior::kUnclipped);
  if (bounds_in_screen_physical_pixels.Contains(
          static_cast<float>(screen_physical_pixel_x),
          static_cast<float>(screen_physical_pixel_y)) &&
      !node_data.IsIgnored()) {
    return GetNativeObject();
  }

  return nullptr;
}

gfx::NativeViewAccessible AXVirtualView::GetFocus() const {
  View* owner_view = GetOwnerView();
  if (owner_view) {
    if (!(owner_view->HasFocus())) {
      return nullptr;
    }
    return owner_view->GetViewAccessibility().GetFocusedDescendant();
  }

  // This virtual view hasn't been added to a parent view yet.
  return nullptr;
}

ui::AXPlatformNode* AXVirtualView::GetFromNodeID(int32_t id) {
  AXVirtualView* virtual_view = GetFromId(id);
  if (virtual_view) {
    return virtual_view->ax_platform_node();
  }
  return nullptr;
}

bool AXVirtualView::AccessibilityPerformAction(const ui::AXActionData& data) {
  bool result = false;
  if (custom_data_.HasAction(data.action))
    result = HandleAccessibleAction(data);
  if (!result && GetOwnerView())
    return HandleAccessibleActionInOwnerView(data);
  return result;
}

bool AXVirtualView::ShouldIgnoreHoveredStateForTesting() {
  // TODO(nektar): Implement.
  return false;
}

bool AXVirtualView::IsOffscreen() const {
  // TODO(nektar): Implement.
  return false;
}

const ui::AXUniqueId& AXVirtualView::GetUniqueId() const {
  return unique_id_;
}

// Virtual views need to implement this function in order for accessibility
// events to be routed correctly.
gfx::AcceleratedWidget AXVirtualView::GetTargetForNativeAccessibilityEvent() {
#if defined(OS_WIN)
  if (GetOwnerView())
    return HWNDForView(GetOwnerView());
#endif
  return gfx::kNullAcceleratedWidget;
}

base::Optional<bool> AXVirtualView::GetTableHasColumnOrRowHeaderNode() const {
  return GetDelegate()->GetTableHasColumnOrRowHeaderNode();
}

std::vector<int32_t> AXVirtualView::GetColHeaderNodeIds() const {
  return GetDelegate()->GetColHeaderNodeIds();
}

std::vector<int32_t> AXVirtualView::GetColHeaderNodeIds(int col_index) const {
  return GetDelegate()->GetColHeaderNodeIds(col_index);
}

base::Optional<int32_t> AXVirtualView::GetCellId(int row_index,
                                                 int col_index) const {
  return GetDelegate()->GetCellId(row_index, col_index);
}

bool AXVirtualView::IsIgnored() const {
  return GetData().IsIgnored();
}

bool AXVirtualView::HandleAccessibleAction(
    const ui::AXActionData& action_data) {
  if (!GetOwnerView())
    return false;

  switch (action_data.action) {
    case ax::mojom::Action::kShowContextMenu: {
      const gfx::Rect screen_bounds = GetBoundsRect(
          ui::AXCoordinateSystem::kScreenDIPs, ui::AXClippingBehavior::kClipped,
          nullptr /* offscreen_result */);
      if (!screen_bounds.IsEmpty()) {
        GetOwnerView()->ShowContextMenu(screen_bounds.CenterPoint(),
                                        ui::MENU_SOURCE_KEYBOARD);
        return true;
      }
      break;
    }

    default:
      break;
  }

  return HandleAccessibleActionInOwnerView(action_data);
}

bool AXVirtualView::HandleAccessibleActionInOwnerView(
    const ui::AXActionData& action_data) {
  DCHECK(GetOwnerView());
  // Save the node id so that the owner view can determine which virtual view
  // is being targeted for action.
  ui::AXActionData forwarded_action_data = action_data;
  forwarded_action_data.target_node_id = GetData().id;
  return GetOwnerView()->HandleAccessibleAction(forwarded_action_data);
}

View* AXVirtualView::GetOwnerView() const {
  if (parent_view_)
    return parent_view_->view();

  if (virtual_parent_view_)
    return virtual_parent_view_->GetOwnerView();

  // This virtual view hasn't been added to a parent view yet.
  return nullptr;
}

ViewAXPlatformNodeDelegate* AXVirtualView::GetDelegate() const {
  DCHECK(GetOwnerView());
  return static_cast<ViewAXPlatformNodeDelegate*>(
      &GetOwnerView()->GetViewAccessibility());
}

AXVirtualViewWrapper* AXVirtualView::GetOrCreateWrapper(
    views::AXAuraObjCache* cache) {
#if defined(USE_AURA)
  // cache might be recreated, and if cache is new, recreate the wrapper.
  if (!wrapper_ || wrapper_->cache() != cache)
    wrapper_ = std::make_unique<AXVirtualViewWrapper>(this, cache);
#endif
  return wrapper_.get();
}

}  // namespace views
