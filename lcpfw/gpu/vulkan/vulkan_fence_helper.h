// Copyright (c) 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_VULKAN_VULKAN_FENCE_HELPER_H_
#define GPU_VULKAN_VULKAN_FENCE_HELPER_H_

#include <vulkan/vulkan.h>

#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/component_export.h"
#include "base/containers/circular_deque.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "gpu/vulkan/vma_wrapper.h"

namespace gpu {

class VulkanDeviceQueue;

class COMPONENT_EXPORT(VULKAN) VulkanFenceHelper {
 public:
  explicit VulkanFenceHelper(VulkanDeviceQueue* device_queue);
  ~VulkanFenceHelper();

  // Destroy the fence helper.
  void Destroy();

  // Class representing a fence registered with this system. Should be treated
  // as an opaque handle.
  class COMPONENT_EXPORT(VULKAN) FenceHandle {
   public:
    FenceHandle();
    FenceHandle(const FenceHandle& other);
    FenceHandle& operator=(const FenceHandle& other);

    bool is_valid() const { return fence_ != VK_NULL_HANDLE; }

   private:
    friend class VulkanFenceHelper;
    FenceHandle(VkFence fence, uint64_t generation_id);

    VkFence fence_ = VK_NULL_HANDLE;
    uint64_t generation_id_ = 0;
  };

  // General fence management functions. Should be used by any Chrome code
  // which creates / submits fences. By registering fences with this class and
  // checking them via the returned FenceHandle, we are able to leverage these
  // same fences for running cleanup tasks.
  //
  // In typical cases, callers will call GetFence to generate/reuse a fence,
  // submit this fence, then call EnqueueFence to register it with this system.
  //
  // In cases where fences are not being generated by Chrome, consumers should
  // ensure that GenerateCleanupFence is called once per frame to allow cleanup
  // tasks to be processed.
  //
  // Creates or recycles a fence.
  VkResult GetFence(VkFence* fence);
  // Enqueues a fence which must eventually signal (must have been submitted).
  // This function takes ownership of the fence. Returns a FenceHandle which
  // can be used to wait on this fence / check status.
  // Note: This should be called immediately after submitting a fence, as
  // calling this will attach cleanup tasks to the fence. If cleanup tasks
  // are able to be inserted between fence submission and this call, we can
  // end up with incorrect cleanup.
  FenceHandle EnqueueFence(VkFence fence);
  // Generates and submits a fence.
  // TODO(ericrk): We should avoid this in all cases if possible.
  FenceHandle GenerateCleanupFence();

  // Creates a callback that calls pending cleanup tasks. Used in cases where an
  // external component (Skia) is submitting / waiting on a fence and cannot
  // share that fence with this class.
  // Note: It is important that no new cleanup tasks or fences are inserted
  // between this call and the submission of the fence which will eventually
  // trigger this callback. Doing so could cause the callbacks associated
  // with this call to run out of order / incorrectly.
  base::OnceClosure CreateExternalCallback();

  // Helper functions which allow clients to wait for or check the statusof a
  // fence submitted with EnqueueFence.
  //
  // Waits for the given fence associated with the given generation id to pass.
  bool Wait(FenceHandle handle, uint64_t timeout_in_nanoseconds = UINT64_MAX);
  // Checks whether the given generation id has passed.
  bool HasPassed(FenceHandle handle);

  // Cleanup helpers. Allow callers to enqueue cleanup tasks which will be run
  // after the next fence provided by EnqueueFence or GenerateCleanupFence
  // passes. Tasks must only be enqueued if all relevant work has already been
  // submitted to the queue - it must be the case that the task can immediately
  // be run after a vkQueueWaitIdle. To ensure that cleanup tasks run, callers
  // should ensure that ProcessCleanupTasks is called once per frame.
  using CleanupTask = base::OnceCallback<void(VulkanDeviceQueue* device_queue,
                                              bool device_lost)>;
  // Submits a cleanup task for already submitted work.  ProcessCleanupTasks
  // must be called periodically to ensure these run. Cleanup tasks will be
  // executed in order they are enqueued.
  void EnqueueCleanupTaskForSubmittedWork(CleanupTask task);
  // Processes CleanupTasks for which a fence has passed.
  void ProcessCleanupTasks(uint64_t retired_generation_id = 0);
  // Helpers for common types:
  void EnqueueSemaphoreCleanupForSubmittedWork(VkSemaphore semaphore);
  void EnqueueSemaphoresCleanupForSubmittedWork(
      std::vector<VkSemaphore> semaphores);
  void EnqueueImageCleanupForSubmittedWork(VkImage image,
                                           VkDeviceMemory memory);
  void EnqueueBufferCleanupForSubmittedWork(VkBuffer buffer,
                                            VmaAllocation allocation);
  // Helpers for VulkanCommandBuffer, VulkanCommandPool, etc
  template <typename T>
  void EnqueueVulkanObjectCleanupForSubmittedWork(std::unique_ptr<T> obj);

 private:
  void PerformImmediateCleanup();

  VulkanDeviceQueue* const device_queue_;

  std::vector<CleanupTask> tasks_pending_fence_;
  uint64_t next_generation_ = 1;
  uint64_t current_generation_ = 0;

  struct TasksForFence {
    // Constructor when tasks associated with a fence.
    TasksForFence(FenceHandle handle, std::vector<CleanupTask> tasks);
    // Constructor when tasks associated with Skia callback.
    TasksForFence(uint64_t generation_id, std::vector<CleanupTask> tasks);
    ~TasksForFence();
    TasksForFence(TasksForFence&& other);
    TasksForFence& operator=(TasksForFence&& other);

    bool UsingCallback() const { return fence == VK_NULL_HANDLE; }

    const VkFence fence = VK_NULL_HANDLE;
    const uint64_t generation_id = 0;

    std::vector<CleanupTask> tasks;
  };
  base::circular_deque<TasksForFence> cleanup_tasks_;

  base::WeakPtrFactory<VulkanFenceHelper> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(VulkanFenceHelper);
};

template <typename T>
void VulkanFenceHelper::EnqueueVulkanObjectCleanupForSubmittedWork(
    std::unique_ptr<T> obj) {
  EnqueueCleanupTaskForSubmittedWork(
      base::BindOnce([](std::unique_ptr<T> obj, VulkanDeviceQueue* device_queue,
                        bool device_lost) { obj->Destroy(); },
                     std::move(obj)));
}

}  // namespace gpu

#endif  // GPU_VULKAN_VULKAN_FENCE_HELPER_H_
