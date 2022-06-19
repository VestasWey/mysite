#include "app_watcher_extra_parts_views.h"

#include "base/path_service.h"

#include "mojo/core/embedder/embedder.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/display/screen.h"
#include "ui/gfx/font_util.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"

#if defined(OS_WIN)
#include "ui/base/win/scoped_ole_initializer.h"
#include "ui/base/resource/resource_bundle_win.h"
#endif

#if defined(USE_AURA)
#include "ui/aura/env.h"
#include "ui/wm/core/wm_state.h"
#endif

#include "components/viz/app_gpu_service_holder.h"
#include "ui/compositor/app_context_factories.h"
#include "ui/views/app_desktop_views_delegate.h"

#include "common/app_constants.h"
#include "common/app_pref_names.h"
#include "content/app_main_process.h"

/*
* 参考//ui/views/examples:examlpes_main_proc.cc的初始化顺序。
* 将examlpes_main_proc.cc中对FeatureList的初始化代码放到app_main模块的入口函数中，如果放在这里就不起作用了。
*/

WatcherMainExtraPartsViews::WatcherMainExtraPartsViews()
{}

void WatcherMainExtraPartsViews::ToolkitInitialized()
{
#ifdef OS_WIN
    // Ole must be initialized before starting message pump, so that TSF
    // (Text Services Framework) module can interact with the message pump
    // on Windows 8 Metro mode.
    ole_initializer_ = std::make_unique<ui::ScopedOleInitializer>();
#endif

    mojo::core::Init();

    gl::init::InitializeGLOneOff();

    // The ContextFactory must exist before any Compositors are created.
    context_factories_ = std::make_unique<ui::AppContextFactories>(false);
    context_factories_->SetUseTestSurface(false);

    ui::RegisterPathProvider();

    LoadResourceBundle();

    gfx::InitializeFonts();

#if defined(USE_AURA)
    aura_env_ = aura::Env::CreateInstance();
    aura_env_->set_context_factory(context_factories_->GetContextFactory());
#endif

    ui::InitializeInputMethod();

    if (!views::ViewsDelegate::GetInstance())
    {
        views_delegate_.reset(new AppDesktopViewsDelegate);
    }

#if defined(USE_AURA)
    wm_state_.reset(new wm::WMState);
#endif

    desktop_screen_.reset(views::CreateDesktopScreen());
}

void WatcherMainExtraPartsViews::PostAppStart()
{
}

void WatcherMainExtraPartsViews::PostMainMessageLoopRun()
{
    desktop_screen_.reset();

    wm_state_.reset();

    views_delegate_.reset();

    ui::ShutdownInputMethod();

#if defined(USE_AURA)
    aura_env_.reset();
#endif

    ui::ResourceBundle::CleanupSharedInstance();

    context_factories_.reset();

    // shutdown GPU main/io thread, must be called before GL context cleanup.
    viz::AppGpuServiceHolder::ResetInstance();

    gl::init::ShutdownGL(false);

#ifdef OS_WIN
    ole_initializer_.reset();
#endif
}

void WatcherMainExtraPartsViews::LoadResourceBundle()
{
    //std::string locale = GetAppMainProcess()->GetApplicationLocale();
    std::string locale = prefs::kLocaleZhCN;

    base::FilePath locales_pak_path;
    CHECK(base::PathService::Get(base::DIR_MODULE, &locales_pak_path));
    // InitSharedInstanceWithPakPath将pak用于图片资源和字符串资源，所以初始化ResourceBundle时应该先传入字符串资源包（当然这个包也可以是字符串和图片等其他资源的完全体整合包）
    auto locale_file = locales_pak_path.Append(FILE_PATH_LITERAL("locales")).AppendASCII(locale + ".pak");
    ui::ResourceBundle::InitSharedInstanceWithPakPath(locale_file);
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        locales_pak_path.Append(FILE_PATH_LITERAL("lcpfw_100_percent.pak")),
        ui::SCALE_FACTOR_100P);
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        locales_pak_path.Append(FILE_PATH_LITERAL("lcpfw_150_percent.pak")),
        ui::SCALE_FACTOR_150P);
    ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
        locales_pak_path.Append(FILE_PATH_LITERAL("lcpfw_200_percent.pak")),
        ui::SCALE_FACTOR_200P);

#ifdef _WIN32
    ui::SetResourcesDataDLL(GetModuleHandleW(lcpfw::kAppResourcesDll));
#endif
}
