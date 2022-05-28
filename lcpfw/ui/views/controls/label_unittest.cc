// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/label.h"

#include <stddef.h>

#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/i18n/rtl.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/gtest_util.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/ax_enums.mojom.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/ui_base_switches.h"
#include "ui/compositor/canvas_painter.h"
#include "ui/events/base_event_utils.h"
#include "ui/events/test/event_generator.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/render_text.h"
#include "ui/gfx/text_constants.h"
#include "ui/gfx/text_elider.h"
#include "ui/strings/grit/ui_strings.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/base_control_test_widget.h"
#include "ui/views/controls/link.h"
#include "ui/views/style/typography.h"
#include "ui/views/test/focus_manager_test.h"
#include "ui/views/test/view_metadata_test_utils.h"
#include "ui/views/test/views_test_base.h"
#include "ui/views/widget/unique_widget_ptr.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_utils.h"

using base::ASCIIToUTF16;
using base::WideToUTF16;

#define EXPECT_STR_EQ(ascii, utf16) EXPECT_EQ(ASCIIToUTF16(ascii), utf16)

namespace views {

namespace {

#if defined(OS_APPLE)
const int kControlCommandModifier = ui::EF_COMMAND_DOWN;
#else
const int kControlCommandModifier = ui::EF_CONTROL_DOWN;
#endif

// All text sizing measurements (width and height) should be greater than this.
const int kMinTextDimension = 4;

class TestLabel : public Label {
 public:
  TestLabel() : Label(ASCIIToUTF16("TestLabel")) { SizeToPreferredSize(); }

  int schedule_paint_count() const { return schedule_paint_count_; }

  void SimulatePaint() {
    SkBitmap bitmap;
    SkColor color = SK_ColorTRANSPARENT;
    Paint(PaintInfo::CreateRootPaintInfo(
        ui::CanvasPainter(&bitmap, bounds().size(), 1.f, color, false)
            .context(),
        bounds().size()));
  }

  // View:
  void OnDidSchedulePaint(const gfx::Rect& r) override {
    ++schedule_paint_count_;
    Label::OnDidSchedulePaint(r);
  }

 private:
  int schedule_paint_count_ = 0;

  DISALLOW_COPY_AND_ASSIGN(TestLabel);
};

// A test utility function to set the application default text direction.
void SetRTL(bool rtl) {
  // Override the current locale/direction.
  base::i18n::SetICUDefaultLocale(rtl ? "he" : "en");
  EXPECT_EQ(rtl, base::i18n::IsRTL());
}

base::string16 GetClipboardText(ui::ClipboardBuffer clipboard_buffer) {
  base::string16 clipboard_text;
  ui::Clipboard::GetForCurrentThread()->ReadText(
      clipboard_buffer, /* data_dst = */ nullptr, &clipboard_text);
  return clipboard_text;
}

// Makes an RTL string by mapping 0..6 to [א,ב,ג,ד,ה,ו,ז].
base::string16 ToRTL(const char* ascii) {
  base::string16 rtl;
  for (const char* c = ascii; *c; ++c) {
    if (*c >= '0' && *c <= '6')
      rtl += L'\x5d0' + (*c - '0');
    else
      rtl += static_cast<base::string16::value_type>(*c);
  }
  return rtl;
}

}  // namespace

class LabelTest : public test::BaseControlTestWidget {
 public:
  LabelTest() = default;
  LabelTest(const LabelTest&) = delete;
  LabelTest& operator=(const LabelTest&) = delete;
  ~LabelTest() override = default;

 protected:
  void CreateWidgetContent(View* container) override {
    label_ = container->AddChildView(std::make_unique<Label>());
  }

  Label* label() { return label_; }

 private:
  Label* label_ = nullptr;
};

// Test fixture for text selection related tests.
class LabelSelectionTest : public LabelTest {
 public:
  // Alias this long identifier for more readable tests.
  static constexpr bool kExtends =
      gfx::RenderText::kDragToEndIfOutsideVerticalBounds;

  // Some tests use cardinal directions to index an array of points above and
  // below the label in either visual direction.
  enum { NW, NORTH, NE, SE, SOUTH, SW };

  LabelSelectionTest() = default;

  // LabelTest overrides:
  void SetUp() override {
    LabelTest::SetUp();
    event_generator_ =
        std::make_unique<ui::test::EventGenerator>(GetRootWindow(widget()));
  }

 protected:
  View* GetFocusedView() {
    return widget()->GetFocusManager()->GetFocusedView();
  }

  void PerformMousePress(const gfx::Point& point) {
    ui::MouseEvent pressed_event = ui::MouseEvent(
        ui::ET_MOUSE_PRESSED, point, point, ui::EventTimeForNow(),
        ui::EF_LEFT_MOUSE_BUTTON, ui::EF_LEFT_MOUSE_BUTTON);
    label()->OnMousePressed(pressed_event);
  }

  void PerformMouseRelease(const gfx::Point& point) {
    ui::MouseEvent released_event = ui::MouseEvent(
        ui::ET_MOUSE_RELEASED, point, point, ui::EventTimeForNow(),
        ui::EF_LEFT_MOUSE_BUTTON, ui::EF_LEFT_MOUSE_BUTTON);
    label()->OnMouseReleased(released_event);
  }

  void PerformClick(const gfx::Point& point) {
    PerformMousePress(point);
    PerformMouseRelease(point);
  }

  void PerformMouseDragTo(const gfx::Point& point) {
    ui::MouseEvent drag(ui::ET_MOUSE_DRAGGED, point, point,
                        ui::EventTimeForNow(), ui::EF_LEFT_MOUSE_BUTTON, 0);
    label()->OnMouseDragged(drag);
  }

  // Used to force layout on the underlying RenderText instance.
  void SimulatePaint() {
    gfx::Canvas canvas;
    label()->OnPaint(&canvas);
  }

  gfx::Point GetCursorPoint(int index) {
    SimulatePaint();
    gfx::RenderText* render_text =
        label()->GetRenderTextForSelectionController();

    // For single-line text, use the glyph bounds since it gives a better
    // representation of the midpoint between glyphs when considering selection.
    // TODO(crbug.com/248597): Add multiline support to GetCursorBounds(...).
    if (!render_text->multiline()) {
      return render_text
          ->GetCursorBounds(gfx::SelectionModel(index, gfx::CURSOR_FORWARD),
                            true)
          .left_center();
    }

    // Otherwise, GetCursorBounds() will give incorrect results. Multiline
    // editing is not supported (http://crbug.com/248597) so there hasn't been
    // a need to draw a cursor. Instead, derive a point from the selection
    // bounds, which always rounds up to an integer after the end of a glyph.
    // This rounding differs to the glyph bounds, which rounds to nearest
    // integer. See http://crbug.com/735346.
    auto bounds = render_text->GetSubstringBounds({index, index + 1});
    DCHECK_EQ(1u, bounds.size());

    const bool rtl =
        render_text->GetDisplayTextDirection() == base::i18n::RIGHT_TO_LEFT;
    // Return Point corresponding to the leading edge of the character.
    return rtl ? bounds[0].right_center() + gfx::Vector2d(-1, 0)
               : bounds[0].left_center() + gfx::Vector2d(1, 0);
  }

  size_t GetLineCount() {
    SimulatePaint();
    return label()->GetRenderTextForSelectionController()->GetNumLines();
  }

  base::string16 GetSelectedText() { return label()->GetSelectedText(); }

  ui::test::EventGenerator* event_generator() { return event_generator_.get(); }

  bool IsMenuCommandEnabled(int command_id) {
    return label()->IsCommandIdEnabled(command_id);
  }

 private:
  std::unique_ptr<ui::test::EventGenerator> event_generator_;

