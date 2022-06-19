// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "gpu/command_buffer/service/image_factory.h"

namespace viz {

class AppImageFactory : public gpu::ImageFactory {
 public:
  AppImageFactory();
  AppImageFactory(const AppImageFactory&) = delete;
  ~AppImageFactory() override;

  AppImageFactory& operator=(const AppImageFactory&) = delete;

  // Overridden from gpu::ImageFactory:
  scoped_refptr<gl::GLImage> CreateImageForGpuMemoryBuffer(
      gfx::GpuMemoryBufferHandle handle,
      const gfx::Size& size,
      gfx::BufferFormat format,
      int client_id,
      gpu::SurfaceHandle surface_handle) override;
};

}  // namespace viz
