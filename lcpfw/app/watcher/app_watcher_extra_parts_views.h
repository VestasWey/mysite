#pragma once

#include "content/app_main_extra_parts.h"

namespace display
{
    class Screen;
}

namespace wm
{
    class WMState;
}

namespace aura
{
    class Env;
}

namespace ui
{
    class ScopedOleInitializer;
    class AppContextFactories;
}

namespace views
{
}

class AppViewsDelegate;

class WatcherMainExtraPartsViews
    : public AppMainExtraParts {
public:
    WatcherMainExtraPartsViews();

    void ToolkitInitialized() override;
    void PostAppStart() override;
    void PostMainMessageLoopRun() override;

private:
    void LoadResourceBundle();

private:
#ifdef OS_WIN
    std::unique_ptr<ui::ScopedOleInitializer> ole_initializer_;
#endif

    std::unique_ptr<wm::WMState> wm_state_;
    std::unique_ptr<ui::AppContextFactories> context_factories_;
    std::unique_ptr<aura::Env> aura_env_;
    std::unique_ptr<display::Screen> desktop_screen_;

    std::unique_ptr<AppViewsDelegate> views_delegate_;

    DISALLOW_COPY_AND_ASSIGN(WatcherMainExtraPartsViews);
};