  DISALLOW_COPY_AND_ASSIGN(LabelSelectionTest);
};

TEST_F(LabelTest, Metadata) {
  // Calling SetMultiLine() will DCHECK unless the label is in multi-line mode.
  label()->SetMultiLine(true);
  test::TestViewMetadata(label());
}

TEST_F(LabelTest, FontPropertySymbol) {
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  // On linux, the fonts are mocked with a custom FontConfig. The "Courier New"
  // family name is mapped to Cousine-Regular.ttf (see: $build/test_fonts/*).
  std::string font_name("Courier New");
#else
  std::string font_name("symbol");
#endif
  gfx::Font font(font_name, 26);
  label()->SetFontList(gfx::FontList(font));
  gfx::Font font_used = label()->font_list().GetPrimaryFont();
  EXPECT_EQ(font_name, font_used.GetFontName());
  EXPECT_EQ(26, font_used.GetFontSize());
}

TEST_F(LabelTest, FontPropertyArial) {
  std::string font_name("arial");
  gfx::Font font(font_name, 30);
  label()->SetFontList(gfx::FontList(font));
  gfx::Font font_used = label()->font_list().GetPrimaryFont();
  EXPECT_EQ(font_name, font_used.GetFontName());
  EXPECT_EQ(30, font_used.GetFontSize());
}

TEST_F(LabelTest, TextProperty) {
  base::string16 test_text(ASCIIToUTF16("A random string."));
  label()->SetText(test_text);
  EXPECT_EQ(test_text, label()->GetText());
}

TEST_F(LabelTest, TextStyleProperty) {
  label()->SetTextStyle(views::style::STYLE_DISABLED);
  EXPECT_EQ(views::style::STYLE_DISABLED, label()->GetTextStyle());
}

TEST_F(LabelTest, ColorProperty) {
  SkColor color = SkColorSetARGB(20, 40, 10, 5);
  label()->SetAutoColorReadabilityEnabled(false);
  label()->SetEnabledColor(color);
  EXPECT_EQ(color, label()->GetEnabledColor());
}

TEST_F(LabelTest, AlignmentProperty) {
  const bool was_rtl = base::i18n::IsRTL();

  for (size_t i = 0; i < 2; ++i) {
    // Toggle the application default text direction (to try each direction).
    SetRTL(!base::i18n::IsRTL());
    bool reverse_alignment = base::i18n::IsRTL();

    // The alignment should be flipped in RTL UI.
    label()->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    EXPECT_EQ(reverse_alignment ? gfx::ALIGN_LEFT : gfx::ALIGN_RIGHT,
              label()->GetHorizontalAlignment());
    label()->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    EXPECT_EQ(reverse_alignment ? gfx::ALIGN_RIGHT : gfx::ALIGN_LEFT,
              label()->GetHorizontalAlignment());
    label()->SetHorizontalAlignment(gfx::ALIGN_CENTER);
    EXPECT_EQ(gfx::ALIGN_CENTER, label()->GetHorizontalAlignment());

    for (size_t j = 0; j < 2; ++j) {
      label()->SetHorizontalAlignment(gfx::ALIGN_TO_HEAD);
      const bool rtl = j == 0;
      label()->SetText(rtl ? base::WideToUTF16(L"\x5d0") : ASCIIToUTF16("A"));
      EXPECT_EQ(gfx::ALIGN_TO_HEAD, label()->GetHorizontalAlignment());
    }
  }

  EXPECT_EQ(was_rtl, base::i18n::IsRTL());
}

TEST_F(LabelTest, MinimumSizeRespectsLineHeight) {
  base::string16 text(ASCIIToUTF16("This is example text."));
  label()->SetText(text);

  const gfx::Size minimum_size = label()->GetMinimumSize();
  const int expected_height = minimum_size.height() + 10;
  label()->SetLineHeight(expected_height);
  EXPECT_EQ(expected_height, label()->GetMinimumSize().height());
}

TEST_F(LabelTest, MinimumSizeRespectsLineHeightMultiline) {
  base::string16 text(ASCIIToUTF16("This is example text."));
  label()->SetText(text);
  label()->SetMultiLine(true);

  const gfx::Size minimum_size = label()->GetMinimumSize();
  const int expected_height = minimum_size.height() + 10;
  label()->SetLineHeight(expected_height);
  EXPECT_EQ(expected_height, label()->GetMinimumSize().height());
}

TEST_F(LabelTest, MinimumSizeRespectsLineHeightWithInsets) {
  base::string16 text(ASCIIToUTF16("This is example text."));
  label()->SetText(text);

  const gfx::Size minimum_size = label()->GetMinimumSize();
  int expected_height = minimum_size.height() + 10;
  label()->SetLineHeight(expected_height);
  constexpr gfx::Insets kInsets{2, 3, 4, 5};
  expected_height += kInsets.height();
  label()->SetBorder(CreateEmptyBorder(kInsets));
  EXPECT_EQ(expected_height, label()->GetMinimumSize().height());
}

TEST_F(LabelTest, MinimumSizeRespectsLineHeightMultilineWithInsets) {
  base::string16 text(ASCIIToUTF16("This is example text."));
  label()->SetText(text);
  label()->SetMultiLine(true);

  const gfx::Size minimum_size = label()->GetMinimumSize();
  int expected_height = minimum_size.height() + 10;
  label()->SetLineHeight(expected_height);
  constexpr gfx::Insets kInsets{2, 3, 4, 5};
  expected_height += kInsets.height();
  label()->SetBorder(CreateEmptyBorder(kInsets));
  EXPECT_EQ(expected_height, label()->GetMinimumSize().height());
}

TEST_F(LabelTest, ElideBehavior) {
  base::string16 text(ASCIIToUTF16("This is example text."));
  label()->SetText(text);
  EXPECT_EQ(gfx::ELIDE_TAIL, label()->GetElideBehavior());
  gfx::Size size = label()->GetPreferredSize();
  label()->SetBoundsRect(gfx::Rect(size));
  EXPECT_EQ(text, label()->GetDisplayTextForTesting());

  size.set_width(size.width() / 2);
  label()->SetBoundsRect(gfx::Rect(size));
  EXPECT_GT(text.size(), label()->GetDisplayTextForTesting().size());

  label()->SetElideBehavior(gfx::NO_ELIDE);
  EXPECT_EQ(text, label()->GetDisplayTextForTesting());
}

// Test the minimum width of a Label is correct depending on its ElideBehavior,
// including |gfx::NO_ELIDE|.
TEST_F(LabelTest, ElideBehaviorMinimumWidth) {
  base::string16 text(ASCIIToUTF16("This is example text."));
  label()->SetText(text);

  // Default should be |gfx::ELIDE_TAIL|.
  EXPECT_EQ(gfx::ELIDE_TAIL, label()->GetElideBehavior());
  gfx::Size size = label()->GetMinimumSize();
  // Elidable labels have a minimum width that fits |gfx::kEllipsisUTF16|.
  EXPECT_EQ(gfx::Canvas::GetStringWidth(base::string16(gfx::kEllipsisUTF16),
                                        label()->font_list()),
            size.width());
  label()->SetSize(label()->GetMinimumSize());
  EXPECT_GT(text.length(), label()->GetDisplayTextForTesting().length());

  // Truncated labels can take up the size they are given, but not exceed that
  // if the text can't fit.
  label()->SetElideBehavior(gfx::TRUNCATE);
  label()->SetSize(gfx::Size(10, 10));
  size = label()->GetMinimumSize();
  EXPECT_LT(size.width(), label()->size().width());
  EXPECT_GT(text.length(), label()->GetDisplayTextForTesting().length());

  // Non-elidable single-line labels should take up their full text size, since
  // this behavior implies the text should not be cut off.
  EXPECT_FALSE(label()->GetMultiLine());
  label()->SetElideBehavior(gfx::NO_ELIDE);
  size = label()->GetMinimumSize();
  EXPECT_EQ(text.length(), label()->GetDisplayTextForTesting().length());

  label()->SetSize(label()->GetMinimumSize());
  EXPECT_EQ(text, label()->GetDisplayTextForTesting());
}

TEST_F(LabelTest, MultiLineProperty) {
  EXPECT_FALSE(label()->GetMultiLine());
  label()->SetMultiLine(true);
  EXPECT_TRUE(label()->GetMultiLine());
  label()->SetMultiLine(false);
  EXPECT_FALSE(label()->GetMultiLine());
}

TEST_F(LabelTest, ObscuredProperty) {
  base::string16 test_text(ASCIIToUTF16("Password!"));
  label()->SetText(test_text);
  label()->SizeToPreferredSize();

  // The text should be unobscured by default.
  EXPECT_FALSE(label()->GetObscured());
  EXPECT_EQ(test_text, label()->GetDisplayTextForTesting());
  EXPECT_EQ(test_text, label()->GetText());

  label()->SetObscured(true);
  label()->SizeToPreferredSize();
  EXPECT_TRUE(label()->GetObscured());
  EXPECT_EQ(base::string16(test_text.size(),
                           gfx::RenderText::kPasswordReplacementChar),
            label()->GetDisplayTextForTesting());
  EXPECT_EQ(test_text, label()->GetText());

  label()->SetText(test_text + test_text);
  label()->SizeToPreferredSize();
  EXPECT_EQ(base::string16(test_text.size() * 2,
                           gfx::RenderText::kPasswordReplacementChar),
            label()->GetDisplayTextForTesting());
  EXPECT_EQ(test_text + test_text, label()->GetText());

  label()->SetObscured(false);
  label()->SizeToPreferredSize();
  EXPECT_FALSE(label()->GetObscured());
  EXPECT_EQ(test_text + test_text, label()->GetDisplayTextForTesting());
  EXPECT_EQ(test_text + test_text, label()->GetText());
}

TEST_F(LabelTest, ObscuredSurrogatePair) {
  // 'MUSICAL SYMBOL G CLEF': represented in UTF-16 as two characters
  // forming the surrogate pair 0x0001D11E.
  base::string16 test_text = base::UTF8ToUTF16("\xF0\x9D\x84\x9E");
  label()->SetText(test_text);
  label()->SetObscured(true);
  label()->SizeToPreferredSize();
  EXPECT_EQ(base::string16(1, gfx::RenderText::kPasswordReplacementChar),
            label()->GetDisplayTextForTesting());
  EXPECT_EQ(test_text, label()->GetText());
}

// This test case verifies the label preferred size will change based on the
// current layout, which may seem wrong. However many of our code base assumes
// this behavior, therefore this behavior will have to be kept until the code
// with this assumption is fixed. See http://crbug.com/468494 and
// http://crbug.com/467526.
// TODO(mukai): fix the code assuming this behavior and then fix Label
// implementation, and remove this test case.
TEST_F(LabelTest, MultilinePreferredSizeTest) {
  label()->SetText(ASCIIToUTF16("This is an example."));

  gfx::Size single_line_size = label()->GetPreferredSize();

  label()->SetMultiLine(true);
  gfx::Size multi_line_size = label()->GetPreferredSize();
  EXPECT_EQ(single_line_size, multi_line_size);

  int new_width = multi_line_size.width() / 2;
  label()->SetBounds(0, 0, new_width, label()->GetHeightForWidth(new_width));
  gfx::Size new_size = label()->GetPreferredSize();
  EXPECT_GT(multi_line_size.width(), new_size.width());
  EXPECT_LT(multi_line_size.height(), new_size.height());
}

TEST_F(LabelTest, SingleLineGetHeightForWidth) {
  // Even an empty label should take one line worth of height.
  const int line_height = label()->GetLineHeight();
  EXPECT_EQ(line_height, label()->GetHeightForWidth(100));

  // Given any amount of width, the label should take one line.
  label()->SetText(ASCIIToUTF16("This is an example."));
  const int width = label()->GetPreferredSize().width();
  EXPECT_EQ(line_height, label()->GetHeightForWidth(width));
  EXPECT_EQ(line_height, label()->GetHeightForWidth(width * 2));
  EXPECT_EQ(line_height, label()->GetHeightForWidth(width / 2));
  EXPECT_EQ(line_height, label()->GetHeightForWidth(0));
}

TEST_F(LabelTest, MultiLineGetHeightForWidth) {
  // Even an empty label should take one line worth of height.
  label()->SetMultiLine(true);
  const int line_height = label()->GetLineHeight();
  EXPECT_EQ(line_height, label()->GetHeightForWidth(100));

  // Given its preferred width or more, the label should take one line.
  label()->SetText(ASCIIToUTF16("This is an example."));
  const int width = label()->GetPreferredSize().width();
  EXPECT_EQ(line_height, label()->GetHeightForWidth(width));
  EXPECT_EQ(line_height, label()->GetHeightForWidth(width * 2));

  // Given too little width, the required number of lines should increase.
  // Linebreaking will affect this, so sanity-checks are sufficient.
  const int height_for_half_width = label()->GetHeightForWidth(width / 2);
  EXPECT_GT(height_for_half_width, line_height);
  EXPECT_GT(label()->GetHeightForWidth(width / 4), height_for_half_width);

  // Given zero width, the label should take GetMaxLines(); if this is not set,
  // default to one.
  EXPECT_EQ(line_height, label()->GetHeightForWidth(0));
  label()->SetMaxLines(10);
  EXPECT_EQ(line_height * 10, label()->GetHeightForWidth(0));
}

TEST_F(LabelTest, TooltipProperty) {
  label()->SetText(ASCIIToUTF16("My cool string."));

  // Initially, label has no bounds, its text does not fit, and therefore its
  // text should be returned as the tooltip text.
  EXPECT_EQ(label()->GetText(), label()->GetTooltipText(gfx::Point()));

  // While tooltip handling is disabled, GetTooltipText() should fail.
  label()->SetHandlesTooltips(false);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());
  label()->SetHandlesTooltips(true);

