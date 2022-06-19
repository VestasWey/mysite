// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "main/ui/background_mode_manager.h"

#include <stddef.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "base/base_paths.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/command_line.h"
#include "base/containers/contains.h"
#include "base/location.h"
#include "base/logging.h"
//#include "base/metrics/histogram_macros.h"
//#include "base/metrics/user_metrics.h"
#include "base/one_shot_event.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/utf_string_conversions.h"
//#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"
#include "components/keep_alive_registry/app_keep_alive_registry.h"
#include "components/keep_alive_registry/app_keep_alive_types.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
//#include "content/public/browser/notification_service.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/image_model.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_family.h"
#include "ui/views/status_icons/status_tray.h"

#if defined(OS_WIN)
#include "ui/win/app_icon.h"
#endif
#include "content/app_main_process.h"
#include "main/ui/menu_item_ids.h"
#include "content/public/notification/notification_service.h"
#include "public/main/notification_types.h"

namespace {

    // Enum for recording menu item clicks in UMA.
    // NOTE: Do not renumber these as that would confuse interpretation of
    // previously logged data. When making changes, also update histograms.xml.
    /*enum MenuItem {
        MENU_ITEM_ABOUT = 0,
        MENU_ITEM_TASK_MANAGER = 1,
        MENU_ITEM_BACKGROUND_CLIENT = 2,
        MENU_ITEM_KEEP_RUNNING = 3,
        MENU_ITEM_EXIT = 4,
        MENU_ITEM_NUM_STATES
    };

    void RecordMenuItemClick(MenuItem item)
    {
        UMA_HISTOGRAM_ENUMERATION("BackgroundMode.MenuItemClick", item,
            MENU_ITEM_NUM_STATES);
    }*/
}  // namespace

///////////////////////////////////////////////////////////////////////////////
//  BackgroundModeManager, public
BackgroundModeManager::BackgroundModeManager(
    const base::CommandLine& command_line)
{
    // Listen for the background mode preference changing.
    //if (GetAppMainProcess()->local_state()) {  // Skip for unit tests
    //  pref_registrar_.Init(GetAppMainProcess()->local_state());
    //  pref_registrar_.Add(
    //      prefs::kBackgroundModeEnabled,
    //      base::BindRepeating(
    //          &BackgroundModeManager::OnBackgroundModeEnabledPrefChanged,
    //          base::Unretained(this)));
    //}

    // Listen for the application shutting down so we can release our KeepAlive.
    /*registrar_.Add(this, chrome::NOTIFICATION_APP_TERMINATING,
                   content::NotificationService::AllSources());*/
}

BackgroundModeManager::~BackgroundModeManager()
{
}

void BackgroundModeManager::UpdateStatusTrayIcon(StatusTrayType type)
{
    status_type_ = type;
    RemoveStatusTrayIcon();
    CreateStatusTrayIcon();
}

///////////////////////////////////////////////////////////////////////////////
//  BackgroundModeManager, content::NotificationObserver overrides
//void BackgroundModeManager::Observe(
//    int type,
//    const content::NotificationSource& source,
//    const content::NotificationDetails& details) {
//  DCHECK_EQ(chrome::NOTIFICATION_APP_TERMINATING, type);
//
//  // Make sure we aren't still keeping the app alive (only happens if we
//  // don't receive an EXTENSIONS_READY notification for some reason).
//  ReleaseStartupKeepAlive();
//  // Performing an explicit shutdown, so exit background mode (does nothing
//  // if we aren't in background mode currently).
//  EndBackgroundMode();
//  // Shutting down, so don't listen for any more notifications so we don't
//  // try to re-enter/exit background mode again.
//  registrar_.RemoveAll();
//  for (const auto& it : background_mode_data_)
//    it.second->applications()->RemoveObserver(this);
//}

///////////////////////////////////////////////////////////////////////////////
//  BackgroundModeManager::BackgroundModeData, StatusIconMenuModel overrides
void BackgroundModeManager::ExecuteCommand(int command_id, int event_flags)
{
    switch (command_id)
    {
    case IDC_SHOW:
        content::NotificationService::current()->Notify(lcpfw::NOTIFICATION_APP_ACTIVE,
            content::NotificationService::AllSources(), content::NotificationService::NoDetails());
        break;
    case IDC_ABOUT:
        break;
    case IDC_EXIT:
        content::NotificationService::current()->Notify(lcpfw::NOTIFICATION_APP_EXIT,
            content::NotificationService::AllSources(), content::NotificationService::NoDetails());
        break;
    default:
        break;
    }
}

