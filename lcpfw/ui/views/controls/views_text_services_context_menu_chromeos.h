// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_VIEWS_TEXT_SERVICES_CONTEXT_MENU_CHROMEOS_H_
#define UI_VIEWS_CONTROLS_VIEWS_TEXT_SERVICES_CONTEXT_MENU_CHROMEOS_H_

#include <memory>

#include "ui/views/controls/views_text_services_context_menu.h"
#include "ui/views/views_export.h"

namespace views {

// This class is used to add and handle text service items in the text context
// menu under the CrOS environment.
class VIEWS_EXPORT ViewsTextServicesContextMenuChromeos
    : public ViewsTextServicesContextMenu {
 public:
  using ImplFactory = base::RepeatingCallback<std::unique_ptr<
      ViewsTextServicesContextMenu>(ui::SimpleMenuModel*, Textfield*)>;

  // Injects the method to construct `impl_`.
  static void SetImplFactory(ImplFactory factory);

  ViewsTextServicesContextMenuChromeos(ui::SimpleMenuModel* menu,
                                       Textfield* client);
  ViewsTextServicesContextMenuChromeos(
      const ViewsTextServicesContextMenuChromeos&) = delete;
  ViewsTextServicesContextMenuChromeos& operator=(
      const ViewsTextServicesContextMenuChromeos&) = delete;
  ~ViewsTextServicesContextMenuChromeos() override;

  // ViewsTextServicesContextMenu:
  bool GetAcceleratorForCommandId(int command_id,
                                  ui::Accelerator* accelerator) const override;
  bool IsCommandIdChecked(int command_id) const override;
  bool IsCommandIdEnabled(int command_id) const override;
  void ExecuteCommand(int command_id, int event_flags) override;
  bool SupportsCommand(int command_id) const override;

 private:
  // CrOS functionality must be provided by the embedder, so requests are
  // forwarded to this concrete object, whose construction can be controlled by
  // `SetImplFactory()`.
  std::unique_ptr<ViewsTextServicesContextMenu> impl_;
};

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_VIEWS_TEXT_SERVICES_CONTEXT_MENU_CHROMEOS_H_