  // When set, custom tooltip text should be returned instead of the label's
  // text.
  base::string16 tooltip_text(ASCIIToUTF16("The tooltip!"));
  label()->SetTooltipText(tooltip_text);
  EXPECT_EQ(tooltip_text, label()->GetTooltipText(gfx::Point()));

  // While tooltip handling is disabled, GetTooltipText() should fail.
  label()->SetHandlesTooltips(false);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());
  label()->SetHandlesTooltips(true);

  // When the tooltip text is set to an empty string, the original behavior is
  // restored.
  label()->SetTooltipText(base::string16());
  EXPECT_EQ(label()->GetText(), label()->GetTooltipText(gfx::Point()));

  // While tooltip handling is disabled, GetTooltipText() should fail.
  label()->SetHandlesTooltips(false);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());
  label()->SetHandlesTooltips(true);

  // Make the label big enough to hold the text
  // and expect there to be no tooltip.
  label()->SetBounds(0, 0, 1000, 40);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());

  // Shrinking the single-line label's height shouldn't trigger a tooltip.
  label()->SetBounds(0, 0, 1000, label()->GetPreferredSize().height() / 2);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());

  // Verify that explicitly set tooltip text is shown, regardless of size.
  label()->SetTooltipText(tooltip_text);
  EXPECT_EQ(tooltip_text, label()->GetTooltipText(gfx::Point()));
  // Clear out the explicitly set tooltip text.
  label()->SetTooltipText(base::string16());

  // Shrink the bounds and the tooltip should come back.
  label()->SetBounds(0, 0, 10, 10);
  EXPECT_FALSE(label()->GetTooltipText(gfx::Point()).empty());

  // Make the label obscured and there is no tooltip.
  label()->SetObscured(true);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());

  // Obscuring the text shouldn't permanently clobber the tooltip.
  label()->SetObscured(false);
  EXPECT_FALSE(label()->GetTooltipText(gfx::Point()).empty());

  // Making the label multiline shouldn't eliminate the tooltip.
  label()->SetMultiLine(true);
  EXPECT_FALSE(label()->GetTooltipText(gfx::Point()).empty());
  // Expanding the multiline label bounds should eliminate the tooltip.
  label()->SetBounds(0, 0, 1000, 1000);
  EXPECT_TRUE(label()->GetTooltipText(gfx::Point()).empty());

  // Verify that setting the tooltip still shows it.
  label()->SetTooltipText(tooltip_text);
  EXPECT_EQ(tooltip_text, label()->GetTooltipText(gfx::Point()));
  // Clear out the tooltip.
  label()->SetTooltipText(base::string16());
}

