// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/virtual_fido_device_factory.h"

#include "device/fido/virtual_fido_device_discovery.h"

namespace device {
namespace test {

VirtualFidoDeviceFactory::VirtualFidoDeviceFactory() = default;
VirtualFidoDeviceFactory::~VirtualFidoDeviceFactory() = default;

void VirtualFidoDeviceFactory::SetSupportedProtocol(
    ProtocolVersion supported_protocol) {
  supported_protocol_ = supported_protocol;
}

void VirtualFidoDeviceFactory::SetTransport(FidoTransportProtocol transport) {
  transport_ = transport;
  state_->transport = transport;
}

void VirtualFidoDeviceFactory::SetCtap2Config(
    const VirtualCtap2Device::Config& config) {
  supported_protocol_ = ProtocolVersion::kCtap2;
  ctap2_config_ = config;
}

VirtualFidoDevice::State* VirtualFidoDeviceFactory::mutable_state() {
  return state_.get();
}

std::vector<std::unique_ptr<FidoDiscoveryBase>>
VirtualFidoDeviceFactory::Create(FidoTransportProtocol transport) {
  if (transport != transport_) {
    return {};
  }
  return SingleDiscovery(std::make_unique<VirtualFidoDeviceDiscovery>(
      transport_, state_, supported_protocol_, ctap2_config_));
}

bool VirtualFidoDeviceFactory::IsTestOverride() {
  return true;
}

}  // namespace test
}  // namespace device
