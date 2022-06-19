// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/app_pixel_output_surface.h"

#include <utility>

#include "base/bind.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/viz/service/display/output_surface_client.h"
#include "components/viz/service/display/output_surface_frame.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "third_party/khronos/GLES2/gl2.h"
#include "ui/gfx/buffer_format_util.h"
#include "ui/gfx/presentation_feedback.h"
#include "ui/gfx/swap_result.h"
#include "ui/gfx/transform.h"

namespace cc {

AppPixelOutputSurface::AppPixelOutputSurface(
    scoped_refptr<viz::ContextProvider> context_provider,
    gfx::SurfaceOrigin origin)
    : OutputSurface(std::move(context_provider)) {
  capabilities_.output_surface_origin = origin;
  capabilities_.supports_stencil = true;
}

AppPixelOutputSurface::AppPixelOutputSurface(
    std::unique_ptr<viz::SoftwareOutputDevice> software_device)
    : OutputSurface(std::move(software_device)) {
  capabilities_.supports_stencil = true;
}

AppPixelOutputSurface::~AppPixelOutputSurface() = default;

void AppPixelOutputSurface::BindToClient(viz::OutputSurfaceClient* client) {
  client_ = client;
}

void AppPixelOutputSurface::EnsureBackbuffer() {}

void AppPixelOutputSurface::DiscardBackbuffer() {}

void AppPixelOutputSurface::BindFramebuffer() {
  context_provider()->ContextGL()->BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AppPixelOutputSurface::Reshape(const gfx::Size& size,
                                     float device_scale_factor,
                                     const gfx::ColorSpace& color_space,
                                     gfx::BufferFormat format,
                                     bool use_stencil) {
  // External stencil test cannot be tested at the same time as |use_stencil|.
  DCHECK(!use_stencil || !external_stencil_test_);
  if (context_provider()) {
    const bool has_alpha = gfx::AlphaBitsForBufferFormat(format);
    context_provider()->ContextGL()->ResizeCHROMIUM(
        size.width(), size.height(), device_scale_factor,
        color_space.AsGLColorSpace(), has_alpha);
  } else {
    software_device()->Resize(size, device_scale_factor);
  }
}

bool AppPixelOutputSurface::HasExternalStencilTest() const {
  return external_stencil_test_;
}

void AppPixelOutputSurface::ApplyExternalStencil() {}

void AppPixelOutputSurface::SwapBuffers(viz::OutputSurfaceFrame frame) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(&AppPixelOutputSurface::SwapBuffersCallback,
                                weak_ptr_factory_.GetWeakPtr()));
}

void AppPixelOutputSurface::SwapBuffersCallback() {
  base::TimeTicks now = base::TimeTicks::Now();
  gfx::SwapTimings timings = {now, now};
  client_->DidReceiveSwapBuffersAck(timings);
  client_->DidReceivePresentationFeedback(
      gfx::PresentationFeedback(base::TimeTicks::Now(), base::TimeDelta(), 0));
}

bool AppPixelOutputSurface::IsDisplayedAsOverlayPlane() const {
  return false;
}

unsigned AppPixelOutputSurface::GetOverlayTextureId() const {
  return 0;
}

uint32_t AppPixelOutputSurface::GetFramebufferCopyTextureFormat() {
  // This format will work if the |context_provider| has an RGB or RGBA
  // framebuffer. For now assume tests do not want/care about alpha in
  // the root render pass.
  return GL_RGB;
}

unsigned AppPixelOutputSurface::UpdateGpuFence() {
  return 0;
}

void AppPixelOutputSurface::SetUpdateVSyncParametersCallback(
    viz::UpdateVSyncParametersCallback callback) {}

gfx::OverlayTransform AppPixelOutputSurface::GetDisplayTransform() {
  return gfx::OVERLAY_TRANSFORM_NONE;
}

}  // namespace cc
