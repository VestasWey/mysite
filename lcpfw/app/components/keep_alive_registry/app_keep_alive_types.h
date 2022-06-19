// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ostream>

// Types here are used to register KeepAlives.
// They Give indications about which kind of optimizations are allowed during
// the KeepAlive's lifetime. This allows to have more info about the state of
// the browser to optimize the resource consumption.

// Refers to the what the KeepAlive's lifetime is tied to, to help debugging.
enum class KeepAliveOrigin {

    APP_MAIN_MODULE,
    APP_UI_EXAMPLE,

    // c/b
    APP_CONTROLLER,
    BROWSER,
    SESSION_RESTORE,

    // c/b/background
    BACKGROUND_MODE_MANAGER,
    BACKGROUND_MODE_MANAGER_STARTUP,
    BACKGROUND_MODE_MANAGER_FORCE_INSTALLED_EXTENSIONS,

    // c/b/background_sync
    BACKGROUND_SYNC,

    // c/b/browsing_data
    BROWSING_DATA_LIFETIME_MANAGER,

    // c/b/chromeos
    LOGIN_DISPLAY_HOST_WEBUI,
    PIN_MIGRATION,

    // c/b/devtools
    REMOTE_DEBUGGING,
    DEVTOOLS_WINDOW,

    // c/b/extensions
    NATIVE_MESSAGING_HOST_ERROR_REPORT,

    // c/b/notifications
    NOTIFICATION,
    PENDING_NOTIFICATION_CLICK_EVENT,

    // c/b/push_messaging
    IN_FLIGHT_PUSH_MESSAGE,

    // c/b/ui
    APP_LIST_SERVICE_VIEWS,
    APP_LIST_SHOWER,
    APP_APP_DELEGATE,
    APP_VIEWS_DELEGATE,
    PANEL,
    PANEL_VIEW,
    PROFILE_HELPER,
    PROFILE_LOADER,
    USER_MANAGER_VIEW,
    CREDENTIAL_PROVIDER_SIGNIN_DIALOG,

    // c/b/web_applications
    APP_START_URL_MIGRATION,
};

// Restart: Allow App to restart when all the registered KeepAlives allow
// restarts
enum class KeepAliveRestartOption { DISABLED, ENABLED };

std::ostream& operator<<(std::ostream& out, const KeepAliveOrigin& origin);
std::ostream& operator<<(std::ostream& out,
    const KeepAliveRestartOption& restart);

