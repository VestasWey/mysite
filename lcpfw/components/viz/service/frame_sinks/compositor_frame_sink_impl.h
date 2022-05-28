// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_SERVICE_FRAME_SINKS_COMPOSITOR_FRAME_SINK_IMPL_H_
#define COMPONENTS_VIZ_SERVICE_FRAME_SINKS_COMPOSITOR_FRAME_SINK_IMPL_H_

#include "base/macros.h"
#include "base/memory/read_only_shared_memory_region.h"
#include "components/viz/common/surfaces/frame_sink_id.h"
#include "components/viz/common/surfaces/local_surface_id.h"
#include "components/viz/service/frame_sinks/compositor_frame_sink_support.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/viz/public/mojom/compositing/compositor_frame_sink.mojom.h"

namespace viz {

class FrameSinkManagerImpl;

// The viz portion of a non-root CompositorFrameSink. Holds the
// Binding/InterfacePtr for the mojom::CompositorFrameSink interface.
class CompositorFrameSinkImpl : public mojom::CompositorFrameSink {
 public:
  CompositorFrameSinkImpl(
      FrameSinkManagerImpl* frame_sink_manager,
      const FrameSinkId& frame_sink_id,
      mojo::PendingReceiver<mojom::CompositorFrameSink> receiver,
      mojo::PendingRemote<mojom::CompositorFrameSinkClient> client);

  ~CompositorFrameSinkImpl() override;

  // mojom::CompositorFrameSink:
  void SetNeedsBeginFrame(bool needs_begin_frame) override;
  void SetWantsAnimateOnlyBeginFrames() override;
  void SubmitCompositorFrame(
      const LocalSurfaceId& local_surface_id,
      CompositorFrame frame,
      base::Optional<HitTestRegionList> hit_test_region_list,
      uint64_t submit_time) override;
  void SubmitCompositorFrameSync(
      const LocalSurfaceId& local_surface_id,
      CompositorFrame frame,
      base::Optional<HitTestRegionList> hit_test_region_list,
      uint64_t submit_time,
      SubmitCompositorFrameSyncCallback callback) override;
  void DidNotProduceFrame(const BeginFrameAck& begin_frame_ack) override;
  void DidAllocateSharedBitmap(base::ReadOnlySharedMemoryRegion region,
                               const SharedBitmapId& id) override;
  void DidDeleteSharedBitmap(const SharedBitmapId& id) override;
  void InitializeCompositorFrameSinkType(
      mojom::CompositorFrameSinkType type) override;

 private:
  void SubmitCompositorFrameInternal(
      const LocalSurfaceId& local_surface_id,
      CompositorFrame frame,
      base::Optional<HitTestRegionList> hit_test_region_list,
      uint64_t submit_time,
      mojom::CompositorFrameSink::SubmitCompositorFrameSyncCallback);

  void OnClientConnectionLost();

  mojo::Remote<mojom::CompositorFrameSinkClient> compositor_frame_sink_client_;
  mojo::Receiver<mojom::CompositorFrameSink> compositor_frame_sink_receiver_;

  // Must be destroyed before |compositor_frame_sink_client_|. This must never
  // change for the lifetime of CompositorFrameSinkImpl.
  const std::unique_ptr<CompositorFrameSinkSupport> support_;

  DISALLOW_COPY_AND_ASSIGN(CompositorFrameSinkImpl);
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_SERVICE_FRAME_SINKS_COMPOSITOR_FRAME_SINK_IMPL_H_