TEST_F(LabelTest, Accessibility) {
  const base::string16 accessible_name = ASCIIToUTF16("A11y text.");

  label()->SetText(ASCIIToUTF16("Displayed text."));

  ui::AXNodeData node_data;
  label()->GetAccessibleNodeData(&node_data);
  EXPECT_EQ(ax::mojom::Role::kStaticText, node_data.role);
  EXPECT_EQ(label()->GetText(),
            node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));
  EXPECT_FALSE(
      node_data.HasIntAttribute(ax::mojom::IntAttribute::kRestriction));

  // Setting a custom accessible name overrides the displayed text in
  // screen reader announcements.
  label()->SetAccessibleName(accessible_name);

  label()->GetAccessibleNodeData(&node_data);
  EXPECT_EQ(accessible_name,
            node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));
  EXPECT_NE(label()->GetText(),
            node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));

  // Changing the displayed text will not impact the non-empty accessible name.
  label()->SetText(ASCIIToUTF16("Different displayed Text."));

  label()->GetAccessibleNodeData(&node_data);
  EXPECT_EQ(accessible_name,
            node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));
  EXPECT_NE(label()->GetText(),
            node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));

  // Clearing the accessible name will cause the screen reader to default to
  // verbalizing the displayed text.
  label()->SetAccessibleName(ASCIIToUTF16(""));

  label()->GetAccessibleNodeData(&node_data);
  EXPECT_EQ(label()->GetText(),
            node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));
}

TEST_F(LabelTest, TextChangeWithoutLayout) {
  label()->SetText(ASCIIToUTF16("Example"));
  label()->SetBounds(0, 0, 200, 200);

  gfx::Canvas canvas(gfx::Size(200, 200), 1.0f, true);
  label()->OnPaint(&canvas);
  EXPECT_TRUE(label()->display_text_);
  EXPECT_EQ(ASCIIToUTF16("Example"), label()->display_text_->GetDisplayText());

  label()->SetText(ASCIIToUTF16("Altered"));
  // The altered text should be painted even though Layout() or SetBounds() are
  // not called.
  label()->OnPaint(&canvas);
  EXPECT_TRUE(label()->display_text_);
  EXPECT_EQ(ASCIIToUTF16("Altered"), label()->display_text_->GetDisplayText());
}

TEST_F(LabelTest, EmptyLabelSizing) {
  const gfx::Size expected_size(0, label()->font_list().GetHeight());
  EXPECT_EQ(expected_size, label()->GetPreferredSize());
  label()->SetMultiLine(!label()->GetMultiLine());
  EXPECT_EQ(expected_size, label()->GetPreferredSize());
}

TEST_F(LabelTest, SingleLineSizing) {
  label()->SetText(ASCIIToUTF16("A not so random string in one line."));
  const gfx::Size size = label()->GetPreferredSize();
  EXPECT_GT(size.height(), kMinTextDimension);
  EXPECT_GT(size.width(), kMinTextDimension);

  // Setting a size smaller than preferred should not change the preferred size.
  label()->SetSize(gfx::Size(size.width() / 2, size.height() / 2));
  EXPECT_EQ(size, label()->GetPreferredSize());

  const gfx::Insets border(10, 20, 30, 40);
  label()->SetBorder(CreateEmptyBorder(border));
  const gfx::Size size_with_border = label()->GetPreferredSize();
  EXPECT_EQ(size_with_border.height(), size.height() + border.height());
  EXPECT_EQ(size_with_border.width(), size.width() + border.width());
  EXPECT_EQ(size.height() + border.height(),
            label()->GetHeightForWidth(size_with_border.width()));
}

TEST_F(LabelTest, MultilineSmallAvailableWidthSizing) {
  label()->SetMultiLine(true);
  label()->SetAllowCharacterBreak(true);
  label()->SetText(ASCIIToUTF16("Too Wide."));

  // Check that Label can be laid out at a variety of small sizes,
  // splitting the words into up to one character per line if necessary.
  // Incorrect word splitting may cause infinite loops in text layout.
  gfx::Size required_size = label()->GetPreferredSize();
  for (int i = 1; i < required_size.width(); ++i)
    EXPECT_GT(label()->GetHeightForWidth(i), 0);
}

// Verifies if SetAllowCharacterBreak(true) doesn't change the preferred size.
// See crbug.com/469559
TEST_F(LabelTest, PreferredSizeForAllowCharacterBreak) {
  label()->SetText(base::ASCIIToUTF16("Example"));
  gfx::Size preferred_size = label()->GetPreferredSize();

  label()->SetMultiLine(true);
  label()->SetAllowCharacterBreak(true);
  EXPECT_EQ(preferred_size, label()->GetPreferredSize());
}

TEST_F(LabelTest, MultiLineSizing) {
  label()->SetText(
      ASCIIToUTF16("A random string\nwith multiple lines\nand returns!"));
  label()->SetMultiLine(true);

  // GetPreferredSize
  gfx::Size required_size = label()->GetPreferredSize();
  EXPECT_GT(required_size.height(), kMinTextDimension);
  EXPECT_GT(required_size.width(), kMinTextDimension);

  // SizeToFit with unlimited width.
  label()->SizeToFit(0);
  int required_width = label()->GetLocalBounds().width();
  EXPECT_GT(required_width, kMinTextDimension);

  // SizeToFit with limited width.
  label()->SizeToFit(required_width - 1);
  int constrained_width = label()->GetLocalBounds().width();
#if defined(OS_WIN)
  // Canvas::SizeStringInt (in ui/gfx/canvas_linux.cc)
  // has to be fixed to return the size that fits to given width/height.
  EXPECT_LT(constrained_width, required_width);
#endif
  EXPECT_GT(constrained_width, kMinTextDimension);

  // Change the width back to the desire width.
  label()->SizeToFit(required_width);
  EXPECT_EQ(required_width, label()->GetLocalBounds().width());

  // General tests for GetHeightForWidth.
  int required_height = label()->GetHeightForWidth(required_width);
  EXPECT_GT(required_height, kMinTextDimension);
  int height_for_constrained_width =
      label()->GetHeightForWidth(constrained_width);
#if defined(OS_WIN)
  // Canvas::SizeStringInt (in ui/gfx/canvas_linux.cc)
  // has to be fixed to return the size that fits to given width/height.
  EXPECT_GT(height_for_constrained_width, required_height);
#endif
  // Using the constrained width or the required_width - 1 should give the
  // same result for the height because the constrainted width is the tight
  // width when given "required_width - 1" as the max width.
  EXPECT_EQ(height_for_constrained_width,
            label()->GetHeightForWidth(required_width - 1));

  // Test everything with borders.
  gfx::Insets border(10, 20, 30, 40);
  label()->SetBorder(CreateEmptyBorder(border));

  // SizeToFit and borders.
  label()->SizeToFit(0);
  int required_width_with_border = label()->GetLocalBounds().width();
  EXPECT_EQ(required_width_with_border, required_width + border.width());

  // GetHeightForWidth and borders.
  int required_height_with_border =
      label()->GetHeightForWidth(required_width_with_border);
  EXPECT_EQ(required_height_with_border, required_height + border.height());

  // Test that the border width is subtracted before doing the height
  // calculation.  If it is, then the height will grow when width
  // is shrunk.
  int height1 = label()->GetHeightForWidth(required_width_with_border - 1);
#if defined(OS_WIN)
  // Canvas::SizeStringInt (in ui/gfx/canvas_linux.cc)
  // has to be fixed to return the size that fits to given width/height.
  EXPECT_GT(height1, required_height_with_border);
#endif
  EXPECT_EQ(height1, height_for_constrained_width + border.height());

  // GetPreferredSize and borders.
  label()->SetBounds(0, 0, 0, 0);
  gfx::Size required_size_with_border = label()->GetPreferredSize();
  EXPECT_EQ(required_size_with_border.height(),
            required_size.height() + border.height());
  EXPECT_EQ(required_size_with_border.width(),
            required_size.width() + border.width());
}

