#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "base/callback_forward.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observer.h"
#include "base/sequenced_task_runner.h"
#include "ui/views/status_icons/status_icon.h"
#include "ui/views/status_icons/status_icon_menu_model.h"
#include "components/keep_alive_registry/app_scoped_keep_alive.h"
#include "components/prefs/pref_change_registrar.h"
//#include "content/public/notification/notification_observer.h"
//#include "content/public/notification/notification_registrar.h"

class PrefRegistrySimple;
class ScopedProfileKeepAlive;
class StatusIcon;
class StatusTray;

namespace base {
    class CommandLine;
}

using CommandIdHandlerVector = std::vector<base::RepeatingClosure>;

enum StatusTrayType
{
    None,

    Login,

    Main,
};

class BackgroundModeManager : //public content::NotificationObserver,
    public StatusIconMenuModel::Delegate {
public:
    BackgroundModeManager(const base::CommandLine& command_line);
    ~BackgroundModeManager() override;

    void UpdateStatusTrayIcon(StatusTrayType type);

private:
    // content::NotificationObserver implementation.
    /*void Observe(int type,
                 const content::NotificationSource& source,
                 const content::NotificationDetails& details) override;*/

                 // Overrides from StatusIconMenuModel::Delegate implementation.
    void ExecuteCommand(int command_id, int event_flags) override;

    // Create a status tray icon to allow the user to shutdown Chrome when running
    // in background mode. Virtual to enable testing.
    virtual void CreateStatusTrayIcon();

    // Removes the status tray icon because we are exiting background mode.
    // Virtual to enable testing.
    virtual void RemoveStatusTrayIcon();

    // Create a context menu, or replace/update an existing context menu, for the
    // status tray icon which, among other things, allows the user to shutdown
    // Chrome when running in background mode. All profiles are listed under
    // the one context menu.
    virtual void UpdateStatusTrayIconContextMenu();

private:
    StatusTrayType status_type_ = StatusTrayType::None;
    // Registrars for managing our change observers.
    //content::NotificationRegistrar registrar_;
    PrefChangeRegistrar pref_registrar_;

    // Indexes the command ids for the entire background menu to their handlers.
    CommandIdHandlerVector command_id_handler_vector_;

    // Maintains submenu lifetime for the multiple profile context menu.
    std::vector<std::unique_ptr<StatusIconMenuModel>> submenus;

    // Reference to our status tray. If null, the platform doesn't support status
    // icons.
    StatusTray* status_tray_ = nullptr;

    // Reference to our status icon (if any) - owned by the StatusTray.
    StatusIcon* status_icon_ = nullptr;

    // Reference to our status icon's context menu (if any) - owned by the
    // status_icon_.
    StatusIconMenuModel* context_menu_ = nullptr;

    // Background mode does not always keep Chrome alive. When it does, it is
    // using this scoped object.
    //std::unique_ptr<ScopedKeepAlive> keep_alive_;

    base::WeakPtrFactory<BackgroundModeManager> weak_factory_{ this };

    DISALLOW_COPY_AND_ASSIGN(BackgroundModeManager);
};
