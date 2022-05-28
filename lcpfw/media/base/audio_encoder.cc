// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/audio_encoder.h"

#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/time/time.h"
#include "media/base/audio_timestamp_helper.h"

namespace media {

AudioEncoder::Options::Options() = default;
AudioEncoder::Options::Options(const Options&) = default;
AudioEncoder::Options::~Options() = default;

EncodedAudioBuffer::EncodedAudioBuffer(const AudioParameters& params,
                                       std::unique_ptr<uint8_t[]> data,
                                       size_t size,
                                       base::TimeTicks timestamp)
    : params(params),
      encoded_data(std::move(data)),
      encoded_data_size(size),
      timestamp(timestamp) {}

EncodedAudioBuffer::EncodedAudioBuffer(EncodedAudioBuffer&&) = default;

EncodedAudioBuffer::~EncodedAudioBuffer() = default;

AudioEncoder::AudioEncoder() {
  DETACH_FROM_SEQUENCE(sequence_checker_);
}

AudioEncoder::~AudioEncoder() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

}  // namespace media
