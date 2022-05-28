// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/trees/throttle_decider.h"

#include <vector>

#include "components/viz/common/quads/compositor_render_pass_draw_quad.h"
#include "components/viz/common/quads/surface_draw_quad.h"
#include "components/viz/common/surfaces/surface_range.h"

namespace cc {

ThrottleDecider::ThrottleDecider() = default;

ThrottleDecider::~ThrottleDecider() = default;

void ThrottleDecider::Prepare() {
  last_ids_.swap(ids_);
  id_to_pass_map_.clear();
  ids_.clear();
}

void ThrottleDecider::ProcessRenderPass(
    const viz::CompositorRenderPass& render_pass) {
  bool foreground_blurred =
      render_pass.filters.HasFilterOfType(FilterOperation::BLUR);
  std::vector<gfx::RectF> blur_backdrop_filter_bounds;
  for (viz::QuadList::ConstIterator it = render_pass.quad_list.begin();
       it != render_pass.quad_list.end(); ++it) {
    const viz::DrawQuad* quad = *it;
    if (quad->material == viz::DrawQuad::Material::kCompositorRenderPass) {
      // If the quad render pass has a blur backdrop filter without a mask, add
      // the filter bounds to the bounds list.
      const auto* render_pass_quad =
          viz::CompositorRenderPassDrawQuad::MaterialCast(quad);
      auto found = id_to_pass_map_.find(render_pass_quad->render_pass_id);
      if (found == id_to_pass_map_.end()) {
        // It is possible that this function is called when the render passes in
        // a frame haven't been cleaned up yet. A RPDQ can possibly refer to an
        // invalid render pass.
        continue;
      }
      const auto& child_rp = *found->second;
      if (child_rp.backdrop_filters.HasFilterOfType(FilterOperation::BLUR) &&
          render_pass_quad->resources
                  .ids[viz::RenderPassDrawQuadInternal::kMaskResourceIdIndex] ==
              viz::kInvalidResourceId) {
        gfx::RectF blur_bounds(child_rp.output_rect);
        if (child_rp.backdrop_filter_bounds)
          blur_bounds.Intersect(child_rp.backdrop_filter_bounds->rect());
        quad->shared_quad_state->quad_to_target_transform.TransformRect(
            &blur_bounds);
        if (quad->shared_quad_state->is_clipped) {
          blur_bounds.Intersect(gfx::RectF(quad->shared_quad_state->clip_rect));
        }
        blur_backdrop_filter_bounds.push_back(blur_bounds);
      }
    } else if (quad->material == viz::DrawQuad::Material::kSurfaceContent) {
      bool inside_backdrop_filter_bounds = false;
      if (!foreground_blurred && !blur_backdrop_filter_bounds.empty()) {
        gfx::RectF rect_in_target_space(quad->visible_rect);
        quad->shared_quad_state->quad_to_target_transform.TransformRect(
            &rect_in_target_space);
        if (quad->shared_quad_state->is_clipped) {
          rect_in_target_space.Intersect(
              gfx::RectF(quad->shared_quad_state->clip_rect));
        }

        for (const gfx::RectF& blur_bounds : blur_backdrop_filter_bounds) {
          if (blur_bounds.Contains(rect_in_target_space)) {
            inside_backdrop_filter_bounds = true;
            break;
          }
        }
      }
      const auto* surface_quad = viz::SurfaceDrawQuad::MaterialCast(quad);
      const viz::SurfaceRange& range = surface_quad->surface_range;
      DCHECK(range.IsValid());
      if (foreground_blurred || inside_backdrop_filter_bounds)
        ids_.insert(range.end().frame_sink_id());
    }
  }
  id_to_pass_map_.emplace(render_pass.id, &render_pass);
}

bool ThrottleDecider::HasThrottlingChanged() const {
  return ids_ != last_ids_;
}

}  // namespace cc
