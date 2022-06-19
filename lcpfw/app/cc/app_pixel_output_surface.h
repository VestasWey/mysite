// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "base/memory/weak_ptr.h"
#include "components/viz/service/display/output_surface.h"

namespace cc {

class AppPixelOutputSurface : public viz::OutputSurface {
 public:
  explicit AppPixelOutputSurface(
      scoped_refptr<viz::ContextProvider> context_provider,
      gfx::SurfaceOrigin origin);
  explicit AppPixelOutputSurface(
      std::unique_ptr<viz::SoftwareOutputDevice> software_device);
  ~AppPixelOutputSurface() override;

  // OutputSurface implementation.
  void BindToClient(viz::OutputSurfaceClient* client) override;
  void EnsureBackbuffer() override;
  void DiscardBackbuffer() override;
  void BindFramebuffer() override;
  void Reshape(const gfx::Size& size,
               float device_scale_factor,
               const gfx::ColorSpace& color_space,
               gfx::BufferFormat format,
               bool use_stencil) override;
  bool HasExternalStencilTest() const override;
  void ApplyExternalStencil() override;
  void SwapBuffers(viz::OutputSurfaceFrame frame) override;
  bool IsDisplayedAsOverlayPlane() const override;
  unsigned GetOverlayTextureId() const override;
  uint32_t GetFramebufferCopyTextureFormat() override;
  unsigned UpdateGpuFence() override;
  void SetUpdateVSyncParametersCallback(
      viz::UpdateVSyncParametersCallback callback) override;
  void SetDisplayTransformHint(gfx::OverlayTransform transform) override {}
  gfx::OverlayTransform GetDisplayTransform() override;

  void set_has_external_stencil_test(bool has_test) {
    external_stencil_test_ = has_test;
  }

 private:
  void SwapBuffersCallback();

  bool external_stencil_test_ = false;
  viz::OutputSurfaceClient* client_ = nullptr;
  base::WeakPtrFactory<AppPixelOutputSurface> weak_ptr_factory_{this};
};

}  // namespace cc
