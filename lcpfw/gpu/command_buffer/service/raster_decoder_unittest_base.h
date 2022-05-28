// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_RASTER_DECODER_UNITTEST_BASE_H_
#define GPU_COMMAND_BUFFER_SERVICE_RASTER_DECODER_UNITTEST_BASE_H_

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "base/test/task_environment.h"
#include "components/viz/common/resources/resource_format.h"
#include "gpu/command_buffer/client/client_test_helper.h"
#include "gpu/command_buffer/common/raster_cmd_format.h"
#include "gpu/command_buffer/service/decoder_client.h"
#include "gpu/command_buffer/service/gl_context_mock.h"
#include "gpu/command_buffer/service/gpu_tracer.h"
#include "gpu/command_buffer/service/memory_tracking.h"
#include "gpu/command_buffer/service/raster_decoder.h"
#include "gpu/command_buffer/service/shared_image_manager.h"
#include "gpu/command_buffer/service/shared_image_representation.h"
#include "gpu/command_buffer/service/test_helper.h"
#include "gpu/config/gpu_driver_bug_workarounds.h"
#include "gpu/config/gpu_preferences.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gl/gl_mock.h"
#include "ui/gl/gl_surface_stub.h"
#include "ui/gl/gl_version_info.h"

namespace gpu {

namespace gles2 {
class MockCopyTextureResourceManager;
}  // namespace gles2

namespace raster {

class RasterDecoderTestBase : public ::testing::TestWithParam<bool>,
                              public DecoderClient {
 public:
  RasterDecoderTestBase();
  ~RasterDecoderTestBase() override;

  void OnConsoleMessage(int32_t id, const std::string& message) override;
  void CacheShader(const std::string& key, const std::string& shader) override;
  void OnFenceSyncRelease(uint64_t release) override;
  void OnDescheduleUntilFinished() override;
  void OnRescheduleAfterFinished() override;
  void OnSwapBuffers(uint64_t swap_id, uint32_t flags) override;
  void ScheduleGrContextCleanup() override {}
  void HandleReturnData(base::span<const uint8_t> data) override {}

  // Template to call glGenXXX functions.
  template <typename T>
  void GenHelper(GLuint client_id) {
    int8_t buffer[sizeof(T) + sizeof(client_id)];
    T& cmd = *reinterpret_cast<T*>(&buffer);
    cmd.Init(1, &client_id);
    EXPECT_EQ(error::kNoError, ExecuteImmediateCmd(cmd, sizeof(client_id)));
  }

  // This template exists solely so we can specialize it for
  // certain commands.
  template <typename T, int id>
  void SpecializedSetup(bool valid) {}

  template <typename T>
  T* GetImmediateAs() {
    return reinterpret_cast<T*>(immediate_buffer_);
  }

  void ClearSharedMemory() {
    memset(shared_memory_base_, kInitialMemoryValue, kSharedBufferSize);
  }

  void SetUp() override;
  void TearDown() override;

  template <typename T>
  error::Error ExecuteCmd(const T& cmd) {
    static_assert(T::kArgFlags == cmd::kFixed,
                  "T::kArgFlags should equal cmd::kFixed");
    int entries_processed = 0;
    return decoder_->DoCommands(1, (const void*)&cmd,
                                ComputeNumEntries(sizeof(cmd)),
                                &entries_processed);
  }

  template <typename T>
  error::Error ExecuteImmediateCmd(const T& cmd, size_t data_size) {
    static_assert(T::kArgFlags == cmd::kAtLeastN,
                  "T::kArgFlags should equal cmd::kAtLeastN");
    int entries_processed = 0;
    return decoder_->DoCommands(1, (const void*)&cmd,
                                ComputeNumEntries(sizeof(cmd) + data_size),
                                &entries_processed);
  }

  template <typename T>
  T GetSharedMemoryAs() {
    return reinterpret_cast<T>(shared_memory_address_);
  }

  template <typename T>
  T GetSharedMemoryAsWithOffset(uint32_t offset) {
    void* ptr = reinterpret_cast<int8_t*>(shared_memory_address_) + offset;
    return reinterpret_cast<T>(ptr);
  }

  void SetBucketData(uint32_t bucket_id, const void* data, uint32_t data_size);
  void SetBucketAsCString(uint32_t bucket_id, const char* str);
  // If we want a valid bucket, just set |count_in_header| as |count|,
  // and set |str_end| as 0.
  void SetBucketAsCStrings(uint32_t bucket_id,
                           GLsizei count,
                           const char** str,
                           GLsizei count_in_header,
                           char str_end);

  void AddExpectationsForGetCapabilities();
  void AddExpectationsForRestoreAttribState(GLuint attrib);

  struct InitState {
    InitState();
    ~InitState();

    std::vector<std::string> extensions = {"GL_ARB_sync"};
    bool lose_context_when_out_of_memory = false;
    gpu::GpuDriverBugWorkarounds workarounds;
    std::string gl_version = "2.1";
    ContextType context_type = CONTEXT_TYPE_OPENGLES2;
  };

  void InitDecoder(const InitState& init);
  void ResetDecoder();

  error::ContextLostReason GetContextLostReason() const {
    return command_buffer_service_->GetState().context_lost_reason;
  }

  ::testing::StrictMock<::gl::MockGLInterface>* GetGLMock() const {
    return gl_.get();
  }

  RasterDecoder* GetDecoder() const { return decoder_.get(); }

  typedef gles2::TestHelper::AttribInfo AttribInfo;
  typedef gles2::TestHelper::UniformInfo UniformInfo;

  gpu::Mailbox CreateFakeTexture(GLuint service_id,
                                 viz::ResourceFormat resource_format,
                                 GLsizei width,
                                 GLsizei height,
                                 bool cleared);

  // Note that the error is returned as GLint instead of GLenum.
  // This is because there is a mismatch in the types of GLenum and
  // the error values GL_NO_ERROR, GL_INVALID_ENUM, etc. GLenum is
  // typedef'd as unsigned int while the error values are defined as
  // integers. This is problematic for template functions such as
  // EXPECT_EQ that expect both types to be the same.
  GLint GetGLError();

  void SetScopedTextureBinderExpectations(GLenum target);

  void SetupClearTextureExpectations(GLuint service_id,
                                     GLuint old_service_id,
                                     GLenum bind_target,
                                     GLenum target,
                                     GLint level,
                                     GLenum format,
                                     GLenum type,
                                     GLint xoffset,
                                     GLint yoffset,
                                     GLsizei width,
                                     GLsizei height,
                                     GLuint bound_pixel_unpack_buffer);

  GLvoid* BufferOffset(unsigned i) { return reinterpret_cast<GLvoid*>(i); }

  SharedImageManager* shared_image_manager() { return &shared_image_manager_; }
  gles2::FeatureInfo* feature_info() { return feature_info_.get(); }

 protected:
  static const GLint kMaxTextureSize = 2048;
  static const GLint kNumTextureUnits = 8;
  static const GLint kNumVertexAttribs = 16;

  static const GLuint kServiceBufferId = 301;
  static const GLuint kServiceTextureId = 304;
  static const GLuint kServiceVertexArrayId = 310;

  static const size_t kSharedBufferSize = 2048;
  static const uint32_t kSharedMemoryOffset = 132;
  static const int32_t kInvalidSharedMemoryId =
      FakeCommandBufferServiceBase::kTransferBufferBaseId - 1;
  static const uint32_t kInvalidSharedMemoryOffset = kSharedBufferSize + 1;
  static const uint32_t kInitialResult = 0xBDBDBDBDu;
  static const uint8_t kInitialMemoryValue = 0xBDu;

  static const uint32_t kNewClientId = 501;
  static const uint32_t kNewServiceId = 502;
  static const uint32_t kInvalidClientId = 601;

  // Use StrictMock to make 100% sure we know how GL will be called.
  std::unique_ptr<::testing::StrictMock<::gl::MockGLInterface>> gl_;
  scoped_refptr<gles2::FeatureInfo> feature_info_;
  scoped_refptr<gl::GLSurfaceStub> surface_;
  scoped_refptr<GLContextMock> context_;
  std::unique_ptr<FakeCommandBufferServiceBase> command_buffer_service_;
  gles2::TraceOutputter outputter_;
  std::unique_ptr<RasterDecoder> decoder_;

  gpu::Mailbox client_texture_mailbox_;

  int32_t shared_memory_id_;
  uint32_t shared_memory_offset_;
  void* shared_memory_address_;
  void* shared_memory_base_;

  uint32_t immediate_buffer_[64];

  const bool ignore_cached_state_for_test_;
  scoped_refptr<SharedContextState> shared_context_state_;

 private:
  GpuPreferences gpu_preferences_;
  SharedImageManager shared_image_manager_;
  MemoryTypeTracker memory_tracker_;
  std::vector<std::unique_ptr<SharedImageRepresentationFactoryRef>>
      shared_images_;
  base::test::SingleThreadTaskEnvironment task_environment_;
  gles2::MockCopyTextureResourceManager* copy_texture_manager_;  // not owned
};

class RasterDecoderManualInitTest : public RasterDecoderTestBase {
 public:
  RasterDecoderManualInitTest() = default;

  // Override default setup so nothing gets setup.
  void SetUp() override {}
};

}  // namespace raster
}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_RASTER_DECODER_UNITTEST_BASE_H_
