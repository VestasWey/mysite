// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/bio/enrollment_handler.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "components/device_event_log/device_event_log.h"
#include "device/fido/fido_authenticator.h"
#include "device/fido/fido_constants.h"
#include "device/fido/pin.h"

namespace device {

BioEnrollmentHandler::BioEnrollmentHandler(
    const base::flat_set<FidoTransportProtocol>& supported_transports,
    base::OnceClosure ready_callback,
    ErrorCallback error_callback,
    GetPINCallback get_pin_callback,
    FidoDiscoveryFactory* factory)
    : FidoRequestHandlerBase(factory, supported_transports),
      ready_callback_(std::move(ready_callback)),
      error_callback_(std::move(error_callback)),
      get_pin_callback_(std::move(get_pin_callback)) {
  Start();
}

BioEnrollmentHandler::~BioEnrollmentHandler() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void BioEnrollmentHandler::EnrollTemplate(
    SampleCallback sample_callback,
    CompletionCallback completion_callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kReady);
  state_ = State::kEnrolling;
  sample_callback_ = std::move(sample_callback);
  enrollment_callback_ = std::move(completion_callback);
  bio_enroller_ =
      std::make_unique<BioEnroller>(this, authenticator_, *pin_token_response_);
}

void BioEnrollmentHandler::CancelEnrollment() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kEnrolling);
  state_ = State::kCancellingEnrollment;
  bio_enroller_->Cancel();
}

void BioEnrollmentHandler::EnumerateTemplates(EnumerationCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(pin_token_response_);
  DCHECK_EQ(state_, State::kReady);
  state_ = State::kEnumerating;
  authenticator_->BioEnrollEnumerate(
      *pin_token_response_,
      base::BindOnce(&BioEnrollmentHandler::OnEnumerateTemplates,
                     weak_factory_.GetWeakPtr(), std::move(callback)));
}

void BioEnrollmentHandler::RenameTemplate(std::vector<uint8_t> template_id,
                                          std::string name,
                                          StatusCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kReady);
  state_ = State::kRenaming;
  authenticator_->BioEnrollRename(
      *pin_token_response_, std::move(template_id), std::move(name),
      base::BindOnce(&BioEnrollmentHandler::OnRenameTemplate,
                     weak_factory_.GetWeakPtr(), std::move(callback)));
}

void BioEnrollmentHandler::DeleteTemplate(std::vector<uint8_t> template_id,
                                          StatusCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kReady);
  state_ = State::kDeleting;
  authenticator_->BioEnrollDelete(
      *pin_token_response_, std::move(template_id),
      base::BindOnce(&BioEnrollmentHandler::OnDeleteTemplate,
                     weak_factory_.GetWeakPtr(), std::move(callback)));
}

void BioEnrollmentHandler::DispatchRequest(FidoAuthenticator* authenticator) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (state_ != State::kWaitingForTouch) {
    return;
  }
  authenticator->GetTouch(base::BindOnce(&BioEnrollmentHandler::OnTouch,
                                         weak_factory_.GetWeakPtr(),
                                         authenticator));
}

void BioEnrollmentHandler::AuthenticatorRemoved(
    FidoDiscoveryBase* discovery,
    FidoAuthenticator* authenticator) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  FidoRequestHandlerBase::AuthenticatorRemoved(discovery, authenticator);
  if (authenticator_ != authenticator || state_ == State::kFinished) {
    return;
  }

  authenticator_ = nullptr;
  Finish(BioEnrollmentStatus::kSuccess);
}

void BioEnrollmentHandler::OnSampleCollected(BioEnrollmentSampleStatus status,
                                             int samples_remaining) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kEnrolling);
  sample_callback_.Run(status, samples_remaining);
}

void BioEnrollmentHandler::OnEnrollmentDone(
    base::Optional<std::vector<uint8_t>> template_id) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  bio_enroller_.reset();
  if (!template_id) {
    DCHECK_EQ(state_, State::kCancellingEnrollment);
    state_ = State::kReady;
    std::move(enrollment_callback_)
        .Run(CtapDeviceResponseCode::kCtap2ErrKeepAliveCancel, {});
    return;
  }
  DCHECK(state_ == State::kEnrolling || state_ == State::kCancellingEnrollment);
  state_ = State::kReady;
  std::move(enrollment_callback_)
      .Run(CtapDeviceResponseCode::kSuccess, std::move(*template_id));
}

void BioEnrollmentHandler::OnEnrollmentError(CtapDeviceResponseCode status) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(state_ == State::kEnrolling || state_ == State::kCancellingEnrollment);
  bio_enroller_.reset();
  state_ = State::kReady;
  std::move(enrollment_callback_).Run(status, {});
}

