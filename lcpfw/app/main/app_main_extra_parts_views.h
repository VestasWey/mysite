#pragma once

//#include "ui/display/screen.h"

#include "content/app_main_extra_parts.h"
//#include "ui/views/app_desktop_views_delegate.h"

class AppMainExtraPartsViews
    : public AppMainExtraParts {
public:
    AppMainExtraPartsViews();

    void ToolkitInitialized() override;

    void PostAppStart() override;

private:
    //std::unique_ptr<display::Screen> desktop_screen_;
    //std::unique_ptr<lcpfw::AppViewsDelegate> views_delegate_;

    DISALLOW_COPY_AND_ASSIGN(AppMainExtraPartsViews);
};
