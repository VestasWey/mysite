// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_GTK_PRIMARY_SELECTION_DEVICE_MANAGER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_GTK_PRIMARY_SELECTION_DEVICE_MANAGER_H_

#include <memory>

#include "ui/ozone/platform/wayland/common/wayland_object.h"
#include "ui/ozone/platform/wayland/host/wayland_data_source.h"

namespace ui {

class GtkPrimarySelectionDevice;
class WaylandConnection;

class GtkPrimarySelectionDeviceManager {
 public:
  using DataSource = GtkPrimarySelectionSource;
  using DataDevice = GtkPrimarySelectionDevice;

  GtkPrimarySelectionDeviceManager(
      gtk_primary_selection_device_manager* manager,
      WaylandConnection* connection);
  GtkPrimarySelectionDeviceManager(const GtkPrimarySelectionDeviceManager&) =
      delete;
  GtkPrimarySelectionDeviceManager& operator=(
      const GtkPrimarySelectionDeviceManager&) = delete;
  ~GtkPrimarySelectionDeviceManager();

  GtkPrimarySelectionDevice* GetDevice();
  std::unique_ptr<GtkPrimarySelectionSource> CreateSource(
      GtkPrimarySelectionSource::Delegate* delegate);

 private:
  wl::Object<gtk_primary_selection_device_manager> device_manager_;

  WaylandConnection* const connection_;

  std::unique_ptr<GtkPrimarySelectionDevice> device_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_GTK_PRIMARY_SELECTION_DEVICE_MANAGER_H_
