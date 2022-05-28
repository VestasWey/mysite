// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/display/display_resource_provider.h"

#include <algorithm>
#include <string>

#include "base/atomic_sequence_num.h"
#include "base/numerics/safe_math.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/memory_dump_manager.h"
#include "base/trace_event/trace_event.h"
#include "components/viz/common/resources/resource_sizes.h"
#include "gpu/command_buffer/common/shared_image_trace_utils.h"
#include "ui/gl/trace_util.h"

namespace viz {

namespace {

// Generates process-unique IDs to use for tracing resources.
base::AtomicSequenceNumber g_next_display_resource_provider_tracing_id;

}  // namespace

DisplayResourceProvider::DisplayResourceProvider(Mode mode)
    : mode_(mode),
      tracing_id_(g_next_display_resource_provider_tracing_id.GetNext()) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  // In certain cases, ThreadTaskRunnerHandle isn't set (Android Webview).
  // Don't register a dump provider in these cases.
  // TODO(crbug.com/517156): Get this working in Android Webview.
  if (base::ThreadTaskRunnerHandle::IsSet()) {
    base::trace_event::MemoryDumpManager::GetInstance()->RegisterDumpProvider(
        this, "cc::ResourceProvider", base::ThreadTaskRunnerHandle::Get());
  }
}

DisplayResourceProvider::~DisplayResourceProvider() {
  DCHECK(children_.empty()) << "Destroy() must be called before dtor";

  base::trace_event::MemoryDumpManager::GetInstance()->UnregisterDumpProvider(
      this);
}

void DisplayResourceProvider::Destroy() {
  while (!children_.empty())
    DestroyChildInternal(children_.begin(), FOR_SHUTDOWN);
}

bool DisplayResourceProvider::OnMemoryDump(
    const base::trace_event::MemoryDumpArgs& args,
    base::trace_event::ProcessMemoryDump* pmd) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  for (const auto& resource_entry : resources_) {
    const auto& resource = resource_entry.second;

    bool backing_memory_allocated = false;
    if (resource.transferable.is_software)
      backing_memory_allocated = !!resource.shared_bitmap;
    else
      backing_memory_allocated = resource.gl_id != 0 || resource.image_context;

    if (!backing_memory_allocated) {
      // Don't log unallocated resources - they have no backing memory.
      continue;
    }

    // ResourceIds are not process-unique, so log with the ResourceProvider's
    // unique id.
    std::string dump_name =
        base::StringPrintf("cc/resource_memory/provider_%d/resource_%u",
                           tracing_id_, resource_entry.first.GetUnsafeValue());
    base::trace_event::MemoryAllocatorDump* dump =
        pmd->CreateAllocatorDump(dump_name);

    // Texture resources may not come with a size, in which case don't report
    // one.
    if (!resource.transferable.size.IsEmpty()) {
      uint64_t total_bytes = ResourceSizes::UncheckedSizeInBytesAligned<size_t>(
          resource.transferable.size, resource.transferable.format);
      dump->AddScalar(base::trace_event::MemoryAllocatorDump::kNameSize,
                      base::trace_event::MemoryAllocatorDump::kUnitsBytes,
                      static_cast<uint64_t>(total_bytes));
    }

    // Resources may be shared across processes and require a shared GUID to
    // prevent double counting the memory.

    // The client that owns the resource will use a higher importance (2), and
    // the GPU service will use a lower one (0).
    constexpr int kImportance = 1;

    if (resource.transferable.is_software) {
      pmd->CreateSharedMemoryOwnershipEdge(
          dump->guid(), resource.shared_bitmap_tracing_guid, kImportance);
    } else {
      // Shared ownership edges for legacy mailboxes aren't supported.
      if (!resource.transferable.mailbox_holder.mailbox.IsSharedImage())
        continue;

      auto guid = GetSharedImageGUIDForTracing(
          resource.transferable.mailbox_holder.mailbox);
      pmd->CreateSharedGlobalAllocatorDump(guid);
      pmd->AddOwnershipEdge(dump->guid(), guid, kImportance);
    }
  }

  return true;
}

