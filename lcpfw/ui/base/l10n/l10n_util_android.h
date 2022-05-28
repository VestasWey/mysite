// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_L10N_L10N_UTIL_ANDROID_H_
#define UI_BASE_L10N_L10N_UTIL_ANDROID_H_

#include <jni.h>

#include <string>

#include "base/component_export.h"
#include "base/strings/string16.h"

namespace l10n_util {

COMPONENT_EXPORT(UI_BASE)
base::string16 GetDisplayNameForLocale(const std::string& locale,
                                       const std::string& display_locale);

COMPONENT_EXPORT(UI_BASE) bool IsLayoutRtl();

}  // namespace l10n_util

#endif  // UI_BASE_L10N_L10N_UTIL_ANDROID_H_
