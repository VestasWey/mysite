// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_TRACING_PUBLIC_CPP_BACKGROUND_TRACING_BACKGROUND_TRACING_AGENT_IMPL_H_
#define SERVICES_TRACING_PUBLIC_CPP_BACKGROUND_TRACING_BACKGROUND_TRACING_AGENT_IMPL_H_

#include <stdint.h>
#include <string>

#include "base/component_export.h"
#include "base/macros.h"
#include "base/metrics/histogram.h"
#include "base/time/time.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/tracing/public/mojom/background_tracing_agent.mojom.h"

namespace base {
class SequencedTaskRunner;
}

namespace tracing {

// This class sends and receives trace messages on child processes.
class COMPONENT_EXPORT(BACKGROUND_TRACING_CPP) BackgroundTracingAgentImpl
    : public mojom::BackgroundTracingAgent {
 public:
  explicit BackgroundTracingAgentImpl(
      mojo::PendingRemote<mojom::BackgroundTracingAgentClient> client);
  ~BackgroundTracingAgentImpl() override;

  // mojom::BackgroundTracingAgent methods:
  void SetUMACallback(const std::string& histogram_name,
                      int32_t histogram_lower_value,
                      int32_t histogram_upper_value,
                      bool repeat) override;
  void ClearUMACallback(const std::string& histogram_name) override;

 private:
  static void OnHistogramChanged(
      base::WeakPtr<BackgroundTracingAgentImpl> weak_self,
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      base::Histogram::Sample reference_lower_value,
      base::Histogram::Sample reference_upper_value,
      bool repeat,
      const char* histogram_name,
      uint64_t name_hash,
      base::Histogram::Sample actual_value);
  void SendTriggerMessage(const std::string& histogram_name);
  void SendAbortBackgroundTracingMessage();

  mojo::Remote<mojom::BackgroundTracingAgentClient> client_;
  base::Time histogram_last_changed_;

  base::WeakPtrFactory<BackgroundTracingAgentImpl> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(BackgroundTracingAgentImpl);
};

}  // namespace tracing

#endif  // SERVICES_TRACING_PUBLIC_CPP_BACKGROUND_TRACING_BACKGROUND_TRACING_AGENT_IMPL_H_
