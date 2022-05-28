// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/vulkan/vulkan_util.h"

#include "base/callback_helpers.h"
#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/pattern.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "build/build_config.h"
#include "gpu/config/gpu_info.h"  // nogncheck
#include "gpu/config/vulkan_info.h"
#include "gpu/vulkan/vulkan_function_pointers.h"

namespace gpu {
namespace {
uint64_t g_submit_count = 0u;
uint64_t g_import_semaphore_into_gl_count = 0u;
}

bool SubmitSignalVkSemaphores(VkQueue vk_queue,
                              const base::span<VkSemaphore>& vk_semaphores,
                              VkFence vk_fence) {
  // Structure specifying a queue submit operation.
  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.signalSemaphoreCount = vk_semaphores.size();
  submit_info.pSignalSemaphores = vk_semaphores.data();
  const unsigned int submit_count = 1;
  return vkQueueSubmit(vk_queue, submit_count, &submit_info, vk_fence) ==
         VK_SUCCESS;
}

bool SubmitSignalVkSemaphore(VkQueue vk_queue,
                             VkSemaphore vk_semaphore,
                             VkFence vk_fence) {
  return SubmitSignalVkSemaphores(
      vk_queue, base::span<VkSemaphore>(&vk_semaphore, 1), vk_fence);
}

bool SubmitWaitVkSemaphores(VkQueue vk_queue,
                            const base::span<VkSemaphore>& vk_semaphores,
                            VkFence vk_fence) {
  DCHECK(!vk_semaphores.empty());
  std::vector<VkPipelineStageFlags> semaphore_stages(vk_semaphores.size());
  std::fill(semaphore_stages.begin(), semaphore_stages.end(),
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  // Structure specifying a queue submit operation.
  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.waitSemaphoreCount = vk_semaphores.size();
  submit_info.pWaitSemaphores = vk_semaphores.data();
  submit_info.pWaitDstStageMask = semaphore_stages.data();
  const unsigned int submit_count = 1;
  return vkQueueSubmit(vk_queue, submit_count, &submit_info, vk_fence) ==
         VK_SUCCESS;
}

bool SubmitWaitVkSemaphore(VkQueue vk_queue,
                           VkSemaphore vk_semaphore,
                           VkFence vk_fence) {
  return SubmitWaitVkSemaphores(
      vk_queue, base::span<VkSemaphore>(&vk_semaphore, 1), vk_fence);
}

VkSemaphore CreateExternalVkSemaphore(
    VkDevice vk_device,
    VkExternalSemaphoreHandleTypeFlags handle_types) {
  base::ScopedClosureRunner uma_runner(base::BindOnce(
      [](base::Time time) {
        UMA_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "GPU.Vulkan.CreateExternalVkSemaphore", base::Time::Now() - time,
            base::TimeDelta::FromMicroseconds(1),
            base::TimeDelta::FromMicroseconds(200), 50);
      },
      base::Time::Now()));

  VkExportSemaphoreCreateInfo export_info = {
      .sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO,
      .handleTypes = handle_types,
  };

  VkSemaphoreCreateInfo sem_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &export_info,
  };

  VkSemaphore semaphore = VK_NULL_HANDLE;
  VkResult result =
      vkCreateSemaphore(vk_device, &sem_info, nullptr, &semaphore);

  if (result != VK_SUCCESS) {
    DLOG(ERROR) << "Failed to create VkSemaphore: " << result;
    return VK_NULL_HANDLE;
  }

  return semaphore;
}

std::string VkVersionToString(uint32_t version) {
  return base::StringPrintf("%u.%u.%u", VK_VERSION_MAJOR(version),
                            VK_VERSION_MINOR(version),
                            VK_VERSION_PATCH(version));
}

VkResult QueueSubmitHook(VkQueue queue,
                         uint32_t submitCount,
                         const VkSubmitInfo* pSubmits,
                         VkFence fence) {
  g_submit_count++;
  return vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

VkResult CreateGraphicsPipelinesHook(
    VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkGraphicsPipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines) {
  base::ScopedClosureRunner uma_runner(base::BindOnce(
      [](base::Time time) {
        UMA_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "GPU.Vulkan.PipelineCache.vkCreateGraphicsPipelines",
            base::Time::Now() - time, base::TimeDelta::FromMicroseconds(100),
            base::TimeDelta::FromMicroseconds(50000), 50);
      },
      base::Time::Now()));
  return vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount,
                                   pCreateInfos, pAllocator, pPipelines);
}

