// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

namespace gl {
    class DisableNullDrawGLBindings;
}

namespace viz {
    class HostFrameSinkManager;
    class ServerSharedBitmapManager;
    class FrameSinkManagerImpl;
}  // namespace viz

namespace ui {

    class AppInProcessContextFactory;
    class ContextFactory;

    // Set up the compositor ContextFactory for a test environment. Unit tests that
    // do not have a full content environment need to call this before initializing
    // the Compositor. Some tests expect pixel output, and they should pass true for
    // |enable_pixel_output|. Most unit tests should pass false.
    class AppContextFactories {
    public:
        explicit AppContextFactories(bool enable_pixel_output);
        AppContextFactories(bool enable_pixel_output, bool use_skia_renderer);
        ~AppContextFactories();

        AppContextFactories(const AppContextFactories&) = delete;
        AppContextFactories& operator=(const AppContextFactories&) = delete;

        ContextFactory* GetContextFactory() const;

        // See AppInProcessContextFactory::set_use_test_surface().
        // If true (the default) an OutputSurface is created that does not display
        // anything. Set to false if you want to see results on the screen.
        void SetUseTestSurface(bool use_test_surface);

    private:
        std::unique_ptr<gl::DisableNullDrawGLBindings> disable_null_draw_;
        std::unique_ptr<viz::ServerSharedBitmapManager> shared_bitmap_manager_;
        std::unique_ptr<viz::FrameSinkManagerImpl> frame_sink_manager_;
        std::unique_ptr<viz::HostFrameSinkManager> host_frame_sink_manager_;
        std::unique_ptr<ui::AppInProcessContextFactory> implicit_factory_;
    };

}  // namespace ui
