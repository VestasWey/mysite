// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/app_views_delegate.h"

#include "build/build_config.h"
#include "ui/views/buildflags.h"

#include <dwmapi.h>
#include "base/win/windows_version.h"

#if BUILDFLAG(ENABLE_DESKTOP_AURA)
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#endif  // BUILDFLAG(ENABLE_DESKTOP_AURA)

#include "ui/win/app_icon.h"


AppViewsDelegate::AppViewsDelegate() = default;

AppViewsDelegate::~AppViewsDelegate() = default;

HICON AppViewsDelegate::GetDefaultWindowIcon() const
{
    return GetAppIcon();
}

HICON AppViewsDelegate::GetSmallWindowIcon() const
{
    return GetSmallAppIcon();
}

void AppViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate)
{
    if (params->opacity == views::Widget::InitParams::WindowOpacity::kInferred)
    {
        params->opacity = use_transparent_windows_
            ? views::Widget::InitParams::WindowOpacity::kTranslucent
            : views::Widget::InitParams::WindowOpacity::kOpaque;
    }
#if BUILDFLAG(ENABLE_DESKTOP_AURA)
    if (!params->native_widget && use_desktop_native_widgets_)
        params->native_widget = new views::DesktopNativeWidgetAura(delegate);
#endif  // BUILDFLAG(ENABLE_DESKTOP_AURA)
}