#if !defined(OS_APPLE)
// TODO(warx): Remove !defined(OS_APPLE) once SetMaxLines() is applied to MAC
// (crbug.com/758720).
TEST_F(LabelTest, MultiLineSetMaxLines) {
  // Ensure SetMaxLines clamps the line count of a string with returns.
  label()->SetText(ASCIIToUTF16("first line\nsecond line\nthird line"));
  label()->SetMultiLine(true);
  gfx::Size string_size = label()->GetPreferredSize();
  label()->SetMaxLines(2);
  gfx::Size two_line_size = label()->GetPreferredSize();
  EXPECT_EQ(string_size.width(), two_line_size.width());
  EXPECT_GT(string_size.height(), two_line_size.height());

  // Ensure GetHeightForWidth also respects SetMaxLines.
  int height = label()->GetHeightForWidth(string_size.width() / 2);
  EXPECT_EQ(height, two_line_size.height());

  // Ensure SetMaxLines also works with line wrapping for SizeToFit.
  label()->SetText(ASCIIToUTF16("A long string that will be wrapped"));
  label()->SetMaxLines(0);  // Used to get the uncapped height.
  label()->SizeToFit(0);    // Used to get the uncapped width.
  label()->SizeToFit(label()->GetPreferredSize().width() / 4);
  string_size = label()->GetPreferredSize();
  label()->SetMaxLines(2);
  two_line_size = label()->GetPreferredSize();
  EXPECT_EQ(string_size.width(), two_line_size.width());
  EXPECT_GT(string_size.height(), two_line_size.height());

  // Ensure SetMaxLines also works with line wrapping for SetMaximumWidth.
  label()->SetMaxLines(0);  // Used to get the uncapped height.
  label()->SizeToFit(0);    // Used to get the uncapped width.
  label()->SetMaximumWidth(label()->GetPreferredSize().width() / 4);
  string_size = label()->GetPreferredSize();
  label()->SetMaxLines(2);
  two_line_size = label()->GetPreferredSize();
  EXPECT_EQ(string_size.width(), two_line_size.width());
  EXPECT_GT(string_size.height(), two_line_size.height());

  // Ensure SetMaxLines respects the requested inset height.
  const gfx::Insets border(1, 2, 3, 4);
  label()->SetBorder(CreateEmptyBorder(border));
  EXPECT_EQ(two_line_size.height() + border.height(),
            label()->GetPreferredSize().height());
}
#endif

// Verifies if the combination of text eliding and multiline doesn't cause
// any side effects of size / layout calculation.
TEST_F(LabelTest, MultiLineSizingWithElide) {
  const base::string16 text =
      ASCIIToUTF16("A random string\nwith multiple lines\nand returns!");
  label()->SetText(text);
  label()->SetMultiLine(true);

  gfx::Size required_size = label()->GetPreferredSize();
  EXPECT_GT(required_size.height(), kMinTextDimension);
  EXPECT_GT(required_size.width(), kMinTextDimension);
  label()->SetBoundsRect(gfx::Rect(required_size));

  label()->SetElideBehavior(gfx::ELIDE_TAIL);
  EXPECT_EQ(required_size, label()->GetPreferredSize());
  EXPECT_EQ(text, label()->GetDisplayTextForTesting());

  label()->SizeToFit(required_size.width() - 1);
  gfx::Size narrow_size = label()->GetPreferredSize();
  EXPECT_GT(required_size.width(), narrow_size.width());
  EXPECT_LT(required_size.height(), narrow_size.height());

  // SetBounds() doesn't change the preferred size.
  label()->SetBounds(0, 0, narrow_size.width() - 1, narrow_size.height());
  EXPECT_EQ(narrow_size, label()->GetPreferredSize());

  // Paint() doesn't change the preferred size.
  gfx::Canvas canvas;
  label()->OnPaint(&canvas);
  EXPECT_EQ(narrow_size, label()->GetPreferredSize());
}

// Check that labels support GetTooltipHandlerForPoint.
TEST_F(LabelTest, GetTooltipHandlerForPoint) {
  label()->SetText(
      ASCIIToUTF16("A string that's long enough to exceed the bounds"));
  label()->SetBounds(0, 0, 10, 10);

  // By default, labels start out as tooltip handlers.
  ASSERT_TRUE(label()->GetHandlesTooltips());

  // There's a default tooltip if the text is too big to fit.
  EXPECT_EQ(label(), label()->GetTooltipHandlerForPoint(gfx::Point(2, 2)));

  // If tooltip handling is disabled, the label should not provide a tooltip
  // handler.
  label()->SetHandlesTooltips(false);
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(2, 2)));
  label()->SetHandlesTooltips(true);

  // If there's no default tooltip, this should return NULL.
  label()->SetBounds(0, 0, 500, 50);
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(2, 2)));

  label()->SetTooltipText(ASCIIToUTF16("a tooltip"));
  // If the point hits the label, and tooltip is set, the label should be
  // returned as its tooltip handler.
  EXPECT_EQ(label(), label()->GetTooltipHandlerForPoint(gfx::Point(2, 2)));

  // Additionally, GetTooltipHandlerForPoint should verify that the label
  // actually contains the point.
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(2, 51)));
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(-1, 20)));

  // Again, if tooltip handling is disabled, the label should not provide a
  // tooltip handler.
  label()->SetHandlesTooltips(false);
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(2, 2)));
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(2, 51)));
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(-1, 20)));
  label()->SetHandlesTooltips(true);

  // GetTooltipHandlerForPoint works should work in child bounds.
  label()->SetBounds(2, 2, 10, 10);
  EXPECT_EQ(label(), label()->GetTooltipHandlerForPoint(gfx::Point(1, 5)));
  EXPECT_FALSE(label()->GetTooltipHandlerForPoint(gfx::Point(3, 11)));
}

// Check that label releases its internal layout data when it's unnecessary.
TEST_F(LabelTest, ResetRenderTextData) {
  label()->SetText(ASCIIToUTF16("Example"));
  label()->SizeToPreferredSize();
  gfx::Size preferred_size = label()->GetPreferredSize();

  EXPECT_NE(gfx::Size(), preferred_size);
  EXPECT_FALSE(label()->display_text_);

  gfx::Canvas canvas(preferred_size, 1.0f, true);
  label()->OnPaint(&canvas);
  EXPECT_TRUE(label()->display_text_);

  // Label should recreate its RenderText object when it's invisible, to release
  // the layout structures and data.
  label()->SetVisible(false);
  EXPECT_FALSE(label()->display_text_);

  // Querying fields or size information should not recompute the layout
  // unnecessarily.
  EXPECT_EQ(ASCIIToUTF16("Example"), label()->GetText());
  EXPECT_FALSE(label()->display_text_);

  EXPECT_EQ(preferred_size, label()->GetPreferredSize());
  EXPECT_FALSE(label()->display_text_);

  // RenderText data should be back when it's necessary.
  label()->SetVisible(true);
  EXPECT_FALSE(label()->display_text_);

  label()->OnPaint(&canvas);
  EXPECT_TRUE(label()->display_text_);

  // Changing layout just resets |display_text_|. It'll recover next time it's
  // drawn.
  label()->SetBounds(0, 0, 10, 10);
  EXPECT_FALSE(label()->display_text_);

  label()->OnPaint(&canvas);
  EXPECT_TRUE(label()->display_text_);
}

TEST_F(LabelTest, MultilineSupportedRenderText) {
  label()->SetText(ASCIIToUTF16("Example of\nmultilined label"));
  label()->SetMultiLine(true);
  label()->SizeToPreferredSize();

  gfx::Canvas canvas(label()->GetPreferredSize(), 1.0f, true);
  label()->OnPaint(&canvas);

  // There's only RenderText instance, which should have multiple lines.
  ASSERT_TRUE(label()->display_text_);
  EXPECT_EQ(2u, label()->display_text_->GetNumLines());
}

// Ensures SchedulePaint() calls are not made in OnPaint().
TEST_F(LabelTest, NoSchedulePaintInOnPaint) {
  TestLabel label;
  int count = 0;
  const auto expect_paint_count_increased = [&]() {
    EXPECT_GT(label.schedule_paint_count(), count);
    count = label.schedule_paint_count();
  };

  // Initialization should schedule at least one paint, but the precise number
  // doesn't really matter.
  expect_paint_count_increased();

  // Painting should never schedule another paint.
  label.SimulatePaint();
  EXPECT_EQ(count, label.schedule_paint_count());

  // Test a few things that should schedule paints. Multiple times is OK.
  label.SetEnabled(false);
  expect_paint_count_increased();

  label.SetText(label.GetText() + ASCIIToUTF16("Changed"));
  expect_paint_count_increased();

  label.SizeToPreferredSize();
  expect_paint_count_increased();

  label.SetEnabledColor(SK_ColorBLUE);
  expect_paint_count_increased();

  label.SimulatePaint();
  EXPECT_EQ(count, label.schedule_paint_count());  // Unchanged.
}

