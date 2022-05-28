// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/ax_active_popup.h"

#include "base/macros.h"
#include "base/no_destructor.h"

namespace ui {

namespace {

base::Optional<AXNodeID>& GetActivePopupAXUniqueIdInstance() {
  // Keeps track of the unique ID that can be used to retrieve the
  // `ViewAccessibility` object that is handling the accessibility for the
  // currently active autofill popup. This singleton is used for communicating
  // the live status of the autofill popup between web contents and Views. The
  // assumption here is that only one autofill popup can exist at a time.
  static base::NoDestructor<base::Optional<AXNodeID>> active_popup_ax_unique_id;
  return *active_popup_ax_unique_id;
}

}  // namespace

base::Optional<AXNodeID> GetActivePopupAxUniqueId() {
  return GetActivePopupAXUniqueIdInstance();
}

void SetActivePopupAxUniqueId(base::Optional<AXNodeID> ax_unique_id) {
  // When an instance of autofill popup hides, the caller of popup hide should
  // make sure active_popup_ax_unique_id is cleared. The assumption is that
  // there can only be one active autofill popup existing at a time. If on
  // popup showing, we encounter active_popup_ax_unique_id is already set,
  // this would indicate two autofill popups are showing at the same time or
  // previous on popup hide call did not clear the variable, so we should fail
  // via DCHECK here.
  DCHECK(!GetActivePopupAXUniqueIdInstance());

  GetActivePopupAXUniqueIdInstance() = ax_unique_id;
}

void ClearActivePopupAxUniqueId() {
  GetActivePopupAXUniqueIdInstance() = base::nullopt;
}

}  // namespace ui