// Gets the image for the status tray icon, at the correct size for the current
// platform and display settings.
gfx::ImageSkia GetStatusTrayIcon()
{
#if defined(OS_WIN)
    // On Windows, use GetSmallAppIconSize to get the correct image size. The
    // user's "text size" setting in Windows determines how large the system tray
    // icon should be.
    gfx::Size size = GetSmallAppIconSize();

    // This loads all of the icon images, which is a bit wasteful because we're
    // going to pick one and throw the rest away, but that is the price of using
    // the ImageFamily abstraction. Note: We could just use the LoadImage function
    // from the Windows API, but that does a *terrible* job scaling images.
    // Therefore, we fetch the images and do our own high-quality scaling.
    std::unique_ptr<gfx::ImageFamily> family = GetAppIconImageFamily();
    DCHECK(family);
    if (!family)
        return gfx::ImageSkia();

    return family->CreateExact(size).AsImageSkia();
#elif defined(OS_LINUX) || defined(OS_CHROMEOS)
    return *ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(
        IDR_PRODUCT_LOGO_128);
#elif defined(OS_MAC)
    return *ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(
        IDR_STATUS_TRAY_ICON);
#else
    NOTREACHED();
    return gfx::ImageSkia();
#endif
}

void BackgroundModeManager::CreateStatusTrayIcon()
{
    if (status_type_ == StatusTrayType::None)
    {
        return;
    }

    // Only need status icons on windows/linux. ChromeOS doesn't allow exiting
    // Chrome and Mac can use the dock icon instead.

    // Since there are multiple profiles which share the status tray, we now
    // use the browser process to keep track of it.
#if !defined(OS_MAC) && !BUILDFLAG(IS_CHROMEOS_ASH) && !BUILDFLAG(IS_CHROMEOS_LACROS)
    if (!status_tray_)
        status_tray_ = GetAppMainProcess()->status_tray();
#endif

    // If the platform doesn't support status icons, or we've already created
    // our status icon, just return.
    if (!status_tray_ || status_icon_)
        return;

    base::string16 title;
    switch (status_type_)
    {
    case StatusTrayType::Login:
        title = L"lcpfw login";
        break;
    default:
        title = L"lcpfw";
        break;
    }

    status_icon_ = status_tray_->CreateStatusIcon(
        StatusTray::BACKGROUND_MODE_ICON, GetStatusTrayIcon(),
        title/*l10n_util::GetStringUTF16(IDS_PRODUCT_NAME)*/);
    if (!status_icon_)
        return;

    UpdateStatusTrayIconContextMenu();
}

void BackgroundModeManager::UpdateStatusTrayIconContextMenu()
{
    // If we don't have a status icon or one could not be created succesfully,
    // then no need to continue the update.
    if (!status_icon_)
        return;

    command_id_handler_vector_.clear();
    submenus.clear();

    std::unique_ptr<StatusIconMenuModel> menu(new StatusIconMenuModel(this));

    //menu->AddItemWithStringId(IDC_TASK_MANAGER, IDS_TASK_MANAGER);
    //menu->AddCheckItemWithStringId(
    //    IDC_STATUS_TRAY_KEEP_CHROME_RUNNING_IN_BACKGROUND,
    //    IDS_STATUS_TRAY_KEEP_CHROME_RUNNING_IN_BACKGROUND);
    //menu->SetCommandIdChecked(IDC_STATUS_TRAY_KEEP_CHROME_RUNNING_IN_BACKGROUND, true);

    menu->AddItem(IDC_SHOW, /*GetMenuItemString()*/L"显示主面板");
    menu->AddSeparator(ui::NORMAL_SEPARATOR);

    menu->AddItem(IDC_ABOUT, /*GetMenuItemString()*/L"关于");
    menu->AddSeparator(ui::NORMAL_SEPARATOR);

    menu->AddItem(IDC_EXIT, L"Exit");

    context_menu_ = menu.get();
    status_icon_->SetContextMenu(std::move(menu));
}

void BackgroundModeManager::RemoveStatusTrayIcon()
{
    if (!status_tray_)
    {
        return;
    }

    if (status_icon_)
        status_tray_->RemoveStatusIcon(status_icon_);
    status_icon_ = nullptr;
    context_menu_ = nullptr;
}