TEST_F(LabelTest, EmptyLabel) {
  label()->SetFocusBehavior(View::FocusBehavior::ALWAYS);
  label()->RequestFocus();
  label()->SizeToPreferredSize();
  EXPECT_TRUE(label()->size().IsEmpty());

  // With no text, neither links nor labels have a size in any dimension.
  Link concrete_link;
  EXPECT_TRUE(concrete_link.GetPreferredSize().IsEmpty());
}

TEST_F(LabelTest, CanForceDirectionality) {
  Label bidi_text_force_url(ToRTL("0123456") + base::ASCIIToUTF16(".com"), 0,
                            style::STYLE_PRIMARY,
                            gfx::DirectionalityMode::DIRECTIONALITY_AS_URL);
  EXPECT_EQ(base::i18n::TextDirection::LEFT_TO_RIGHT,
            bidi_text_force_url.GetTextDirectionForTesting());

  Label rtl_text_force_ltr(ToRTL("0123456"), 0, style::STYLE_PRIMARY,
                           gfx::DirectionalityMode::DIRECTIONALITY_FORCE_LTR);
  EXPECT_EQ(base::i18n::TextDirection::LEFT_TO_RIGHT,
            rtl_text_force_ltr.GetTextDirectionForTesting());

  Label ltr_text_force_rtl(base::ASCIIToUTF16("0123456"), 0,
                           style::STYLE_PRIMARY,
                           gfx::DirectionalityMode::DIRECTIONALITY_FORCE_RTL);
  EXPECT_EQ(base::i18n::TextDirection::RIGHT_TO_LEFT,
            ltr_text_force_rtl.GetTextDirectionForTesting());

  SetRTL(true);
  Label ltr_use_ui(base::ASCIIToUTF16("0123456"), 0, style::STYLE_PRIMARY,
                   gfx::DirectionalityMode::DIRECTIONALITY_FROM_UI);
  EXPECT_EQ(base::i18n::TextDirection::RIGHT_TO_LEFT,
            ltr_use_ui.GetTextDirectionForTesting());

  SetRTL(false);
  Label rtl_use_ui(ToRTL("0123456"), 0, style::STYLE_PRIMARY,
                   gfx::DirectionalityMode::DIRECTIONALITY_FROM_UI);
  EXPECT_EQ(base::i18n::TextDirection::LEFT_TO_RIGHT,
            rtl_use_ui.GetTextDirectionForTesting());
}

TEST_F(LabelTest, DefaultDirectionalityIsFromText) {
  Label ltr(base::ASCIIToUTF16("Foo"));
  EXPECT_EQ(base::i18n::TextDirection::LEFT_TO_RIGHT,
            ltr.GetTextDirectionForTesting());

  Label rtl(ToRTL("0123456"));
  EXPECT_EQ(base::i18n::TextDirection::RIGHT_TO_LEFT,
            rtl.GetTextDirectionForTesting());
}

TEST_F(LabelTest, IsDisplayTextTruncated) {
  const base::string16 text = ASCIIToUTF16("A random string");
  label()->SetText(text);

  gfx::Size zero_size;
  label()->SetElideBehavior(gfx::ELIDE_TAIL);
  label()->SetBoundsRect(gfx::Rect(zero_size));
  EXPECT_TRUE(label()->IsDisplayTextTruncated());

  label()->SetElideBehavior(gfx::NO_ELIDE);
  EXPECT_TRUE(label()->IsDisplayTextTruncated());

  gfx::Size minimum_size(1, 1);
  label()->SetBoundsRect(gfx::Rect(minimum_size));
  EXPECT_TRUE(label()->IsDisplayTextTruncated());

  gfx::Size enough_size(100, 100);
  label()->SetBoundsRect(gfx::Rect(enough_size));
  EXPECT_FALSE(label()->IsDisplayTextTruncated());

  const base::string16 empty_text;
  label()->SetText(empty_text);
  EXPECT_FALSE(label()->IsDisplayTextTruncated());
  label()->SetBoundsRect(gfx::Rect(zero_size));
  EXPECT_FALSE(label()->IsDisplayTextTruncated());
}

TEST_F(LabelTest, TextChangedCallback) {
  bool text_changed = false;
  auto subscription = label()->AddTextChangedCallback(base::BindRepeating(
      [](bool* text_changed) { *text_changed = true; }, &text_changed));

  label()->SetText(ASCIIToUTF16("abc"));
  EXPECT_TRUE(text_changed);
}

// Verify that GetSubstringBounds returns the correct bounds, accounting for
// label insets.
TEST_F(LabelTest, GetSubstringBounds) {
  label()->SetText(ASCIIToUTF16("abc"));
  auto substring_bounds = label()->GetSubstringBounds(gfx::Range(0, 3));
  EXPECT_EQ(1u, substring_bounds.size());

  gfx::Insets insets{2, 3, 4, 5};
  label()->SetBorder(CreateEmptyBorder(insets));
  auto substring_bounds_with_inset =
      label()->GetSubstringBounds(gfx::Range(0, 3));
  EXPECT_EQ(1u, substring_bounds_with_inset.size());
  EXPECT_EQ(substring_bounds[0].x() + 3, substring_bounds_with_inset[0].x());
  EXPECT_EQ(substring_bounds[0].y() + 2, substring_bounds_with_inset[0].y());
  EXPECT_EQ(substring_bounds[0].width(),
            substring_bounds_with_inset[0].width());
  EXPECT_EQ(substring_bounds[0].height(),
            substring_bounds_with_inset[0].height());
}

// TODO(crbug.com/1139395): Enable on ChromeOS along with the DCHECK in Label.
#if !BUILDFLAG(IS_CHROMEOS_ASH)
// Ensures DCHECK for subpixel rendering on transparent layer is working.
TEST_F(LabelTest, ChecksSubpixelRenderingOntoOpaqueSurface) {
  View view;
  Label* label = view.AddChildView(std::make_unique<TestLabel>());
  EXPECT_TRUE(label->GetSubpixelRenderingEnabled());

  gfx::Canvas canvas;

  // Painting on a view not painted to a layer should be fine.
  label->OnPaint(&canvas);

  // Painting to an opaque layer should also be fine.
  view.SetPaintToLayer();
  label->OnPaint(&canvas);

  // Set up a transparent layer for the parent view.
  view.layer()->SetFillsBoundsOpaquely(false);

  // Painting on a transparent layer should DCHECK.
  EXPECT_DCHECK_DEATH(label->OnPaint(&canvas));

  // We should not DCHECK if the check is skipped.
  label->SetSkipSubpixelRenderingOpacityCheck(true);
  label->OnPaint(&canvas);
  label->SetSkipSubpixelRenderingOpacityCheck(false);

  // Painting onto a transparent layer should not DCHECK if there's an opaque
  // background in a parent of the Label.
  view.SetBackground(CreateSolidBackground(SK_ColorWHITE));
  label->OnPaint(&canvas);
}
#endif  // !BUILDFLAG(IS_CHROMEOS_ASH)

TEST_F(LabelSelectionTest, Selectable) {
  // By default, labels don't support text selection.
  EXPECT_FALSE(label()->GetSelectable());

  ASSERT_TRUE(label()->SetSelectable(true));
  EXPECT_TRUE(label()->GetSelectable());

  // Verify that making a label multiline still causes the label to support text
  // selection.
  label()->SetMultiLine(true);
  EXPECT_TRUE(label()->GetSelectable());

  // Verify that obscuring the label text causes the label to not support text
  // selection.
  label()->SetObscured(true);
  EXPECT_FALSE(label()->GetSelectable());
}

