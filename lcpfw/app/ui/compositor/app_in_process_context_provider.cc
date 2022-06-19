// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/compositor/app_in_process_context_provider.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/lazy_instance.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/trace_event.h"
#include "components/viz/common/gpu/context_cache_controller.h"
#include "components/viz/app_gpu_service_holder.h"
#include "gpu/command_buffer/client/gles2_implementation.h"
#include "gpu/command_buffer/client/raster_implementation_gles.h"
#include "gpu/command_buffer/client/shared_memory_limits.h"
#include "gpu/config/skia_limits.h"
#include "gpu/ipc/gl_in_process_context.h"
#include "gpu/skia_bindings/grcontext_for_gles2_interface.h"
#include "ipc/common/surface_handle.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace ui {

    // static
    scoped_refptr<AppInProcessContextProvider> AppInProcessContextProvider::Create(
        const gpu::ContextCreationAttribs& attribs,
        gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager,
        gpu::ImageFactory* image_factory,
        gpu::SurfaceHandle window,
        const std::string& debug_name,
        bool support_locking)
    {
        return new AppInProcessContextProvider(attribs, gpu_memory_buffer_manager,
            image_factory, window, debug_name,
            support_locking);
    }

    // static
    scoped_refptr<AppInProcessContextProvider>
        AppInProcessContextProvider::CreateOffscreen(
            gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager,
            gpu::ImageFactory* image_factory,
            bool support_locking)
    {
        gpu::ContextCreationAttribs attribs;
        attribs.alpha_size = 8;
        attribs.blue_size = 8;
        attribs.green_size = 8;
        attribs.red_size = 8;
        attribs.depth_size = 0;
        attribs.stencil_size = 8;
        attribs.samples = 0;
        attribs.sample_buffers = 0;
        attribs.fail_if_major_perf_caveat = false;
        attribs.bind_generates_resource = false;
        return new AppInProcessContextProvider(attribs, gpu_memory_buffer_manager,
            image_factory, gpu::kNullSurfaceHandle,
            "Offscreen", support_locking);
    }

    AppInProcessContextProvider::AppInProcessContextProvider(
        const gpu::ContextCreationAttribs& attribs,
        gpu::GpuMemoryBufferManager* gpu_memory_buffer_manager,
        gpu::ImageFactory* image_factory,
        gpu::SurfaceHandle window,
        const std::string& debug_name,
        bool support_locking)
        : support_locking_(support_locking),
        attribs_(attribs),
        gpu_memory_buffer_manager_(gpu_memory_buffer_manager),
        image_factory_(image_factory),
        window_(window),
        debug_name_(debug_name)
    {
        DCHECK(main_thread_checker_.CalledOnValidThread());
        context_thread_checker_.DetachFromThread();
    }

    AppInProcessContextProvider::~AppInProcessContextProvider()
    {
        DCHECK(main_thread_checker_.CalledOnValidThread() ||
            context_thread_checker_.CalledOnValidThread());
    }

    void AppInProcessContextProvider::AddRef() const
    {
        base::RefCountedThreadSafe<AppInProcessContextProvider>::AddRef();
    }

    void AppInProcessContextProvider::Release() const
    {
        base::RefCountedThreadSafe<AppInProcessContextProvider>::Release();
    }

    gpu::ContextResult AppInProcessContextProvider::BindToCurrentThread()
    {
        // This is called on the thread the context will be used.
        DCHECK(context_thread_checker_.CalledOnValidThread());

        if (bind_tried_)
            return bind_result_;
        bind_tried_ = true;

        context_ = std::make_unique<gpu::GLInProcessContext>();
        bind_result_ = context_->Initialize(
            viz::AppGpuServiceHolder::GetInstance()->task_executor(),
            /*surface=*/nullptr,
            /*is_offscreen=*/window_ == gpu::kNullSurfaceHandle, window_, attribs_,
            gpu::SharedMemoryLimits(), gpu_memory_buffer_manager_, image_factory_,
            /*gpu_task_scheduler=*/nullptr,
            /*display_controller_on_gpu=*/nullptr,
            base::ThreadTaskRunnerHandle::Get());

        if (bind_result_ != gpu::ContextResult::kSuccess)
            return bind_result_;

        cache_controller_ = std::make_unique<viz::ContextCacheController>(
            context_->GetImplementation(), base::ThreadTaskRunnerHandle::Get());
        if (support_locking_)
            cache_controller_->SetLock(GetLock());

        std::string unique_context_name =
            base::StringPrintf("%s-%p", debug_name_.c_str(), context_.get());
        context_->GetImplementation()->TraceBeginCHROMIUM(
            "gpu_toplevel", unique_context_name.c_str());

        raster_context_ = std::make_unique<gpu::raster::RasterImplementationGLES>(
            context_->GetImplementation(), context_->GetImplementation());

        return bind_result_;
    }

    const gpu::Capabilities& AppInProcessContextProvider::ContextCapabilities() const
    {
        CheckValidThreadOrLockAcquired();
        return context_->GetImplementation()->capabilities();
    }

    const gpu::GpuFeatureInfo& AppInProcessContextProvider::GetGpuFeatureInfo() const
    {
        CheckValidThreadOrLockAcquired();
        return context_->GetGpuFeatureInfo();
    }

    gpu::gles2::GLES2Interface* AppInProcessContextProvider::ContextGL()
    {
        CheckValidThreadOrLockAcquired();
        return context_->GetImplementation();
    }

    gpu::raster::RasterInterface* AppInProcessContextProvider::RasterInterface()
    {
        CheckValidThreadOrLockAcquired();

        return raster_context_.get();
    }

    gpu::ContextSupport* AppInProcessContextProvider::ContextSupport()
    {
        return context_->GetImplementation();
    }

    class GrDirectContext* AppInProcessContextProvider::GrContext()
    {
        CheckValidThreadOrLockAcquired();

        if (gr_context_)
            return gr_context_->get();

        size_t max_resource_cache_bytes;
        size_t max_glyph_cache_texture_bytes;
        gpu::DefaultGrCacheLimitsForTests(&max_resource_cache_bytes,
            &max_glyph_cache_texture_bytes);
        gr_context_ = std::make_unique<skia_bindings::GrContextForGLES2Interface>(
            ContextGL(), ContextSupport(), ContextCapabilities(),
            max_resource_cache_bytes, max_glyph_cache_texture_bytes);
        cache_controller_->SetGrContext(gr_context_->get());

        return gr_context_->get();
    }

    gpu::SharedImageInterface* AppInProcessContextProvider::SharedImageInterface()
    {
        return context_->GetSharedImageInterface();
    }

    viz::ContextCacheController* AppInProcessContextProvider::CacheController()
    {
        CheckValidThreadOrLockAcquired();
        return cache_controller_.get();
    }

    base::Lock* AppInProcessContextProvider::GetLock()
    {
        if (!support_locking_)
            return nullptr;
        return &context_lock_;
    }

    void AppInProcessContextProvider::AddObserver(viz::ContextLostObserver* obs)
    {
        observers_.AddObserver(obs);
    }

    void AppInProcessContextProvider::RemoveObserver(viz::ContextLostObserver* obs)
    {
        observers_.RemoveObserver(obs);
    }

    uint32_t AppInProcessContextProvider::GetCopyTextureInternalFormat()
    {
        if (attribs_.alpha_size > 0)
            return GL_RGBA;
        DCHECK_NE(attribs_.red_size, 0);
        DCHECK_NE(attribs_.green_size, 0);
        DCHECK_NE(attribs_.blue_size, 0);
        return GL_RGB;
    }

    void AppInProcessContextProvider::SendOnContextLost()
    {
        for (auto& observer : observers_)
            observer.OnContextLost();
    }

}  // namespace ui