#if defined(OS_ANDROID)
bool DisplayResourceProvider::IsBackedBySurfaceTexture(ResourceId id) {
  ChildResource* resource = GetResource(id);
  return resource->transferable.is_backed_by_surface_texture;
}

size_t DisplayResourceProvider::CountPromotionHintRequestsForTesting() {
  return wants_promotion_hints_set_.size();
}

void DisplayResourceProvider::InitializePromotionHintRequest(ResourceId id) {
  ChildResource* resource = TryGetResource(id);
  // TODO(ericrk): We should never fail TryGetResource, but we appear to
  // be doing so on Android in rare cases. Handle this gracefully until a
  // better solution can be found. https://crbug.com/811858
  if (!resource)
    return;

  // We could sync all |wants_promotion_hint| resources elsewhere, and send 'no'
  // to all resources that weren't used.  However, there's no real advantage.
  if (resource->transferable.wants_promotion_hint)
    wants_promotion_hints_set_.insert(id);
}
#endif

bool DisplayResourceProvider::DoesResourceWantPromotionHint(
    ResourceId id) const {
#if defined(OS_ANDROID)
  return wants_promotion_hints_set_.count(id) > 0;
#else
  return false;
#endif
}

bool DisplayResourceProvider::DoAnyResourcesWantPromotionHints() const {
#if defined(OS_ANDROID)
  return wants_promotion_hints_set_.size() > 0;
#else
  return false;
#endif
}

bool DisplayResourceProvider::IsOverlayCandidate(ResourceId id) {
  ChildResource* resource = TryGetResource(id);
  // TODO(ericrk): We should never fail TryGetResource, but we appear to
  // be doing so on Android in rare cases. Handle this gracefully until a
  // better solution can be found. https://crbug.com/811858
  return resource && resource->transferable.is_overlay_candidate;
}

bool DisplayResourceProvider::IsResourceSoftwareBacked(ResourceId id) {
  return GetResource(id)->transferable.is_software;
}

gfx::BufferFormat DisplayResourceProvider::GetBufferFormat(ResourceId id) {
  return BufferFormat(GetResourceFormat(id));
}

ResourceFormat DisplayResourceProvider::GetResourceFormat(ResourceId id) {
  ChildResource* resource = GetResource(id);
  return resource->transferable.format;
}

const gfx::ColorSpace& DisplayResourceProvider::GetColorSpace(ResourceId id) {
  ChildResource* resource = GetResource(id);
  return resource->transferable.color_space;
}

int DisplayResourceProvider::CreateChild(ReturnCallback return_callback) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  int child_id = next_child_++;
  Child& child = children_[child_id];
  child.return_callback = std::move(return_callback);

  return child_id;
}

void DisplayResourceProvider::DestroyChild(int child_id) {
  auto it = children_.find(child_id);
  DCHECK(it != children_.end());
  DestroyChildInternal(it, NORMAL);
}

void DisplayResourceProvider::ReceiveFromChild(
    int child_id,
    const std::vector<TransferableResource>& resources) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  // TODO(crbug.com/855785): Fishing for misuse of DisplayResourceProvider
  // causing crashes.
  CHECK(child_id);
  auto child_it = children_.find(child_id);
  // TODO(crbug.com/855785): Fishing for misuse of DisplayResourceProvider
  // causing crashes.
  CHECK(child_it != children_.end());
  Child& child_info = child_it->second;
  DCHECK(!child_info.marked_for_deletion);
  for (const TransferableResource& resource : resources) {
    auto resource_in_map_it = child_info.child_to_parent_map.find(resource.id);
    if (resource_in_map_it != child_info.child_to_parent_map.end()) {
      ChildResource* resource = GetResource(resource_in_map_it->second);
      resource->marked_for_deletion = false;
      resource->imported_count++;
      continue;
    }

    if (resource.is_software != IsSoftware() ||
        resource.mailbox_holder.mailbox.IsZero()) {
      TRACE_EVENT0(
          "viz", "DisplayResourceProvider::ReceiveFromChild dropping invalid");
      child_info.return_callback.Run({resource.ToReturnedResource()});
      continue;
    }

    ResourceId local_id = resource_id_generator_.GenerateNextId();
    DCHECK(!resource.is_software || IsBitmapFormatSupported(resource.format));
    resources_.emplace(local_id, ChildResource(child_id, resource));
    child_info.child_to_parent_map[resource.id] = local_id;
  }
}

