// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_SERVICE_DISPLAY_DISPLAY_RESOURCE_PROVIDER_SKIA_H_
#define COMPONENTS_VIZ_SERVICE_DISPLAY_DISPLAY_RESOURCE_PROVIDER_SKIA_H_

#include <utility>
#include <vector>

#include "components/viz/service/display/display_resource_provider.h"
#include "components/viz/service/display/external_use_client.h"
#include "components/viz/service/viz_service_export.h"

namespace viz {

// DisplayResourceProvider implementation used with SkiaRenderer.
class VIZ_SERVICE_EXPORT DisplayResourceProviderSkia
    : public DisplayResourceProvider {
 public:
  DisplayResourceProviderSkia();
  ~DisplayResourceProviderSkia() override;

  // Maintains set of resources locked for external use by SkiaRenderer.
  class VIZ_SERVICE_EXPORT LockSetForExternalUse {
   public:
    // There should be at most one instance of this class per
    // |resource_provider|. Both |resource_provider| and |client| outlive this
    // class.
    LockSetForExternalUse(DisplayResourceProviderSkia* resource_provider,
                          ExternalUseClient* client);
    ~LockSetForExternalUse();

    LockSetForExternalUse(const LockSetForExternalUse&) = delete;
    LockSetForExternalUse& operator=(const LockSetForExternalUse& other) =
        delete;

    // Lock a resource for external use. The return value was created by
    // |client| at some point in the past. The SkImage color space will be set
    // to |color_space| if valid, otherwise it will be set to the resource's
    // color space. If |is_video_plane| is true, the image color space will be
    // set to nullptr (to avoid LOG spam).
    ExternalUseClient::ImageContext* LockResource(
        ResourceId resource_id,
        bool maybe_concurrent_reads,
        bool is_video_plane,
        const gfx::ColorSpace& color_space = gfx::ColorSpace());

    // Unlock all locked resources with a |sync_token|.  The |sync_token| should
    // be waited on before reusing the resource's backing to ensure that any
    // external use of it is completed. This |sync_token| should have been
    // verified.  All resources must be unlocked before destroying this class.
    void UnlockResources(const gpu::SyncToken& sync_token);

   private:
    DisplayResourceProviderSkia* const resource_provider_;
    std::vector<std::pair<ResourceId, ChildResource*>> resources_;
  };

 private:
  // DisplayResourceProvider overrides:
  std::vector<ReturnedResource> DeleteAndReturnUnusedResourcesToChildImpl(
      Child& child_info,
      DeleteStyle style,
      const std::vector<ResourceId>& unused) override;

  // Used to release resources held by an external consumer.
  ExternalUseClient* external_use_client_ = nullptr;
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_SERVICE_DISPLAY_DISPLAY_RESOURCE_PROVIDER_SKIA_H_
