#pragma once

#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/widget/widget_delegate.h"

#include "content/public/notification/notification_observer.h"
#include "content/public/notification/notification_registrar.h"


class CrashNotifWindow
    : public views::WidgetDelegateView,
    public content::NotificationObserver
{
public:
    static void ShowWindow(base::RepeatingClosure quit_closure);

protected:
    CrashNotifWindow(const base::RepeatingClosure& closure);
    ~CrashNotifWindow() override;

    // WidgetDelegate
    void WindowClosing() override;
    base::string16 GetWindowTitle() const override;

    // View
    void ViewHierarchyChanged(const views::ViewHierarchyChangedDetails& details) override;
    gfx::Size CalculatePreferredSize() const override;

    // NotificationObserver
    void Observe(int type,
        const content::NotificationSource& source,
        const content::NotificationDetails& details) override;

private:
    void InitView();
    void OnProfileButtonClick(const ui::Event& event);
    void OnLoginButtonClick(const ui::Event& event);

private:
    base::RepeatingClosure quit_closure_;
    views::ImageButton* profile_button_ = nullptr;
    views::Textfield* account_edit_ = nullptr;
    views::Textfield* psw_edit_ = nullptr;

    content::NotificationRegistrar ntf_reg_;

    DISALLOW_COPY_AND_ASSIGN(CrashNotifWindow);
};