void DisplayResourceProvider::DeclareUsedResourcesFromChild(
    int child,
    const ResourceIdSet& resources_from_child) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  // TODO(crbug.com/855785): Fishing for misuse of DisplayResourceProvider
  // causing crashes.
  CHECK(child);
  auto child_it = children_.find(child);
  // TODO(crbug.com/855785): Fishing for misuse of DisplayResourceProvider
  // causing crashes.
  CHECK(child_it != children_.end());
  Child& child_info = child_it->second;
  DCHECK(!child_info.marked_for_deletion);

  std::vector<ResourceId> unused;
  for (auto& entry : child_info.child_to_parent_map) {
    ResourceId local_id = entry.second;
    bool resource_is_in_use = resources_from_child.count(entry.first) > 0;
    if (!resource_is_in_use)
      unused.push_back(local_id);
  }
  DeleteAndReturnUnusedResourcesToChild(child_it, NORMAL, unused);
}

gpu::Mailbox DisplayResourceProvider::GetMailbox(ResourceId resource_id) {
  ChildResource* resource = TryGetResource(resource_id);
  if (!resource)
    return gpu::Mailbox();
  return resource->transferable.mailbox_holder.mailbox;
}

const std::unordered_map<ResourceId, ResourceId, ResourceIdHasher>&
DisplayResourceProvider::GetChildToParentMap(int child) const {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  auto it = children_.find(child);
  DCHECK(it != children_.end());
  DCHECK(!it->second.marked_for_deletion);
  return it->second.child_to_parent_map;
}

bool DisplayResourceProvider::InUse(ResourceId id) {
  ChildResource* resource = GetResource(id);
  return resource->InUse();
}

DisplayResourceProvider::ChildResource* DisplayResourceProvider::GetResource(
    ResourceId id) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  DCHECK(id);
  auto it = resources_.find(id);
  DCHECK(it != resources_.end());
  return &it->second;
}

DisplayResourceProvider::ChildResource* DisplayResourceProvider::TryGetResource(
    ResourceId id) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (!id)
    return nullptr;
  auto it = resources_.find(id);
  if (it == resources_.end())
    return nullptr;
  return &it->second;
}

void DisplayResourceProvider::TryReleaseResource(ResourceId id,
                                                 ChildResource* resource) {
  if (resource->marked_for_deletion && !resource->InUse()) {
    auto child_it = children_.find(resource->child_id);
    DeleteAndReturnUnusedResourcesToChild(child_it, NORMAL, {id});
  }
}

bool DisplayResourceProvider::ReadLockFenceHasPassed(
    const ChildResource* resource) {
  return !resource->read_lock_fence || resource->read_lock_fence->HasPassed();
}

#if defined(OS_ANDROID)
void DisplayResourceProvider::DeletePromotionHint(ResourceMap::iterator it) {
  ChildResource* resource = &it->second;
  // If this resource was interested in promotion hints, then remove it from
  // the set of resources that we'll notify.
  if (resource->transferable.wants_promotion_hint)
    wants_promotion_hints_set_.erase(it->first);
}
#endif

DisplayResourceProvider::CanDeleteNowResult
DisplayResourceProvider::CanDeleteNow(const Child& child_info,
                                      const ChildResource& resource,
                                      DeleteStyle style) {
  if (resource.InUse()) {
    // We can't postpone the deletion, so we'll have to lose it.
    if (style == FOR_SHUTDOWN)
      return CanDeleteNowResult::kYesButLoseResource;

    // Defer this resource deletion.
    return CanDeleteNowResult::kNo;
  } else if (!ReadLockFenceHasPassed(&resource)) {
    // TODO(dcastagna): see if it's possible to use this logic for
    // the branch above too, where the resource is locked or still exported.
    // We can't postpone the deletion, so we'll have to lose it.
    if (style == FOR_SHUTDOWN || child_info.marked_for_deletion)
      return CanDeleteNowResult::kYesButLoseResource;

    // Defer this resource deletion.
    return CanDeleteNowResult::kNo;
  }
  return CanDeleteNowResult::kYes;
}

