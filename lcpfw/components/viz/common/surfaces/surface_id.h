// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_COMMON_SURFACES_SURFACE_ID_H_
#define COMPONENTS_VIZ_COMMON_SURFACES_SURFACE_ID_H_

#include <stdint.h>

#include <iosfwd>
#include <string>

#include "base/format_macros.h"
#include "base/hash/hash.h"
#include "components/viz/common/surfaces/frame_sink_id.h"
#include "components/viz/common/surfaces/local_surface_id.h"
#include "components/viz/common/viz_common_export.h"
#include "mojo/public/cpp/bindings/struct_traits.h"

namespace viz {

namespace mojom {
class SurfaceIdDataView;
}

class VIZ_COMMON_EXPORT SurfaceId {
 public:
  constexpr SurfaceId() = default;

  constexpr SurfaceId(const SurfaceId& other) = default;

  // A SurfaceId consists of two components: FrameSinkId and LocalSurfaceId.
  // A |frame_sink_id| consists of two components; one is allocated by the
  // display compositor service and one is allocated by the client. The
  // |frame_sink_id| uniquely identifies a FrameSink (and frame source).
  // A |local_surface_id| is a sequentially allocated ID generated by the frame
  // source that uniquely identifies a sequential set of frames of the same size
  // and device scale factor.
  constexpr SurfaceId(const FrameSinkId& frame_sink_id,
                      const LocalSurfaceId& local_surface_id)
      : frame_sink_id_(frame_sink_id), local_surface_id_(local_surface_id) {}

  static constexpr SurfaceId MaxSequenceId(const FrameSinkId& frame_sink_id) {
    return SurfaceId(frame_sink_id, LocalSurfaceId::MaxSequenceId());
  }

  bool is_valid() const {
    return frame_sink_id_.is_valid() && local_surface_id_.is_valid();
  }

  size_t hash() const {
    return base::HashInts(static_cast<uint64_t>(frame_sink_id_.hash()),
                          static_cast<uint64_t>(local_surface_id_.hash()));
  }

  const FrameSinkId& frame_sink_id() const { return frame_sink_id_; }

  const LocalSurfaceId& local_surface_id() const { return local_surface_id_; }

  std::string ToString() const;

  std::string ToString(base::StringPiece frame_sink_debug_label) const;

  // Returns whether this SurfaceId was generated after |other|.
  bool IsNewerThan(const SurfaceId& other) const;

  // Returns whether this SurfaceId is the same as or was generated after
  // |other|.
  bool IsSameOrNewerThan(const SurfaceId& other) const;

  // Returns the smallest valid SurfaceId with the same FrameSinkId and embed
  // token as this SurfaceId.
  SurfaceId ToSmallestId() const;

  // Returns whether this SurfaceId has the same embed token as |other|.
  bool HasSameEmbedTokenAs(const SurfaceId& other) const;

  bool operator==(const SurfaceId& other) const {
    return frame_sink_id_ == other.frame_sink_id_ &&
           local_surface_id_ == other.local_surface_id_;
  }

  bool operator!=(const SurfaceId& other) const { return !(*this == other); }

  bool operator<(const SurfaceId& other) const {
    return std::tie(frame_sink_id_, local_surface_id_) <
           std::tie(other.frame_sink_id_, other.local_surface_id_);
  }

 private:
  friend struct mojo::StructTraits<mojom::SurfaceIdDataView, SurfaceId>;

  FrameSinkId frame_sink_id_;
  LocalSurfaceId local_surface_id_;
};

VIZ_COMMON_EXPORT std::ostream& operator<<(std::ostream& out,
                                           const SurfaceId& surface_id);

struct SurfaceIdHash {
  size_t operator()(const SurfaceId& key) const { return key.hash(); }
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_COMMON_SURFACES_SURFACE_ID_H_