// Verify that labels supporting text selection get focus on clicks.
TEST_F(LabelSelectionTest, FocusOnClick) {
  label()->SetText(ASCIIToUTF16("text"));
  label()->SizeToPreferredSize();

  // By default, labels don't get focus on click.
  PerformClick(gfx::Point());
  EXPECT_NE(label(), GetFocusedView());

  ASSERT_TRUE(label()->SetSelectable(true));
  PerformClick(gfx::Point());
  EXPECT_EQ(label(), GetFocusedView());
}

// Verify that labels supporting text selection do not get focus on tab
// traversal by default.
TEST_F(LabelSelectionTest, FocusTraversal) {
  // Add another view before |label()|.
  View* view = new View();
  view->SetFocusBehavior(View::FocusBehavior::ALWAYS);
  widget()->GetContentsView()->AddChildViewAt(view, 0);

  // By default, labels are not focusable.
  view->RequestFocus();
  EXPECT_EQ(view, GetFocusedView());
  widget()->GetFocusManager()->AdvanceFocus(false);
  EXPECT_NE(label(), GetFocusedView());

  // On enabling text selection, labels can get focus on clicks but not via tab
  // traversal.
  view->RequestFocus();
  EXPECT_EQ(view, GetFocusedView());
  EXPECT_TRUE(label()->SetSelectable(true));
  widget()->GetFocusManager()->AdvanceFocus(false);
  EXPECT_NE(label(), GetFocusedView());

  // A label with FocusBehavior::ALWAYS should get focus via tab traversal.
  view->RequestFocus();
  EXPECT_EQ(view, GetFocusedView());
  EXPECT_TRUE(label()->SetSelectable(false));
  label()->SetFocusBehavior(View::FocusBehavior::ALWAYS);
  widget()->GetFocusManager()->AdvanceFocus(false);
  EXPECT_EQ(label(), GetFocusedView());
}

// Verify label text selection behavior on double and triple clicks.
TEST_F(LabelSelectionTest, DoubleTripleClick) {
  label()->SetText(ASCIIToUTF16("Label double click"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));

  PerformClick(GetCursorPoint(0));
  EXPECT_TRUE(GetSelectedText().empty());

  // Double clicking should select the word under cursor.
  PerformClick(GetCursorPoint(0));
  EXPECT_STR_EQ("Label", GetSelectedText());

  // Triple clicking should select all the text.
  PerformClick(GetCursorPoint(0));
  EXPECT_EQ(label()->GetText(), GetSelectedText());

  // Clicking again should alternate to double click.
  PerformClick(GetCursorPoint(0));
  EXPECT_STR_EQ("Label", GetSelectedText());

  // Clicking at another location should clear the selection.
  PerformClick(GetCursorPoint(8));
  EXPECT_TRUE(GetSelectedText().empty());
  PerformClick(GetCursorPoint(8));
  EXPECT_STR_EQ("double", GetSelectedText());
}

// Verify label text selection behavior on mouse drag.
TEST_F(LabelSelectionTest, MouseDrag) {
  label()->SetText(ASCIIToUTF16("Label mouse drag"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));

  PerformMousePress(GetCursorPoint(5));
  PerformMouseDragTo(GetCursorPoint(0));
  EXPECT_STR_EQ("Label", GetSelectedText());

  PerformMouseDragTo(GetCursorPoint(8));
  EXPECT_STR_EQ(" mo", GetSelectedText());

  PerformMouseDragTo(gfx::Point(200, GetCursorPoint(0).y()));
  PerformMouseRelease(gfx::Point(200, GetCursorPoint(0).y()));
  EXPECT_STR_EQ(" mouse drag", GetSelectedText());

  event_generator()->PressKey(ui::VKEY_C, kControlCommandModifier);
  EXPECT_STR_EQ(" mouse drag",
                GetClipboardText(ui::ClipboardBuffer::kCopyPaste));
}

TEST_F(LabelSelectionTest, MouseDragMultilineLTR) {
  label()->SetMultiLine(true);
  label()->SetText(ASCIIToUTF16("abcd\nefgh"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));
  ASSERT_EQ(2u, GetLineCount());

  PerformMousePress(GetCursorPoint(2));
  PerformMouseDragTo(GetCursorPoint(0));
  EXPECT_STR_EQ("ab", GetSelectedText());

  PerformMouseDragTo(GetCursorPoint(7));
  EXPECT_STR_EQ("cd\nef", GetSelectedText());

  PerformMouseDragTo(gfx::Point(-5, GetCursorPoint(6).y()));
  EXPECT_STR_EQ("cd\n", GetSelectedText());

  PerformMouseDragTo(gfx::Point(100, GetCursorPoint(6).y()));
  EXPECT_STR_EQ("cd\nefgh", GetSelectedText());

  const gfx::Point points[] = {
      {GetCursorPoint(1).x(), -5},   // NW.
      {GetCursorPoint(2).x(), -5},   // NORTH.
      {GetCursorPoint(3).x(), -5},   // NE.
      {GetCursorPoint(8).x(), 100},  // SE.
      {GetCursorPoint(7).x(), 100},  // SOUTH.
      {GetCursorPoint(6).x(), 100},  // SW.
  };
  constexpr const char* kExtendLeft = "ab";
  constexpr const char* kExtendRight = "cd\nefgh";

  // For multiline, N* extends left, S* extends right.
  PerformMouseDragTo(points[NW]);
  EXPECT_STR_EQ(kExtends ? kExtendLeft : "b", GetSelectedText());
  PerformMouseDragTo(points[NORTH]);
  EXPECT_STR_EQ(kExtends ? kExtendLeft : "", GetSelectedText());
  PerformMouseDragTo(points[NE]);
  EXPECT_STR_EQ(kExtends ? kExtendLeft : "c", GetSelectedText());
  PerformMouseDragTo(points[SE]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "cd\nefg", GetSelectedText());
  PerformMouseDragTo(points[SOUTH]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "cd\nef", GetSelectedText());
  PerformMouseDragTo(points[SW]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "cd\ne", GetSelectedText());
}

// Single line fields consider the x offset as well. Ties go to the right.
TEST_F(LabelSelectionTest, MouseDragSingleLineLTR) {
  label()->SetText(ASCIIToUTF16("abcdef"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));
  PerformMousePress(GetCursorPoint(2));
  const gfx::Point points[] = {
      {GetCursorPoint(1).x(), -5},   // NW.
      {GetCursorPoint(2).x(), -5},   // NORTH.
      {GetCursorPoint(3).x(), -5},   // NE.
      {GetCursorPoint(3).x(), 100},  // SE.
      {GetCursorPoint(2).x(), 100},  // SOUTH.
      {GetCursorPoint(1).x(), 100},  // SW.
  };
  constexpr const char* kExtendLeft = "ab";
  constexpr const char* kExtendRight = "cdef";

  // For single line, western directions extend left, all others extend right.
  PerformMouseDragTo(points[NW]);
  EXPECT_STR_EQ(kExtends ? kExtendLeft : "b", GetSelectedText());
  PerformMouseDragTo(points[NORTH]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "", GetSelectedText());
  PerformMouseDragTo(points[NE]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "c", GetSelectedText());
  PerformMouseDragTo(points[SE]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "c", GetSelectedText());
  PerformMouseDragTo(points[SOUTH]);
  EXPECT_STR_EQ(kExtends ? kExtendRight : "", GetSelectedText());
  PerformMouseDragTo(points[SW]);
  EXPECT_STR_EQ(kExtends ? kExtendLeft : "b", GetSelectedText());
}

