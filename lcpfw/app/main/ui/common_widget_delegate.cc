#include "common_widget_delegate.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/views/widget/native_widget.h"
#include "ui/views/widget/widget.h"
#include "ui/views/background.h"



CommonWidgetDelegateView::CommonWidgetDelegateView(const base::string16& title)
    : title_(title)
{
    //SetHasWindowSizeControls(true);
    SetBackground(CreateThemedSolidBackground(
        this, ui::NativeTheme::kColorId_DialogBackground));
}

CommonWidgetDelegateView::~CommonWidgetDelegateView()
{
}

std::unique_ptr<views::NonClientFrameView> CommonWidgetDelegateView::CreateNonClientFrameView(views::Widget* widget)
{
    return __super::CreateNonClientFrameView(widget);
}

base::string16 CommonWidgetDelegateView::GetWindowTitle() const
{
    return title_;
}

bool CommonWidgetDelegateView::ExecuteWindowsCommand(int command_id)
{
#if OS_WIN
    if (command_id == SC_CLOSE)
    {
        GetWidget()->Close();
        return true;
    }
#endif
    return false;
}