void DisplayResourceProvider::DeleteAndReturnUnusedResourcesToChild(
    ChildMap::iterator child_it,
    DeleteStyle style,
    const std::vector<ResourceId>& unused) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  DCHECK(child_it != children_.end());
  Child& child_info = child_it->second;

  // No work is done in this case.
  if (unused.empty() && !child_info.marked_for_deletion)
    return;

  // Store unused resources while batching is enabled or we can't access gpu
  // thread right now.
  // TODO(vasilyt): Technically we need to delay only resources with
  // |image_context|.
  if (batch_return_resources_lock_count_ > 0 || !can_access_gpu_thread_) {
    int child_id = child_it->first;
    auto& child_resources = batched_returning_resources_[child_id];
    child_resources.reserve(child_resources.size() + unused.size());
    child_resources.insert(child_resources.end(), unused.begin(), unused.end());
    return;
  }

  std::vector<ReturnedResource> to_return =
      DeleteAndReturnUnusedResourcesToChildImpl(child_info, style, unused);

  if (!to_return.empty())
    child_info.return_callback.Run(to_return);

  if (child_info.marked_for_deletion &&
      child_info.child_to_parent_map.empty()) {
    children_.erase(child_it);
  }
}

void DisplayResourceProvider::DestroyChildInternal(ChildMap::iterator it,
                                                   DeleteStyle style) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  Child& child = it->second;
  DCHECK(style == FOR_SHUTDOWN || !child.marked_for_deletion);

  std::vector<ResourceId> resources_for_child;
  for (auto& entry : child.child_to_parent_map) {
    resources_for_child.push_back(entry.second);
  }

  child.marked_for_deletion = true;

  DeleteAndReturnUnusedResourcesToChild(it, style, resources_for_child);
}

void DisplayResourceProvider::TryFlushBatchedResources() {
  if (batch_return_resources_lock_count_ == 0 && can_access_gpu_thread_) {
    for (auto& child_resources_kv : batched_returning_resources_) {
      auto child_it = children_.find(child_resources_kv.first);

      // Remove duplicates from child's unused resources.  Duplicates are
      // possible when batching is enabled because resources are saved in
      // |batched_returning_resources_| for removal, and not removed from the
      // child's |child_to_parent_map|, so the same set of resources can be
      // saved again using DeclareUsedResourcesForChild() or DestroyChild().
      auto& unused_resources = child_resources_kv.second;
      std::sort(unused_resources.begin(), unused_resources.end());
      auto last = std::unique(unused_resources.begin(), unused_resources.end());
      unused_resources.erase(last, unused_resources.end());

      DeleteAndReturnUnusedResourcesToChild(child_it, NORMAL, unused_resources);
    }
    batched_returning_resources_.clear();
  }
}

void DisplayResourceProvider::SetBatchReturnResources(bool batch) {
  if (batch) {
    DCHECK_GE(batch_return_resources_lock_count_, 0);
    batch_return_resources_lock_count_++;
  } else {
    DCHECK_GT(batch_return_resources_lock_count_, 0);
    batch_return_resources_lock_count_--;
    if (batch_return_resources_lock_count_ == 0) {
      TryFlushBatchedResources();
    }
  }
}

void DisplayResourceProvider::SetAllowAccessToGPUThread(bool allow) {
  can_access_gpu_thread_ = allow;
  if (allow) {
    TryFlushBatchedResources();
  }
}

