// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines all the command-line switches used by ui/base.

#ifndef UI_BASE_UI_BASE_SWITCHES_H_
#define UI_BASE_UI_BASE_SWITCHES_H_

#include "base/component_export.h"
#include "build/build_config.h"

namespace switches {

#if defined(OS_MAC)
COMPONENT_EXPORT(UI_BASE) extern const char kDisableAVFoundationOverlays[];
COMPONENT_EXPORT(UI_BASE) extern const char kDisableMacOverlays[];
COMPONENT_EXPORT(UI_BASE) extern const char kDisableModalAnimations[];
COMPONENT_EXPORT(UI_BASE) extern const char kDisableRemoteCoreAnimation[];
COMPONENT_EXPORT(UI_BASE) extern const char kShowMacOverlayBorders[];
#endif

COMPONENT_EXPORT(UI_BASE) extern const char kDisableCompositedAntialiasing[];
COMPONENT_EXPORT(UI_BASE) extern const char kDisableDwmComposition[];
COMPONENT_EXPORT(UI_BASE) extern const char kDisableTouchDragDrop[];
COMPONENT_EXPORT(UI_BASE) extern const char kEnableTouchDragDrop[];
COMPONENT_EXPORT(UI_BASE) extern const char kForceCaptionStyle[];
COMPONENT_EXPORT(UI_BASE) extern const char kForceDarkMode[];
COMPONENT_EXPORT(UI_BASE) extern const char kForceHighContrast[];
COMPONENT_EXPORT(UI_BASE) extern const char kLang[];
COMPONENT_EXPORT(UI_BASE) extern const char kShowOverdrawFeedback[];
COMPONENT_EXPORT(UI_BASE) extern const char kSlowDownCompositingScaleFactor[];
COMPONENT_EXPORT(UI_BASE) extern const char kTintCompositedContent[];
COMPONENT_EXPORT(UI_BASE) extern const char kTopChromeTouchUi[];
COMPONENT_EXPORT(UI_BASE) extern const char kTopChromeTouchUiAuto[];
COMPONENT_EXPORT(UI_BASE) extern const char kTopChromeTouchUiDisabled[];
COMPONENT_EXPORT(UI_BASE) extern const char kTopChromeTouchUiEnabled[];
COMPONENT_EXPORT(UI_BASE) extern const char kUIDisablePartialSwap[];
COMPONENT_EXPORT(UI_BASE) extern const char kUseSystemClipboard[];

// Test related.
COMPONENT_EXPORT(UI_BASE) extern const char kDisallowNonExactResourceReuse[];
COMPONENT_EXPORT(UI_BASE) extern const char kMangleLocalizedStrings[];

}  // namespace switches

#endif  // UI_BASE_UI_BASE_SWITCHES_H_
