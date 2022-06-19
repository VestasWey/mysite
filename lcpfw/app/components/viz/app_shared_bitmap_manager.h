// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <map>
#include <set>

#include "base/memory/shared_memory_mapping.h"
#include "base/sequence_checker.h"
#include "components/viz/service/display/shared_bitmap_manager.h"

namespace viz {

class AppSharedBitmapManager : public SharedBitmapManager {
 public:
  AppSharedBitmapManager();
  ~AppSharedBitmapManager() override;

  // SharedBitmapManager implementation.
  std::unique_ptr<SharedBitmap> GetSharedBitmapFromId(
      const gfx::Size& size,
      ResourceFormat format,
      const SharedBitmapId& id) override;
  base::UnguessableToken GetSharedBitmapTracingGUIDFromId(
      const SharedBitmapId& id) override;
  bool ChildAllocatedSharedBitmap(base::ReadOnlySharedMemoryMapping mapping,
                                  const SharedBitmapId& id) override;
  void ChildDeletedSharedBitmap(const SharedBitmapId& id) override;

 private:
  SEQUENCE_CHECKER(sequence_checker_);

  std::map<SharedBitmapId, base::ReadOnlySharedMemoryMapping> mapping_map_;
  std::set<SharedBitmapId> notified_set_;
};

}  // namespace viz
