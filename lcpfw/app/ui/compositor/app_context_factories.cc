// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/compositor/app_context_factories.h"

#include "base/command_line.h"
#include "components/viz/common/features.h"
#include "components/viz/host/host_frame_sink_manager.h"
#include "components/viz/service/display_embedder/server_shared_bitmap_manager.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "ui/compositor/compositor_switches.h"
#include "ui/compositor/app_in_process_context_factory.h"
#include "ui/gl/gl_implementation.h"

namespace ui {

    AppContextFactories::AppContextFactories(bool enable_pixel_output)
        : AppContextFactories(enable_pixel_output, features::IsUsingSkiaRenderer())
    {
    }

    AppContextFactories::AppContextFactories(bool enable_pixel_output,
        bool use_skia_renderer)
    {
        auto* command_line = base::CommandLine::ForCurrentProcess();
        if (command_line->HasSwitch(switches::kEnablePixelOutputInTests))
            enable_pixel_output = true;
        if (enable_pixel_output)
            disable_null_draw_ = std::make_unique<gl::DisableNullDrawGLBindings>();
        shared_bitmap_manager_ = std::make_unique<viz::ServerSharedBitmapManager>();
        frame_sink_manager_ = std::make_unique<viz::FrameSinkManagerImpl>(shared_bitmap_manager_.get());
        host_frame_sink_manager_ = std::make_unique<viz::HostFrameSinkManager>();
        implicit_factory_ = std::make_unique<AppInProcessContextFactory>(
            host_frame_sink_manager_.get(), 
            frame_sink_manager_.get(),
            use_skia_renderer);
        implicit_factory_->SetUseFastRefreshRateForTests();

        // Directly connect without using Mojo.
        frame_sink_manager_->SetLocalClient(host_frame_sink_manager_.get());
        host_frame_sink_manager_->SetLocalManager(frame_sink_manager_.get());
    }

    AppContextFactories::~AppContextFactories() = default;

    ContextFactory* AppContextFactories::GetContextFactory() const
    {
        return implicit_factory_.get();
    }

    void AppContextFactories::SetUseTestSurface(bool use_test_surface)
    {
        implicit_factory_->set_use_test_surface(use_test_surface);
    }

}  // namespace ui