TEST_F(LabelSelectionTest, MouseDragMultilineRTL) {
  label()->SetMultiLine(true);
  label()->SetText(ToRTL("012\n345"));
  // Sanity check.
  EXPECT_EQ(WideToUTF16(L"\x5d0\x5d1\x5d2\n\x5d3\x5d4\x5d5"),
            label()->GetText());

  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));
  ASSERT_EQ(2u, GetLineCount());

  PerformMousePress(GetCursorPoint(1));  // Note: RTL drag starts at 1, not 2.
  PerformMouseDragTo(GetCursorPoint(0));
  EXPECT_EQ(ToRTL("0"), GetSelectedText());

  PerformMouseDragTo(GetCursorPoint(6));
  EXPECT_EQ(ToRTL("12\n34"), GetSelectedText());

  PerformMouseDragTo(gfx::Point(-5, GetCursorPoint(6).y()));
  EXPECT_EQ(ToRTL("12\n345"), GetSelectedText());

  PerformMouseDragTo(gfx::Point(100, GetCursorPoint(6).y()));
  EXPECT_EQ(ToRTL("12\n"), GetSelectedText());

  const gfx::Point points[] = {
      {GetCursorPoint(2).x(), -5},   // NW: Now towards the end of the string.
      {GetCursorPoint(1).x(), -5},   // NORTH,
      {GetCursorPoint(0).x(), -5},   // NE: Towards the start.
      {GetCursorPoint(4).x(), 100},  // SE.
      {GetCursorPoint(5).x(), 100},  // SOUTH.
      {GetCursorPoint(6).x(), 100},  // SW.
  };

  // Visual right, so to the beginning of the string for RTL.
  const base::string16 extend_right = ToRTL("0");
  const base::string16 extend_left = ToRTL("12\n345");

  // For multiline, N* extends right, S* extends left.
  PerformMouseDragTo(points[NW]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL("1"), GetSelectedText());
  PerformMouseDragTo(points[NORTH]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL(""), GetSelectedText());
  PerformMouseDragTo(points[NE]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL("0"), GetSelectedText());
  PerformMouseDragTo(points[SE]);
  EXPECT_EQ(kExtends ? extend_left : ToRTL("12\n"), GetSelectedText());
  PerformMouseDragTo(points[SOUTH]);
  EXPECT_EQ(kExtends ? extend_left : ToRTL("12\n3"), GetSelectedText());
  PerformMouseDragTo(points[SW]);
  EXPECT_EQ(kExtends ? extend_left : ToRTL("12\n34"), GetSelectedText());
}

TEST_F(LabelSelectionTest, MouseDragSingleLineRTL) {
  label()->SetText(ToRTL("0123456"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));

  PerformMousePress(GetCursorPoint(1));
  const gfx::Point points[] = {
      {GetCursorPoint(2).x(), -5},   // NW.
      {GetCursorPoint(1).x(), -5},   // NORTH.
      {GetCursorPoint(0).x(), -5},   // NE.
      {GetCursorPoint(0).x(), 100},  // SE.
      {GetCursorPoint(1).x(), 100},  // SOUTH.
      {GetCursorPoint(2).x(), 100},  // SW.
  };

  // Visual right, so to the beginning of the string for RTL.
  const base::string16 extend_right = ToRTL("0");
  const base::string16 extend_left = ToRTL("123456");

  // For single line, western directions extend left, all others extend right.
  PerformMouseDragTo(points[NW]);
  EXPECT_EQ(kExtends ? extend_left : ToRTL("1"), GetSelectedText());
  PerformMouseDragTo(points[NORTH]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL(""), GetSelectedText());
  PerformMouseDragTo(points[NE]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL("0"), GetSelectedText());
  PerformMouseDragTo(points[SE]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL("0"), GetSelectedText());
  PerformMouseDragTo(points[SOUTH]);
  EXPECT_EQ(kExtends ? extend_right : ToRTL(""), GetSelectedText());
  PerformMouseDragTo(points[SW]);
  EXPECT_EQ(kExtends ? extend_left : ToRTL("1"), GetSelectedText());
}

// Verify the initially selected word on a double click, remains selected on
// mouse dragging.
TEST_F(LabelSelectionTest, MouseDragWord) {
  label()->SetText(ASCIIToUTF16("Label drag word"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));

  PerformClick(GetCursorPoint(8));
  PerformMousePress(GetCursorPoint(8));
  EXPECT_STR_EQ("drag", GetSelectedText());

  PerformMouseDragTo(GetCursorPoint(0));
  EXPECT_STR_EQ("Label drag", GetSelectedText());

  PerformMouseDragTo(gfx::Point(200, GetCursorPoint(0).y()));
  PerformMouseRelease(gfx::Point(200, GetCursorPoint(0).y()));
  EXPECT_STR_EQ("drag word", GetSelectedText());
}

// TODO(crbug.com/1052397): Revisit the macro expression once build flag switch
// of lacros-chrome is complete.
#if defined(OS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
// Verify selection clipboard behavior on text selection.
TEST_F(LabelSelectionTest, SelectionClipboard) {
  label()->SetText(ASCIIToUTF16("Label selection clipboard"));
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));

  // Verify programmatic modification of selection, does not modify the
  // selection clipboard.
  label()->SelectRange(gfx::Range(2, 5));
  EXPECT_STR_EQ("bel", GetSelectedText());
  EXPECT_TRUE(GetClipboardText(ui::ClipboardBuffer::kSelection).empty());

  // Verify text selection using the mouse updates the selection clipboard.
  PerformMousePress(GetCursorPoint(5));
  PerformMouseDragTo(GetCursorPoint(0));
  PerformMouseRelease(GetCursorPoint(0));
  EXPECT_STR_EQ("Label", GetSelectedText());
  EXPECT_STR_EQ("Label", GetClipboardText(ui::ClipboardBuffer::kSelection));
}
#endif

// Verify that keyboard shortcuts for Copy and Select All work when a selectable
// label is focused.
TEST_F(LabelSelectionTest, KeyboardActions) {
  const base::string16 initial_text = ASCIIToUTF16("Label keyboard actions");
  label()->SetText(initial_text);
  label()->SizeToPreferredSize();
  ASSERT_TRUE(label()->SetSelectable(true));

  PerformClick(gfx::Point());
  EXPECT_EQ(label(), GetFocusedView());

  event_generator()->PressKey(ui::VKEY_A, kControlCommandModifier);
  EXPECT_EQ(initial_text, GetSelectedText());

  event_generator()->PressKey(ui::VKEY_C, kControlCommandModifier);
  EXPECT_EQ(initial_text, GetClipboardText(ui::ClipboardBuffer::kCopyPaste));

  // The selection should get cleared on changing the text, but focus should not
  // be affected.
  const base::string16 new_text = ASCIIToUTF16("Label obscured text");
  label()->SetText(new_text);
  EXPECT_FALSE(label()->HasSelection());
  EXPECT_EQ(label(), GetFocusedView());

  // Obscured labels do not support text selection.
  label()->SetObscured(true);
  EXPECT_FALSE(label()->GetSelectable());
  event_generator()->PressKey(ui::VKEY_A, kControlCommandModifier);
  EXPECT_EQ(base::string16(), GetSelectedText());
}

// Verify the context menu options are enabled and disabled appropriately.
TEST_F(LabelSelectionTest, ContextMenuContents) {
  label()->SetText(ASCIIToUTF16("Label context menu"));
  label()->SizeToPreferredSize();

  // A non-selectable label should not show a context menu and both copy and
  // select-all context menu items should be disabled for it.
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kCopy));
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kSelectAll));

  // For a selectable label with no selection, only kSelectAll should be
  // enabled.
  ASSERT_TRUE(label()->SetSelectable(true));
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kCopy));
  EXPECT_TRUE(IsMenuCommandEnabled(Label::MenuCommands::kSelectAll));

  // For a selectable label with a selection, both copy and select-all should
  // be enabled.
  label()->SelectRange(gfx::Range(0, 4));
  EXPECT_TRUE(IsMenuCommandEnabled(Label::MenuCommands::kCopy));
  EXPECT_TRUE(IsMenuCommandEnabled(Label::MenuCommands::kSelectAll));
  // Ensure unsupported commands are not enabled.
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kLastCommandId + 1));

  // An obscured label would not show a context menu and both copy and
  // select-all should be disabled for it.
  label()->SetObscured(true);
  EXPECT_FALSE(label()->GetSelectable());
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kCopy));
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kSelectAll));
  label()->SetObscured(false);

  // For an empty label, both copy and select-all should be disabled.
  label()->SetText(base::string16());
  ASSERT_TRUE(label()->SetSelectable(true));
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kCopy));
  EXPECT_FALSE(IsMenuCommandEnabled(Label::MenuCommands::kSelectAll));
}

}  // namespace views