void BioEnrollmentHandler::OnTouch(FidoAuthenticator* authenticator) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (state_ != State::kWaitingForTouch) {
    return;
  }

  CancelActiveAuthenticators(authenticator->GetId());

  if (!authenticator->Options() ||
      (authenticator->Options()->bio_enrollment_availability ==
           AuthenticatorSupportedOptions::BioEnrollmentAvailability::
               kNotSupported &&
       authenticator->Options()->bio_enrollment_availability_preview ==
           AuthenticatorSupportedOptions::BioEnrollmentAvailability::
               kNotSupported)) {
    Finish(BioEnrollmentStatus::kAuthenticatorMissingBioEnrollment);
    return;
  }

  if (authenticator->Options()->client_pin_availability !=
      AuthenticatorSupportedOptions::ClientPinAvailability::
          kSupportedAndPinSet) {
    Finish(BioEnrollmentStatus::kNoPINSet);
    return;
  }

  if (authenticator->ForcePINChange()) {
    Finish(BioEnrollmentStatus::kForcePINChange);
    return;
  }

  authenticator_ = authenticator;
  state_ = State::kGettingRetries;
  authenticator_->GetPinRetries(base::BindOnce(
      &BioEnrollmentHandler::OnRetriesResponse, weak_factory_.GetWeakPtr()));
}

void BioEnrollmentHandler::OnRetriesResponse(
    CtapDeviceResponseCode status,
    base::Optional<pin::RetriesResponse> response) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kGettingRetries);
  if (!response || status != CtapDeviceResponseCode::kSuccess) {
    Finish(BioEnrollmentStatus::kAuthenticatorResponseInvalid);
    return;
  }

  if (response->retries == 0) {
    Finish(BioEnrollmentStatus::kHardPINBlock);
    return;
  }

  state_ = State::kWaitingForPIN;
  get_pin_callback_.Run(authenticator_->CurrentMinPINLength(),
                        response->retries,
                        base::BindOnce(&BioEnrollmentHandler::OnHavePIN,
                                       weak_factory_.GetWeakPtr()));
}

void BioEnrollmentHandler::OnHavePIN(std::string pin) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kWaitingForPIN);
  state_ = State::kGettingPINToken;
  authenticator_->GetPINToken(
      std::move(pin), {pin::Permissions::kBioEnrollment},
      /*rp_id=*/base::nullopt,
      base::BindOnce(&BioEnrollmentHandler::OnHavePINToken,
                     weak_factory_.GetWeakPtr()));
}

void BioEnrollmentHandler::OnHavePINToken(
    CtapDeviceResponseCode status,
    base::Optional<pin::TokenResponse> response) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kGettingPINToken);

  if (status != CtapDeviceResponseCode::kSuccess) {
    switch (status) {
      case CtapDeviceResponseCode::kCtap2ErrPinInvalid:
        state_ = State::kGettingRetries;
        authenticator_->GetPinRetries(
            base::BindOnce(&BioEnrollmentHandler::OnRetriesResponse,
                           weak_factory_.GetWeakPtr()));
        return;
      case CtapDeviceResponseCode::kCtap2ErrPinAuthBlocked:
        Finish(BioEnrollmentStatus::kSoftPINBlock);
        return;
      case CtapDeviceResponseCode::kCtap2ErrPinBlocked:
        Finish(BioEnrollmentStatus::kHardPINBlock);
        return;
      default:
        Finish(BioEnrollmentStatus::kAuthenticatorResponseInvalid);
        return;
    }
  }

  state_ = State::kReady;
  pin_token_response_ = std::move(response);
  std::move(ready_callback_).Run();
}

void BioEnrollmentHandler::OnEnumerateTemplates(
    EnumerationCallback callback,
    CtapDeviceResponseCode status,
    base::Optional<BioEnrollmentResponse> response) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kEnumerating);

  state_ = State::kReady;

  if (status != CtapDeviceResponseCode::kSuccess) {
    std::move(callback).Run(status, base::nullopt);
    return;
  }

  if (!response || !response->template_infos) {
    Finish(BioEnrollmentStatus::kAuthenticatorResponseInvalid);
    return;
  }

  std::move(callback).Run(status, std::move(*response->template_infos));
}

void BioEnrollmentHandler::OnRenameTemplate(
    StatusCallback callback,
    CtapDeviceResponseCode status,
    base::Optional<BioEnrollmentResponse> response) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kRenaming);
  state_ = State::kReady;
  std::move(callback).Run(status);
}

void BioEnrollmentHandler::OnDeleteTemplate(
    StatusCallback callback,
    CtapDeviceResponseCode status,
    base::Optional<BioEnrollmentResponse> response) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_EQ(state_, State::kDeleting);
  state_ = State::kReady;
  std::move(callback).Run(status);
}

void BioEnrollmentHandler::Finish(BioEnrollmentStatus status) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK_NE(state_, State::kFinished);
  state_ = State::kFinished;
  std::move(error_callback_).Run(status);
}

}  // namespace device
