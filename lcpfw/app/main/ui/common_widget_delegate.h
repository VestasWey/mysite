#pragma once

#include "ui/views/widget/widget_delegate.h"


class CommonWidgetDelegateView
    : public views::WidgetDelegateView
{
public:
    CommonWidgetDelegateView(const base::string16& title);
    ~CommonWidgetDelegateView() override;

protected:
    // WidgetDelegate
    std::unique_ptr<views::NonClientFrameView> CreateNonClientFrameView(views::Widget* widget) override;
    base::string16 GetWindowTitle() const override;
    bool ExecuteWindowsCommand(int command_id) override;

private:
    base::string16 title_;
    DISALLOW_COPY_AND_ASSIGN(CommonWidgetDelegateView);
};
