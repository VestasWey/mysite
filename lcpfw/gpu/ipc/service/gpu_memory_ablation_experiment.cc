// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/service/gpu_memory_ablation_experiment.h"

#include <algorithm>

#include "base/bind.h"
#include "base/metrics/field_trial_params.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"
#include "base/trace_event/common/trace_event_common.h"
#include "components/viz/common/features.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/mailbox_manager_impl.h"
#include "gpu/command_buffer/service/shared_image_factory.h"
#include "gpu/command_buffer/service/shared_image_representation.h"
#include "gpu/ipc/common/surface_handle.h"
#include "gpu/ipc/service/gpu_channel_manager.h"
#include "gpu/ipc/service/gpu_memory_buffer_factory.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_surface.h"

namespace gpu {

// Main feature flag to control the entire experiment, encompassing bot CPU and
// GPU ablations.
const base::Feature kGPUMemoryAblationFeature{
    "GPUMemoryAblation", base::FEATURE_DISABLED_BY_DEFAULT};

// Field Trial Parameter that defines the size of memory allocations.
const char kGPUMemoryAblationFeatureSizeParam[] = "Size";

// Image allocation parameters.
constexpr viz::ResourceFormat kFormat = viz::ResourceFormat::RGBA_8888;
constexpr uint32_t kUsage = SHARED_IMAGE_USAGE_DISPLAY;

bool GpuMemoryAblationExperiment::ExperimentSupported() {
  if (!base::FeatureList::IsEnabled(kGPUMemoryAblationFeature))
    return false;
  gl::GLImplementation gl_impl = gl::GetGLImplementation();
  // Mock and Stub implementations are used in tests. It is not possible to
  // create a valid GrContext with these. A disabled implementation is used by
  // the GPU Info Collection Process. This also has no graphical output
  // possible.
  if (gl_impl == gl::kGLImplementationNone ||
      gl_impl == gl::kGLImplementationMockGL ||
      gl_impl == gl::kGLImplementationStubGL ||
      gl_impl == gl::kGLImplementationDisabled) {
    return false;
  }
  return true;
}

GpuMemoryAblationExperiment::GpuMemoryAblationExperiment(
    GpuChannelManager* channel_manager,
    scoped_refptr<base::SequencedTaskRunner> task_runner)
    : channel_manager_(channel_manager), task_runner_(task_runner) {
  if (!GpuMemoryAblationExperiment::ExperimentSupported())
    init_status_ = Status::DISABLED;
}

GpuMemoryAblationExperiment::~GpuMemoryAblationExperiment() {
  // Some unittests don't properly clean up. Clean up our allocations if
  // necessary.
  if (mailboxes_.empty())
    return;
  bool have_context = context_state_->MakeCurrent(context_state_->surface());
  factory_->DestroyAllSharedImages(have_context);
}

void GpuMemoryAblationExperiment::OnMemoryAllocated(uint64_t old_size,
                                                    uint64_t new_size) {
  if (init_status_ == Status::DISABLED) {
    return;
  } else if (init_status_ == Status::UNINITIALIZED) {
    if (InitGpu(channel_manager_)) {
      init_status_ = Status::ENABLED;
    } else {
      init_status_ = Status::DISABLED;
      context_state_ = nullptr;
      return;
    }
  }
  // TODO(jonross): Investigate why there are 0 size allocations.
  if (new_size > old_size) {
    // TODO(jonross): Impl CPU ablation
    AllocateGpuMemory();
  } else if (old_size > new_size) {
    // TODO(jonross): Impl CPU ablation
    if (!mailboxes_.empty()) {
      // We need to perform this as a PostTask. Though the delete calls are
      // similarly nested to the allocations, attempting to restore the
      // previous context will attempt to restore bindings or surfaces which
      // were in the process of being deleted. (https://crbug.com/1106926)
      task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(&GpuMemoryAblationExperiment::DeleteGpuMemory,
                         weak_factory_.GetWeakPtr()));
    }
  }
}

uint64_t GpuMemoryAblationExperiment::GetPeakMemory(
    uint32_t sequence_num) const {
  auto it = sequences_.find(sequence_num);
  if (it == sequences_.end())
    return 0u;

  return it->second.peak_memory_;
}

void GpuMemoryAblationExperiment::StartSequence(uint32_t sequence_num) {
  sequences_.emplace(sequence_num, SequenceTracker());
}

void GpuMemoryAblationExperiment::StopSequence(uint32_t sequence_num) {
  auto it = sequences_.find(sequence_num);
  if (it == sequences_.end())
    return;

  TRACE_EVENT_INSTANT2("gpu.memory", "Memory.GPU.PeakMemoryUsage.AblationTimes",
                       TRACE_EVENT_SCOPE_THREAD, "alloc",
                       it->second.allocs_.InMilliseconds(), "dealloc",
                       it->second.deallocs_.InMilliseconds());

  sequences_.erase(it);
}

