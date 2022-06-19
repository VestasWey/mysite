// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_TRUST_TOKENS_BORINGSSL_TRUST_TOKEN_ISSUANCE_CRYPTOGRAPHER_H_
#define SERVICES_NETWORK_TRUST_TOKENS_BORINGSSL_TRUST_TOKEN_ISSUANCE_CRYPTOGRAPHER_H_

#include <string>

#include "base/containers/flat_map.h"
#include "services/network/trust_tokens/trust_token_request_issuance_helper.h"
#include "third_party/boringssl/src/include/openssl/base.h"

namespace network {

// Executes one instance of the Trust Tokens protocol's issuance operation by
// calling the appropriate BoringSSL methods.
class BoringsslTrustTokenIssuanceCryptographer
    : public TrustTokenRequestIssuanceHelper::Cryptographer {
 public:
  BoringsslTrustTokenIssuanceCryptographer();
  ~BoringsslTrustTokenIssuanceCryptographer() override;

  // TrustTokenRequestIssuanceHelper::Cryptographer implementation:
  bool Initialize(mojom::TrustTokenProtocolVersion issuer_configured_version,
                  int issuer_configured_batch_size) override;
  bool AddKey(base::StringPiece key) override;
  base::Optional<std::string> BeginIssuance(size_t num_tokens) override;
  std::unique_ptr<UnblindedTokens> ConfirmIssuance(
      base::StringPiece response_header) override;

 private:
  // Maintains Trust Tokens protocol state.
  bssl::UniquePtr<TRUST_TOKEN_CLIENT> ctx_;

  // Maps key indices, which are arbitrary but unique values provided by Boring
  // when we call AddKey, to the corresponding key material. When concluding
  // issuance, we need this information in order to retrieve the key material
  // corresponding to the index, since Boring just tells us the index of the key
  // used to issue the tokens.
  base::flat_map<size_t, std::string> keys_by_index_;
};

}  // namespace network

#endif  // SERVICES_NETWORK_TRUST_TOKENS_BORINGSSL_TRUST_TOKEN_ISSUANCE_CRYPTOGRAPHER_H_
