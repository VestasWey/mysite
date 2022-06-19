#pragma once

#include "main/ui/common_widget_delegate.h"

#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/button/image_button.h"

#include "content/public/notification/notification_observer.h"
#include "content/public/notification/notification_registrar.h"

enum LoginResult
{
    Success,
    AccountError,
    PswError,
    Cancel,
};

class LoginWindow
    : public CommonWidgetDelegateView,
    public content::NotificationObserver
{
public:
    static LoginResult ShowWindow();

protected:
    LoginWindow(const base::RepeatingClosure& closure);
    ~LoginWindow() override;

    // WidgetDelegate
    void WindowClosing() override;

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

    DISALLOW_COPY_AND_ASSIGN(LoginWindow);
};
