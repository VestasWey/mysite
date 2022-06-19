#include "main/ui/main_module.h"

#include "content/app_post_task_helper.h"
#include "components/keep_alive_registry/app_scoped_keep_alive.h"
#include "components/keep_alive_registry/app_keep_alive_types.h"
#include "main/ui/main/main_window.h"


MainModule::MainModule()
    : keep_alive_(new ScopedKeepAlive(KeepAliveOrigin::APP_MAIN_MODULE, KeepAliveRestartOption::DISABLED))
{
}

MainModule::~MainModule()
{
}

void MainModule::Init()
{
    InitMainWindow();
}

void MainModule::Uninit()
{
    main_window_ = nullptr;
    keep_alive_.reset();    // notify AppMainProcess quit MessageLoop
}

void MainModule::InitMainWindow()
{
    main_window_ = MainWindow::ShowWindow(base::BindOnce(&MainModule::OnMainWindowDestroyed, this));
}

void MainModule::OnMainWindowDestroyed()
{
    main_window_ = nullptr;
    lcpfw::PostTask(FROM_HERE, base::BindOnce(&MainModule::ActuallyShutdown, this));
}

void MainModule::ActuallyShutdown()
{
    Uninit();
}

MainWindow* MainModule::main_window() const
{
    DCHECK(main_window_);
    return main_window_;
}
