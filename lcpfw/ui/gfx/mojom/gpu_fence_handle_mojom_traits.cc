// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/mojom/gpu_fence_handle_mojom_traits.h"

#include "build/build_config.h"
#include "mojo/public/cpp/system/platform_handle.h"

namespace mojo {

#if defined(OS_POSIX)
mojo::PlatformHandle
StructTraits<gfx::mojom::GpuFenceHandleDataView,
             gfx::GpuFenceHandle>::native_fd(gfx::GpuFenceHandle& handle) {
  return mojo::PlatformHandle(std::move(handle.owned_fd));
}
#elif defined(OS_WIN)
mojo::PlatformHandle
StructTraits<gfx::mojom::GpuFenceHandleDataView,
             gfx::GpuFenceHandle>::native_handle(gfx::GpuFenceHandle& handle) {
  return mojo::PlatformHandle(std::move(handle.owned_handle));
}
#endif

bool StructTraits<gfx::mojom::GpuFenceHandleDataView, gfx::GpuFenceHandle>::
    Read(gfx::mojom::GpuFenceHandleDataView data, gfx::GpuFenceHandle* out) {
#if defined(OS_POSIX)
  out->owned_fd = data.TakeNativeFd().TakeFD();
  return true;
#elif defined(OS_WIN)
  out->owned_handle = data.TakeNativeHandle().TakeHandle();
  return true;
#else
  return false;
#endif
}

void StructTraits<gfx::mojom::GpuFenceHandleDataView,
                  gfx::GpuFenceHandle>::SetToNull(gfx::GpuFenceHandle* handle) {
#if defined(OS_POSIX)
  handle->owned_fd.reset();
#elif defined(OS_WIN)
  handle->owned_handle.Close();
#endif
}

bool StructTraits<gfx::mojom::GpuFenceHandleDataView,
                  gfx::GpuFenceHandle>::IsNull(const gfx::GpuFenceHandle&
                                                   handle) {
  return handle.is_null();
}

}  // namespace mojo
