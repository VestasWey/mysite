#include "app_main_extra_parts_views.h"

#include "base/command_line.h"
//#include "components/prefs/pref_service.h"
//
//#include "ui/base/ui_base_switches.h"
//#include "ui/views/buildflags.h"
//
//#if BUILDFLAG(ENABLE_DESKTOP_AURA)
//#include "ui/views/widget/desktop_aura/desktop_screen.h"
//#endif


AppMainExtraPartsViews::AppMainExtraPartsViews()
{}

void AppMainExtraPartsViews::ToolkitInitialized()
{
    //CommandLine::ForCurrentProcess()->AppendSwitch(switches::kDisableDwmComposition);

//#if BUILDFLAG(ENABLE_DESKTOP_AURA)
//    desktop_screen_ =
//        base::WrapUnique(views::CreateDesktopScreen());
//#endif
//
//    if (!views::ViewsDelegate::GetInstance())
//    {
//        views_delegate_.reset(new lcpfw::AppDesktopViewsDelegate);
//    }
}

void AppMainExtraPartsViews::PostAppStart()
{
    //CommonPrefService::RegisterAppHotkey();
}
