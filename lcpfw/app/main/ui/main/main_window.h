#pragma once

#include "main/ui/common_widget_delegate.h"

#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/button/image_button.h"


class MainWindow
    : public CommonWidgetDelegateView
{
public:
    static MainWindow* ShowWindow(base::OnceClosure quit_closure);

protected:
    MainWindow(base::OnceClosure quit_closure);
    ~MainWindow() override;

    // WidgetDelegate
    void WindowClosing() override;

    void ViewHierarchyChanged(const views::ViewHierarchyChangedDetails& details) override;
    gfx::Size CalculatePreferredSize() const override;

private:
    void OnProfileButtonClick(const ui::Event& event);
    void OnLoginButtonClick(const ui::Event& event);

private:
    base::OnceClosure quit_closure_;
    views::ImageButton* profile_button_ = nullptr;
    views::Textfield* account_edit_ = nullptr;
    views::Textfield* psw_edit_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(MainWindow);
};