DisplayResourceProvider::ScopedReadLockSharedImage::ScopedReadLockSharedImage(
    DisplayResourceProvider* resource_provider,
    ResourceId resource_id)
    : resource_provider_(resource_provider),
      resource_id_(resource_id),
      resource_(resource_provider_->GetResource(resource_id_)) {
  DCHECK(resource_);
  DCHECK(resource_->is_gpu_resource_type());
  // Remove this #if defined(OS_WIN), when shared image is used on Windows.
#if !defined(OS_WIN)
  DCHECK(resource_->transferable.mailbox_holder.mailbox.IsSharedImage());
#endif
  resource_->lock_for_overlay_count++;
}

DisplayResourceProvider::ScopedReadLockSharedImage::ScopedReadLockSharedImage(
    ScopedReadLockSharedImage&& other) {
  *this = std::move(other);
}

DisplayResourceProvider::ScopedReadLockSharedImage::
    ~ScopedReadLockSharedImage() {
  Reset();
}

DisplayResourceProvider::ScopedReadLockSharedImage&
DisplayResourceProvider::ScopedReadLockSharedImage::operator=(
    ScopedReadLockSharedImage&& other) {
  Reset();
  resource_provider_ = other.resource_provider_;
  resource_id_ = other.resource_id_;
  resource_ = other.resource_;
  other.resource_provider_ = nullptr;
  other.resource_id_ = kInvalidResourceId;
  other.resource_ = nullptr;
  return *this;
}

void DisplayResourceProvider::ScopedReadLockSharedImage::Reset() {
  if (!resource_provider_)
    return;
  DCHECK(resource_->lock_for_overlay_count);
  resource_->lock_for_overlay_count--;
  resource_provider_->TryReleaseResource(resource_id_, resource_);
  resource_provider_ = nullptr;
  resource_id_ = kInvalidResourceId;
  resource_ = nullptr;
}

DisplayResourceProvider::ScopedBatchReturnResources::ScopedBatchReturnResources(
    DisplayResourceProvider* resource_provider,
    bool allow_access_to_gpu_thread)
    : resource_provider_(resource_provider),
      was_access_to_gpu_thread_allowed_(
          resource_provider_->can_access_gpu_thread_) {
  resource_provider_->SetBatchReturnResources(true);
  if (allow_access_to_gpu_thread)
    resource_provider_->SetAllowAccessToGPUThread(true);
}

DisplayResourceProvider::ScopedBatchReturnResources::
    ~ScopedBatchReturnResources() {
  resource_provider_->SetBatchReturnResources(false);
  resource_provider_->SetAllowAccessToGPUThread(
      was_access_to_gpu_thread_allowed_);
}

DisplayResourceProvider::Child::Child() = default;
DisplayResourceProvider::Child::Child(Child&& other) = default;
DisplayResourceProvider::Child& DisplayResourceProvider::Child::operator=(
    Child&& other) = default;
DisplayResourceProvider::Child::~Child() = default;

DisplayResourceProvider::ChildResource::ChildResource(
    int child_id,
    const TransferableResource& transferable)
    : child_id(child_id), transferable(transferable), filter(GL_NONE) {
  if (is_gpu_resource_type())
    UpdateSyncToken(transferable.mailbox_holder.sync_token);
  else
    SetSynchronized();
}

DisplayResourceProvider::ChildResource::ChildResource(ChildResource&& other) =
    default;
DisplayResourceProvider::ChildResource::~ChildResource() = default;

void DisplayResourceProvider::ChildResource::SetLocallyUsed() {
  synchronization_state_ = LOCALLY_USED;
  sync_token_.Clear();
}

void DisplayResourceProvider::ChildResource::SetSynchronized() {
  synchronization_state_ = SYNCHRONIZED;
}

void DisplayResourceProvider::ChildResource::UpdateSyncToken(
    const gpu::SyncToken& sync_token) {
  DCHECK(is_gpu_resource_type());
  // An empty sync token may be used if commands are guaranteed to have run on
  // the gpu process or in case of context loss.
  sync_token_ = sync_token;
  synchronization_state_ = sync_token.HasData() ? NEEDS_WAIT : SYNCHRONIZED;
}

}  // namespace viz
