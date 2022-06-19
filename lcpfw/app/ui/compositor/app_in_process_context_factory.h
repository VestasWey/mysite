// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <stdint.h>
#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "cc/app_task_graph_runner.h"
#include "components/viz/common/surfaces/frame_sink_id_allocator.h"
#include "components/viz/common/surfaces/subtree_capture_id_allocator.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "components/viz/app_gpu_memory_buffer_manager.h"
#include "components/viz/app_image_factory.h"
#include "components/viz/app_shared_bitmap_manager.h"
#include "gpu/ipc/common/surface_handle.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "services/viz/privileged/mojom/compositing/vsync_parameter_observer.mojom.h"
#include "ui/compositor/compositor.h"

namespace viz {
class HostFrameSinkManager;
}

namespace ui {
class AppInProcessContextProvider;

class AppInProcessContextFactory : public ContextFactory {
 public:
  // Both |host_frame_sink_manager| and |frame_sink_manager| must outlive the
  // ContextFactory. The constructor without |use_skia_renderer| will use
  // SkiaRenderer if the feature is enabled.
  // TODO(crbug.com/657959): |frame_sink_manager| should go away and we should
  // use the LayerTreeFrameSink from the HostFrameSinkManager.
  AppInProcessContextFactory(viz::HostFrameSinkManager* host_frame_sink_manager,
                          viz::FrameSinkManagerImpl* frame_sink_manager);
  AppInProcessContextFactory(viz::HostFrameSinkManager* host_frame_sink_manager,
                          viz::FrameSinkManagerImpl* frame_sink_manager,
                          bool use_skia_renderer);
  ~AppInProcessContextFactory() override;

  viz::FrameSinkManagerImpl* GetFrameSinkManager() {
    return frame_sink_manager_;
  }

  // If true (the default) an OutputSurface is created that does not display
  // anything. Set to false if you want to see results on the screen.
  void set_use_test_surface(bool use_test_surface) {
    use_test_surface_ = use_test_surface;
  }

  // Set refresh rate will be set to 200 to spend less time waiting for
  // BeginFrame when used for tests.
  void SetUseFastRefreshRateForTests();

  // ContextFactory implementation.
  void CreateLayerTreeFrameSink(base::WeakPtr<Compositor> compositor) override;
  scoped_refptr<viz::ContextProvider> SharedMainThreadContextProvider() override;
  scoped_refptr<viz::RasterContextProvider> SharedMainThreadRasterContextProvider() override;

  void RemoveCompositor(Compositor* compositor) override;
  gpu::GpuMemoryBufferManager* GetGpuMemoryBufferManager() override;
  cc::TaskGraphRunner* GetTaskGraphRunner() override;
  viz::FrameSinkId AllocateFrameSinkId() override;
  viz::SubtreeCaptureId AllocateSubtreeCaptureId() override;
  viz::HostFrameSinkManager* GetHostFrameSinkManager() override;

  SkMatrix44 GetOutputColorMatrix(Compositor* compositor) const;
  gfx::DisplayColorSpaces GetDisplayColorSpaces(Compositor* compositor) const;
  //float GetSDRWhiteLevel(Compositor* compositor) const;
  base::TimeTicks GetDisplayVSyncTimeBase(Compositor* compositor) const;
  base::TimeDelta GetDisplayVSyncTimeInterval(Compositor* compositor) const;
  void ResetDisplayOutputParameters(Compositor* compositor);

 private:
  class PerCompositorData;

  PerCompositorData* CreatePerCompositorData(Compositor* compositor);

  scoped_refptr<AppInProcessContextProvider> shared_main_thread_contexts_;
  scoped_refptr<AppInProcessContextProvider> shared_worker_context_provider_;
  viz::AppSharedBitmapManager shared_bitmap_manager_;
  viz::AppGpuMemoryBufferManager gpu_memory_buffer_manager_;
  viz::AppImageFactory image_factory_;
  cc::AppTaskGraphRunner task_graph_runner_;
  viz::FrameSinkIdAllocator frame_sink_id_allocator_;
  viz::SubtreeCaptureIdAllocator subtree_capture_id_allocator_;
  bool use_test_surface_;
  bool disable_vsync_ = false;
  double refresh_rate_ = 60.0;
  viz::HostFrameSinkManager* const host_frame_sink_manager_;
  viz::FrameSinkManagerImpl* const frame_sink_manager_;

  viz::RendererSettings renderer_settings_;
  viz::DebugRendererSettings debug_settings_;
  using PerCompositorDataMap =
      std::unordered_map<Compositor*, std::unique_ptr<PerCompositorData>>;
  PerCompositorDataMap per_compositor_data_;

  DISALLOW_COPY_AND_ASSIGN(AppInProcessContextFactory);
};

}  // namespace ui