void RecordImportingVKSemaphoreIntoGL() {
  g_import_semaphore_into_gl_count++;
}

void ReportUMAPerSwapBuffers() {
  static uint64_t last_submit_count = 0u;
  static uint64_t last_semaphore_count = 0u;
  UMA_HISTOGRAM_CUSTOM_COUNTS("GPU.Vulkan.QueueSubmitPerSwapBuffers",
                              g_submit_count - last_submit_count, 1, 50, 50);
  UMA_HISTOGRAM_CUSTOM_COUNTS(
      "GPU.Vulkan.ImportSemaphoreGLPerSwapBuffers",
      g_import_semaphore_into_gl_count - last_semaphore_count, 1, 50, 50);
  last_submit_count = g_submit_count;
  last_semaphore_count = g_import_semaphore_into_gl_count;
}

bool CheckVulkanCompabilities(const VulkanInfo& vulkan_info,
                              const GPUInfo& gpu_info,
                              std::string enable_by_device_name) {
// Android uses AHB and SyncFD for interop. They are imported into GL with other
// API.
#if !defined(OS_ANDROID)
#if defined(OS_WIN)
  constexpr char kMemoryObjectExtension[] = "GL_EXT_memory_object_win32";
  constexpr char kSemaphoreExtension[] = "GL_EXT_semaphore_win32";
#elif defined(OS_FUCHSIA)
  constexpr char kMemoryObjectExtension[] = "GL_ANGLE_memory_object_fuchsia";
  constexpr char kSemaphoreExtension[] = "GL_ANGLE_semaphore_fuchsia";
#else
  constexpr char kMemoryObjectExtension[] = "GL_EXT_memory_object_fd";
  constexpr char kSemaphoreExtension[] = "GL_EXT_semaphore_fd";
#endif
  // If both Vulkan and GL are using native GPU (non swiftshader), check
  // necessary extensions for GL and Vulkan interop.
  const auto extensions = gfx::MakeExtensionSet(gpu_info.gl_extensions);
  if (!gfx::HasExtension(extensions, kMemoryObjectExtension) ||
      !gfx::HasExtension(extensions, kSemaphoreExtension)) {
    DLOG(ERROR) << kMemoryObjectExtension << " or " << kSemaphoreExtension
                << " is not supported.";
    return false;
  }
#endif  // !defined(OS_ANDROID)

#if defined(OS_ANDROID)
  if (vulkan_info.physical_devices.empty())
    return false;

  const auto& device_info = vulkan_info.physical_devices.front();

  auto enable_patterns = base::SplitString(
      enable_by_device_name, "|", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  for (const auto& enable_pattern : enable_patterns) {
    if (base::MatchPattern(device_info.properties.deviceName, enable_pattern))
      return true;
  }

  if (device_info.properties.vendorID == kVendorARM) {
    // https://crbug.com/1096222: Display problem with Huawei and Honor devices
    // with Mali GPU. The Mali driver version is < 19.0.0.
    if (device_info.properties.driverVersion < VK_MAKE_VERSION(19, 0, 0))
      return false;

    // Remove "Mali-" prefix.
    base::StringPiece device_name(device_info.properties.deviceName);
    if (!base::StartsWith(device_name, "Mali-")) {
      LOG(ERROR) << "Unexpected device_name " << device_name;
      return false;
    }
    device_name.remove_prefix(5);

    // Remove anything trailing a space (e.g. "G76 MC4" => "G76").
    device_name = device_name.substr(0, device_name.find(" "));

    // Older Mali GPUs are not performant with Vulkan -- this blocks all Utgard
    // gen, Midgard gen, and some Bifrost 1st & 2nd gen.
    std::vector<const char*> slow_gpus = {"2??", "3??", "4??", "T???",
                                          "G31", "G51", "G52"};
    for (base::StringPiece slow_gpu : slow_gpus) {
      if (base::MatchPattern(device_name, slow_gpu))
        return false;
    }
  }

  // https:://crbug.com/1165783: Performance is not yet as good as GL.
  if (device_info.properties.vendorID == kVendorQualcomm) {
    return false;
  }

  // https://crbug.com/1122650: Poor performance and untriaged crashes with
  // Imagination GPUs.
  if (device_info.properties.vendorID == kVendorImagination)
    return false;
#endif  // defined(OS_ANDROID)

  return true;
}

}  // namespace gpu
