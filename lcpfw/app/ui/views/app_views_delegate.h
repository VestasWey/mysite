#pragma once

#include "base/compiler_specific.h"
#include "build/build_config.h"
#include "ui/views/views_delegate.h"
#include "ui/views/layout/layout_provider.h"


class AppViewsDelegate
    : public views::ViewsDelegate
{
public:
    AppViewsDelegate();
    ~AppViewsDelegate() override;

    // If set to |true|, forces widgets that do not provide a native widget to use
    // DesktopNativeWidgetAura instead of whatever the default native widget would
    // be. This has no effect on ChromeOS.
    void set_use_desktop_native_widgets(bool desktop)
    {
        use_desktop_native_widgets_ = desktop;
    }

    void set_use_transparent_windows(bool transparent)
    {
        use_transparent_windows_ = transparent;
    }

#if defined(OS_APPLE)
    // Allows tests to provide a ContextFactory via the ViewsDelegate interface.
    void set_context_factory(ui::ContextFactory* context_factory)
    {
        context_factory_ = context_factory;
    }
#endif

    // For convenience, we create a layout provider by default, but embedders
    // that use their own layout provider subclasses may need to set those classes
    // as the layout providers for their tests.
    void set_layout_provider(std::unique_ptr<views::LayoutProvider> layout_provider)
    {
        layout_provider_.swap(layout_provider);
    }

    // ViewsDelegate:
#if defined(OS_WIN)
    HICON GetDefaultWindowIcon() const override;
    HICON GetSmallWindowIcon() const override;
#endif

    void OnBeforeWidgetInit(views::Widget::InitParams* params,
        views::internal::NativeWidgetDelegate* delegate) override;

#if defined(OS_APPLE)
    ui::ContextFactory* GetContextFactory() override;
#endif

private:
#if defined(OS_APPLE)
    ui::ContextFactory* context_factory_ = nullptr;
#endif
    bool use_desktop_native_widgets_ = false;
    bool use_transparent_windows_ = false;
    std::unique_ptr<views::LayoutProvider> layout_provider_ = std::make_unique<views::LayoutProvider>();

    DISALLOW_COPY_AND_ASSIGN(AppViewsDelegate);
};
