// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/public/cpp/cors/cors_error_status.h"

#include <ostream>

#include "net/base/net_errors.h"
#include "services/network/public/mojom/cors.mojom-shared.h"
#include "services/network/public/mojom/ip_address_space.mojom-shared.h"

namespace network {

CorsErrorStatus::CorsErrorStatus() = default;

CorsErrorStatus::CorsErrorStatus(const CorsErrorStatus&) = default;
CorsErrorStatus& CorsErrorStatus::operator=(const CorsErrorStatus&) = default;
CorsErrorStatus::CorsErrorStatus(CorsErrorStatus&&) = default;
CorsErrorStatus& CorsErrorStatus::operator=(CorsErrorStatus&&) = default;

CorsErrorStatus::CorsErrorStatus(mojom::CorsError cors_error)
    : cors_error(cors_error) {}

CorsErrorStatus::CorsErrorStatus(mojom::CorsError cors_error,
                                 const std::string& failed_parameter)
    : cors_error(cors_error), failed_parameter(failed_parameter) {}

CorsErrorStatus::CorsErrorStatus(mojom::IPAddressSpace resource_address_space)
    : cors_error(mojom::CorsError::kInsecurePrivateNetwork),
      resource_address_space(resource_address_space) {}

CorsErrorStatus::~CorsErrorStatus() = default;

bool CorsErrorStatus::operator==(const CorsErrorStatus& rhs) const {
  return cors_error == rhs.cors_error &&
         failed_parameter == rhs.failed_parameter &&
         resource_address_space == rhs.resource_address_space;
}

std::ostream& operator<<(std::ostream& os, const CorsErrorStatus& status) {
  return os << "CorsErrorStatus{ cors_error = " << status.cors_error
            << ", failed_parameter = " << status.failed_parameter
            << ", resource_address_space = " << status.resource_address_space
            << " }";
}

}  // namespace network
