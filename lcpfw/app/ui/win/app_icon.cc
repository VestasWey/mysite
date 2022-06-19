// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/win/app_icon.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/image/image_family.h"

#include "common/app_constants.h"
#include "resources/win/main_dll_resource.h"

namespace {

// Returns the resource id of the application icon.
int GetAppIconResourceId() {
  return IDR_MAINFRAME;
}

}  // namespace

HICON GetAppIcon() {
  // TODO(mgiuca): Use GetAppIconImageFamily/CreateExact instead of LoadIcon, to
  // get correct scaling. (See http://crbug.com/551256)
  const int icon_id = GetAppIconResourceId();
  // HICON returned from LoadIcon do not leak and do not have to be destroyed.
  return LoadIcon(GetModuleHandle(lcpfw::kAppMainDll),
                  MAKEINTRESOURCE(icon_id));
}

HICON GetSmallAppIcon() {
  // TODO(mgiuca): Use GetAppIconImageFamily/CreateExact instead of LoadIcon, to
  // get correct scaling. (See http://crbug.com/551256)
  const int icon_id = GetAppIconResourceId();
  gfx::Size size = GetSmallAppIconSize();
  // HICON returned from LoadImage must be released using DestroyIcon.
  return static_cast<HICON>(LoadImage(
      GetModuleHandle(lcpfw::kAppMainDll), MAKEINTRESOURCE(icon_id),
      IMAGE_ICON, size.width(), size.height(), LR_DEFAULTCOLOR | LR_SHARED));
}

gfx::Size GetAppIconSize() {
  return gfx::Size(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
}

gfx::Size GetSmallAppIconSize() {
  return gfx::Size(GetSystemMetrics(SM_CXSMICON),
                   GetSystemMetrics(SM_CYSMICON));
}

std::unique_ptr<gfx::ImageFamily> GetAppIconImageFamily() {
  const int icon_id = GetAppIconResourceId();
  // Get the icon from chrome.dll (not chrome.exe, which has different resource
  // IDs). If chrome.dll is not loaded, we are probably in a unit test, so fall
  // back to getting the icon from the current module (assuming it is
  // unit_tests.exe, that has the same resource IDs as chrome.dll).
  HMODULE module = GetModuleHandle(lcpfw::kAppMainDll);
  if (!module)
    module = GetModuleHandle(nullptr);
  DCHECK(module);

  return IconUtil::CreateImageFamilyFromIconResource(module, icon_id);
}
