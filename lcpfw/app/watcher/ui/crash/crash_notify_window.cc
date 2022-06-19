#include "crash_notify_window.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/process/launch.h"
#include "base/time/time.h"
#include "ui/base/ui_base_switches.h"
#include "ui/views/widget/native_widget.h"
#include "ui/views/widget/widget.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/background.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/controls/image_view.h"
#include "ui/base/ime/text_input_flags.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/label_button_border.h"
#include "ui/base/resource/resource_bundle.h"

#include "content/public/notification/notification_service.h"
#include "common/app_constants.h"


void CrashNotifWindow::ShowWindow(base::RepeatingClosure quit_closure)
{
    views::Widget* widget = views::Widget::CreateWindowWithContext(
        new CrashNotifWindow(quit_closure),
        gfx::kNullNativeWindow);
    widget->Show();
}

CrashNotifWindow::CrashNotifWindow(const base::RepeatingClosure& closure)
    : quit_closure_(closure)
{
}

CrashNotifWindow::~CrashNotifWindow()
{
}

void CrashNotifWindow::ViewHierarchyChanged(const views::ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
        InitView();
    }
}

void CrashNotifWindow::WindowClosing()
{
    __super::WindowClosing();

    quit_closure_.Run();
}

base::string16 CrashNotifWindow::GetWindowTitle() const
{
    return L"lcpfw crash feedback";
}

gfx::Size CrashNotifWindow::CalculatePreferredSize() const
{
    gfx::Size size(430, 300);
    return size;
}

void CrashNotifWindow::Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details)
{
    /*switch (type)
    {
    case lcpfw::NOTIFICATION_APP_ACTIVE:
        GetWidget()->Activate();
        break;
    case lcpfw::NOTIFICATION_APP_EXIT:
        GetWidget()->Close();
    	break;
    default:
        break;
    }*/
}

void CrashNotifWindow::InitView()
{
    SetBackground(views::CreateSolidBackground(SK_ColorWHITE));

    views::GridLayout* layout = SetLayoutManager(std::make_unique<views::GridLayout>());
    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, 12);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0,
        views::GridLayout::ColumnSize::kUsePreferred, 0, 0);
    column_set->AddPaddingColumn(0, 12);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(1.0f, 12);
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::FILL, 0,
        views::GridLayout::ColumnSize::kUsePreferred, 0, 0);
    column_set->AddPaddingColumn(1.0f, 12);

    std::unique_ptr<views::ImageView> logo(new views::ImageView());
    logo->SetImage(ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(171));//IDR_DEFAULT_FAVICON,171

    layout->StartRow(0, 0);
    layout->AddView(std::move(logo));

    std::unique_ptr<views::ImageButton> profile_button(new views::ImageButton(base::BindRepeating(&CrashNotifWindow::OnProfileButtonClick, base::Unretained(this))));
    profile_button->SetImageHorizontalAlignment(views::ImageButton::ALIGN_CENTER);
    profile_button->SetImage(views::Button::STATE_HOVERED, ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(8704));//IDR_DEFAULT_FAVICON_64,8704
    profile_button->SetImage(views::Button::STATE_PRESSED, ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(8704));//IDR_DEFAULT_FAVICON_64,8704
    profile_button->SetImage(views::Button::STATE_NORMAL, ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(8704));//IDR_DEFAULT_FAVICON_64,8704

    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 1);
    profile_button_ = layout->AddView(std::move(profile_button));

    std::unique_ptr<views::Textfield> account_edit(new views::Textfield());
    account_edit->SetPlaceholderText(L"请输入账号");
    account_edit->SetDefaultWidthInChars(20);
    std::unique_ptr<views::Textfield> psw_edit(new views::Textfield());
    psw_edit->SetPlaceholderText(L"请输入密码");
    psw_edit->SetDefaultWidthInChars(20);
    psw_edit->SetTextInputType(ui::TEXT_INPUT_TYPE_PASSWORD);

    layout->AddPaddingRow(0, 12);
    layout->StartRow(0, 1);
    account_edit_ = layout->AddView(std::move(account_edit));

    layout->AddPaddingRow(0, 12);
    layout->StartRow(0, 1);
    psw_edit_ = layout->AddView(std::move(psw_edit));

    std::unique_ptr<views::LabelButton> login_btn(new views::LabelButton(
        base::BindRepeating(&CrashNotifWindow::OnLoginButtonClick, base::Unretained(this)),
        L"登录"));
    login_btn->SetIsDefault(true);
    login_btn->SetBorder(login_btn->CreateDefaultBorder());

    layout->AddPaddingRow(0, 12);
    layout->StartRow(0, 1);
    layout->AddView(std::move(login_btn));

    layout->AddPaddingRow(0, 24);

    account_edit_->RequestFocus();
}

void CrashNotifWindow::OnProfileButtonClick(const ui::Event& event)
{
    GetWidget()->Close();
}

void CrashNotifWindow::OnLoginButtonClick(const ui::Event& event)
{
    // Restart app
    auto cmdline = base::CommandLine::ForCurrentProcess();
    base::CommandLine crashpad(cmdline->GetProgram());
    crashpad.AppendSwitch(lcpfw::kSwitchCrashRestart);
    base::LaunchProcess(crashpad, base::LaunchOptions());

    GetWidget()->Close();
}
