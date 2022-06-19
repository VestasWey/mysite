// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "base/macros.h"
#include "ui/views/app_views_delegate.h"

// Most aura test code is written assuming a single RootWindow view, however,
// at higher levels like content_browsertests and
// views_examples_with_content_exe, we must use the Desktop variants.
class AppDesktopViewsDelegate : public AppViewsDelegate {
public:
    AppDesktopViewsDelegate() = default;
    ~AppDesktopViewsDelegate() override = default;

    // Overridden from ViewsDelegate:
    void OnBeforeWidgetInit(views::Widget::InitParams* params,
        views::internal::NativeWidgetDelegate* delegate) override;

private:
    DISALLOW_COPY_AND_ASSIGN(AppDesktopViewsDelegate);
};