void GpuMemoryAblationExperiment::AllocateGpuMemory() {
  // We can't successfully create an image without a context, so do not even
  // perform the initial allocations.
  std::unique_ptr<ui::ScopedMakeCurrent> scoped_current =
      ScopedMakeContextCurrent();
  if (!scoped_current)
    return;
  base::Time start = base::Time::Now();

  auto mailbox = Mailbox::GenerateForSharedImage();
  auto color_space = gfx::ColorSpace::CreateSRGB();

  if (!factory_->CreateSharedImage(
          mailbox, kFormat, size_, color_space, kTopLeft_GrSurfaceOrigin,
          kPremul_SkAlphaType, gpu::kNullSurfaceHandle, kUsage)) {
    return;
  }

  auto skia_rep = rep_factory_->ProduceSkia(mailbox, context_state_);
  if (!skia_rep)
    return;

  auto write_access = skia_rep->BeginScopedWriteAccess(
      /*begin_semaphores=*/{}, /*end_semaphores=*/{},
      SharedImageRepresentation::AllowUnclearedAccess::kYes);
  if (!write_access)
    return;

  auto* canvas = write_access->surface()->getCanvas();
  canvas->clear(SK_ColorWHITE);

  mailboxes_.push_back(mailbox);

  base::TimeDelta delta = base::Time::Now() - start;
  for (auto& it : sequences_)
    it.second.allocs_ += delta;
}

void GpuMemoryAblationExperiment::DeleteGpuMemory() {
  if (mailboxes_.empty())
    return;
  base::Time start = base::Time::Now();

  auto mailbox = mailboxes_.front();
  // We can't successfully destroy the image if we cannot get the context,
  // however we still need to cleanup our internal state.
  //
  // Unlike in initialization and allocating memory, we cannot use an
  // ui::ScopedMakeCurrent here. Though we use a PostTask to separate the
  // delete calls from the nesting, attempting to restore the previous context
  // will attempt to restore bindings or surfaces which were in the process of
  // being deleted. (https://crbug.com/1106926)
  if (context_state_->MakeCurrent(nullptr))
    factory_->DestroySharedImage(mailbox);

  mailboxes_.erase(mailboxes_.begin());

  base::TimeDelta delta = base::Time::Now() - start;
  for (auto& it : sequences_)
    it.second.deallocs_ += delta;
}

bool GpuMemoryAblationExperiment::InitGpu(GpuChannelManager* channel_manager) {
  ContextResult result;
  context_state_ = channel_manager->GetSharedContextState(&result);
  if (result != ContextResult::kSuccess)
    return false;

  const int default_value = 0;
  int arg_value = base::GetFieldTrialParamByFeatureAsInt(
      kGPUMemoryAblationFeature, kGPUMemoryAblationFeatureSizeParam,
      default_value);
  if (arg_value == default_value)
    return false;
  int texture_size =
      std::min(256 * arg_value, context_state_->gr_context()->maxTextureSize());
  size_ = gfx::Size(texture_size, texture_size);

  std::unique_ptr<ui::ScopedMakeCurrent> scoped_current =
      ScopedMakeContextCurrent();
  if (!scoped_current)
    return false;

  gpu::GpuMemoryBufferFactory* gmb_factory =
      channel_manager->gpu_memory_buffer_factory();
  factory_ = std::make_unique<SharedImageFactory>(
      channel_manager->gpu_preferences(),
      channel_manager->gpu_driver_bug_workarounds(),
      channel_manager->gpu_feature_info(), context_state_.get(),
      channel_manager->mailbox_manager(),
      channel_manager->shared_image_manager(),
      gmb_factory ? gmb_factory->AsImageFactory() : nullptr, this,
      features::IsUsingSkiaRenderer());

  rep_factory_ = std::make_unique<SharedImageRepresentationFactory>(
      channel_manager->shared_image_manager(), this);
  return true;
}

std::unique_ptr<ui::ScopedMakeCurrent>
GpuMemoryAblationExperiment::ScopedMakeContextCurrent() {
  std::unique_ptr<ui::ScopedMakeCurrent> scoped_current =
      std::make_unique<ui::ScopedMakeCurrent>(context_state_->context(),
                                              context_state_->surface());
  if (!context_state_->IsCurrent(context_state_->surface()))
    return nullptr;
  return scoped_current;
}

// MemoryTracker:
void GpuMemoryAblationExperiment::TrackMemoryAllocatedChange(int64_t delta) {
  DCHECK(delta >= 0 || gpu_allocation_size_ >= static_cast<uint64_t>(-delta));
  gpu_allocation_size_ += delta;
  for (auto& it : sequences_) {
    if (gpu_allocation_size_ > it.second.peak_memory_)
      it.second.peak_memory_ = gpu_allocation_size_;
  }
}

// Unused methods that form the basis of memory dumps
uint64_t GpuMemoryAblationExperiment::GetSize() const {
  return 0u;
}

uint64_t GpuMemoryAblationExperiment::ClientTracingId() const {
  return 0u;
}

int GpuMemoryAblationExperiment::ClientId() const {
  return 0;
}

uint64_t GpuMemoryAblationExperiment::ContextGroupTracingId() const {
  return 0u;
}

}  // namespace gpu