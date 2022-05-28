// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_ANDROID_COLOR_HELPERS_H_
#define UI_ANDROID_COLOR_HELPERS_H_

#include <stdint.h>

#include <limits>
#include <string>

#include "base/optional.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/android/ui_android_export.h"

namespace ui {

UI_ANDROID_EXPORT constexpr int64_t kInvalidJavaColor =
    static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1;

// Converts |color| to a CSS color string. If |color| is null, the empty string
// is returned.
UI_ANDROID_EXPORT std::string OptionalSkColorToString(
    const base::Optional<SkColor>& color);

// Conversions between a Java color and an Optional<SkColor>. Java colors are
// represented as 64-bit signed integers. Valid colors are in the range
// [std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()].
// while |kInvalidJavaColor| is reserved for representing a null/unset color.
UI_ANDROID_EXPORT int64_t
OptionalSkColorToJavaColor(const base::Optional<SkColor>& skcolor);
UI_ANDROID_EXPORT base::Optional<SkColor> JavaColorToOptionalSkColor(
    int64_t java_color);

}  // namespace ui

#endif  // UI_ANDROID_COLOR_HELPERS_H_
