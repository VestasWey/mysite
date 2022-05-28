// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/metrics/public/cpp/ukm_source_id.h"

#include <cmath>

#include "base/atomic_sequence_num.h"
#include "base/check.h"
#include "base/check_op.h"
#include "base/rand_util.h"

namespace ukm {

namespace {

const int64_t kLowBitsMask = (INT64_C(1) << 32) - 1;

int64_t GetNumTypeBits() {
  return std::ceil(
      std::log2(static_cast<int64_t>(SourceIdObj::Type::kMaxValue) + 1));
}

}  // namespace

// static
SourceIdObj SourceIdObj::New() {
  // Generate some bits which are unique to this process, so we can generate
  // IDs independently in different processes. IDs generated by this method may
  // collide, but it should be sufficiently rare enough to not impact data
  // quality.
  const static int64_t process_id_bits =
      static_cast<int64_t>(base::RandUint64()) & ~kLowBitsMask;
  // Generate some bits which are unique within the process, using a counter.
  static base::AtomicSequenceNumber seq;
  SourceIdObj local_id =
      FromOtherId(seq.GetNext() + 1, SourceIdObj::Type::DEFAULT);
  // Combine the local and process bits to generate a unique ID.
  return SourceIdObj((local_id.value_ & kLowBitsMask) | process_id_bits);
}

// static
SourceIdObj SourceIdObj::FromOtherId(int64_t other_id, SourceIdObj::Type type) {
  // Note on syntax: std::ceil and std::log2 are not constexpr functions thus
  // these variables cannot be initialized statically in the global scope above.
  // Function static initialization here is thread safe; so they are initialized
  // at most once.
  static const int64_t kNumTypeBits = GetNumTypeBits();
  static const int64_t kTypeMask = (INT64_C(1) << kNumTypeBits) - 1;

  const int64_t type_bits = static_cast<int64_t>(type);
  DCHECK_EQ(type_bits, type_bits & kTypeMask);
  // Stores the type of the source ID in its lower bits, and shift the rest of
  // the ID to make room. This could cause the original ID to overflow, but
  // that should be rare enough that it won't matter for UKM's purposes.
  return SourceIdObj((other_id << kNumTypeBits) | type_bits);
}

SourceIdObj::Type SourceIdObj::GetType() const {
  static const int64_t kNumTypeBits = GetNumTypeBits();
  static const int64_t kTypeMask = (INT64_C(1) << kNumTypeBits) - 1;
  return static_cast<SourceIdObj::Type>(value_ & kTypeMask);
}

SourceId AssignNewSourceId() {
  return ukm::SourceIdObj::New().ToInt64();
}

SourceId ConvertToSourceId(int64_t other_id, SourceIdType id_type) {
  // DCHECK is to restrict the usage of WEBAPK_ID and PAYMENT_APP_ID. WebApk and
  // Payment apps should use |UkmRecorder::GetSourceIdForWebApkManifestUrl()|
  // and |UkmRecorder::GetSourceIdForPaymentAppFromScope()| instead.
  // TODO(crbug.com/1046964): Ideally we should restrict
  // SourceIdObj::FromOtherId() as well.
  DCHECK(id_type != SourceIdType::WEBAPK_ID);
  DCHECK(id_type != SourceIdType::PAYMENT_APP_ID);
  return ukm::SourceIdObj::FromOtherId(other_id, id_type).ToInt64();
}

SourceIdType GetSourceIdType(SourceId source_id) {
  return ukm::SourceIdObj::FromInt64(source_id).GetType();
}

}  // namespace ukm
