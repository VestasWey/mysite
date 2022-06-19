// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/fido_authenticator.h"

#include <utility>

#include "base/callback.h"
#include "base/notreached.h"

namespace device {

void FidoAuthenticator::GetNextAssertion(
    FidoAuthenticator::GetAssertionCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::GetTouch(base::OnceCallback<void()> callback) {}

void FidoAuthenticator::GetPinRetries(
    FidoAuthenticator::GetRetriesCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::GetPINToken(
    std::string pin,
    std::vector<pin::Permissions> permissions,
    base::Optional<std::string> rp_id,
    FidoAuthenticator::GetTokenCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::GetUvRetries(
    FidoAuthenticator::GetRetriesCallback callback) {
  NOTREACHED();
}

bool FidoAuthenticator::CanGetUvToken() {
  return false;
}

void FidoAuthenticator::GetUvToken(
    std::vector<pin::Permissions> permissions,
    base::Optional<std::string> rp_id,
    FidoAuthenticator::GetTokenCallback callback) {
  NOTREACHED();
}

uint32_t FidoAuthenticator::CurrentMinPINLength() {
  NOTREACHED();
  return kMinPinLength;
}

uint32_t FidoAuthenticator::NewMinPINLength() {
  NOTREACHED();
  return kMinPinLength;
}

bool FidoAuthenticator::ForcePINChange() {
  NOTREACHED();
  return false;
}

void FidoAuthenticator::SetPIN(const std::string& pin,
                               FidoAuthenticator::SetPINCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::ChangePIN(const std::string& old_pin,
                                  const std::string& new_pin,
                                  SetPINCallback callback) {
  NOTREACHED();
}

FidoAuthenticator::PINUVDisposition
FidoAuthenticator::PINUVDispositionForMakeCredential(
    const CtapMakeCredentialRequest& request,
    const FidoRequestHandlerBase::Observer* observer) {
  return PINUVDisposition::kNoUV;
}

FidoAuthenticator::PINUVDisposition
FidoAuthenticator::PINUVDispositionForGetAssertion(
    const CtapGetAssertionRequest& request,
    const FidoRequestHandlerBase::Observer* observer) {
  return PINUVDisposition::kNoUV;
}

void FidoAuthenticator::GetCredentialsMetadata(
    const pin::TokenResponse& pin_token,
    GetCredentialsMetadataCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::EnumerateCredentials(
    const pin::TokenResponse& pin_token,
    EnumerateCredentialsCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::DeleteCredential(
    const pin::TokenResponse& pin_token,
    const PublicKeyCredentialDescriptor& credential_id,
    DeleteCredentialCallback callback) {
  NOTREACHED();
}

void FidoAuthenticator::GetModality(BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::GetSensorInfo(BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::BioEnrollFingerprint(
    const pin::TokenResponse&,
    base::Optional<std::vector<uint8_t>> template_id,
    BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::BioEnrollCancel(BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::BioEnrollEnumerate(const pin::TokenResponse&,
                                           BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::BioEnrollRename(const pin::TokenResponse&,
                                        std::vector<uint8_t>,
                                        std::string,
                                        BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::BioEnrollDelete(const pin::TokenResponse&,
                                        std::vector<uint8_t>,
                                        BioEnrollmentCallback) {
  NOTREACHED();
}

void FidoAuthenticator::WriteLargeBlob(
    const std::vector<uint8_t>& large_blob,
    const LargeBlobKey& large_blob_key,
    const base::Optional<pin::TokenResponse> pin_uv_auth_token,
    base::OnceCallback<void(CtapDeviceResponseCode)> callback) {
  NOTREACHED();
}

void FidoAuthenticator::ReadLargeBlob(
    const std::vector<LargeBlobKey>& large_blob_keys,
    const base::Optional<pin::TokenResponse> pin_uv_auth_token,
    LargeBlobReadCallback callback) {
  NOTREACHED();
}

base::Optional<base::span<const int32_t>> FidoAuthenticator::GetAlgorithms() {
  return base::nullopt;
}

bool FidoAuthenticator::DiscoverableCredentialStorageFull() const {
  return false;
}

void FidoAuthenticator::Reset(ResetCallback callback) {
  std::move(callback).Run(CtapDeviceResponseCode::kCtap1ErrInvalidCommand,
                          base::nullopt);
}

std::string FidoAuthenticator::GetDisplayName() const {
  return GetId();
}

ProtocolVersion FidoAuthenticator::SupportedProtocol() const {
  return ProtocolVersion::kUnknown;
}

bool FidoAuthenticator::SupportsCredProtectExtension() const {
  return Options() && Options()->supports_cred_protect;
}

bool FidoAuthenticator::SupportsHMACSecretExtension() const {
  return false;
}

bool FidoAuthenticator::SupportsEnterpriseAttestation() const {
  return false;
}

}  // namespace device
