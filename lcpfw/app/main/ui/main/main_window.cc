#include "main_window.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
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

namespace {
}

MainWindow* MainWindow::ShowWindow(base::OnceClosure quit_closure)
{
    views::Widget* widget = views::Widget::CreateWindowWithContext(
        new MainWindow(std::move(quit_closure)),
        gfx::kNullNativeWindow);
    widget->Show();

    //views::Widget* widget = new views::Widget;
    //views::Widget::InitParams params;
    //params.delegate = new MainWindow(runloop.QuitWhenIdleClosure());
    //params.context = gfx::kNullNativeWindow;
    ////params.name = "lcpfw login UI";
    //widget->Init(std::move(params));
    //widget->Show();

    return (MainWindow*)widget->widget_delegate();
}

MainWindow::MainWindow(base::OnceClosure quit_closure)
    : CommonWidgetDelegateView(L"lcpfw main"),
    quit_closure_(std::move(quit_closure))
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::ViewHierarchyChanged(const views::ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
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

        std::unique_ptr<views::ImageButton> profile_button(new views::ImageButton(base::BindRepeating(&MainWindow::OnProfileButtonClick, base::Unretained(this))));
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
            base::BindRepeating(&MainWindow::OnLoginButtonClick, base::Unretained(this)),
            L"登录"));
        login_btn->SetIsDefault(true);
        login_btn->SetBorder(login_btn->CreateDefaultBorder());

        layout->AddPaddingRow(0, 12);
        layout->StartRow(0, 1);
        layout->AddView(std::move(login_btn));

        layout->AddPaddingRow(0, 24);

        account_edit_->RequestFocus();
    }
}

void MainWindow::WindowClosing()
{
    __super::WindowClosing();

    std::move(quit_closure_).Run();
}

gfx::Size MainWindow::CalculatePreferredSize() const
{
    gfx::Size size(800, 600);
    return size;
}

void MainWindow::OnProfileButtonClick(const ui::Event& event)
{
    GetWidget()->Close();
}

void MainWindow::OnLoginButtonClick(const ui::Event& event)
{
    GetWidget()->Close();
}
