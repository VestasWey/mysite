// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/fido/filter.h"

#include "base/feature_list.h"
#include "base/json/json_reader.h"
#include "base/no_destructor.h"
#include "base/optional.h"
#include "base/strings/pattern.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "components/device_event_log/device_event_log.h"

namespace device {
namespace fido_filter {

namespace {

const base::Feature kFilter{"WebAuthenticationFilter",
                            base::FEATURE_DISABLED_BY_DEFAULT};

const base::FeatureParam<std::string> kFilterJSON{
    &kFilter,
    "json",
    "",
};

struct FilterStep {
  base::Optional<std::string> operation;
  std::vector<std::string> rp_id;
  base::Optional<std::string> device;
  base::Optional<std::string> id_type;
  std::vector<std::string> id;
  base::Optional<size_t> id_min_size;
  base::Optional<size_t> id_max_size;
  Action action;
};

bool IsString(const base::Value& v) {
  return v.is_string();
}

bool IsNonEmptyString(const base::Value& v) {
  return v.is_string() && !v.GetString().empty();
}

bool IsListOf(const base::Value* v, bool (*predicate)(const base::Value&)) {
  if (!v->is_list()) {
    return false;
  }
  auto contents = v->GetList();
  return !contents.empty() &&
         std::all_of(contents.begin(), contents.end(), predicate);
}

std::vector<std::string> GetStringOrListOfStrings(const base::Value* v) {
  if (v->is_string()) {
    return {v->GetString()};
  }

  std::vector<std::string> ret;
  for (const auto& elem : v->GetList()) {
    ret.push_back(elem.GetString());
  }
  return ret;
}

base::Optional<std::vector<FilterStep>> ParseJSON(base::StringPiece json) {
  base::Optional<base::Value> v =
      base::JSONReader::Read(json, base::JSON_ALLOW_TRAILING_COMMAS);
  if (!v || !v->is_dict()) {
    return base::nullopt;
  }

  const base::Value* filters = v->FindKey("filters");
  if (!filters || !filters->is_list()) {
    return base::nullopt;
  }

  std::vector<FilterStep> ret;
  const auto filter_list = filters->GetList();
  for (const auto& filter : filter_list) {
    if (!filter.is_dict()) {
      return base::nullopt;
    }

    // These are the keys that are extracted from the JSON:
    const base::Value* operation = nullptr;
    const base::Value* rp_id = nullptr;
    const base::Value* device = nullptr;
    const base::Value* id_type = nullptr;
    const base::Value* id = nullptr;
    const base::Value* id_min_size = nullptr;
    const base::Value* id_max_size = nullptr;
    const base::Value* action = nullptr;

    // DictItems is used so that unknown keys in the dictionary can be rejected.
    for (const auto& pair : filter.DictItems()) {
      if (pair.first == "operation") {
        operation = &pair.second;
      } else if (pair.first == "rp_id") {
        rp_id = &pair.second;
      } else if (pair.first == "device") {
        device = &pair.second;
      } else if (pair.first == "id_type") {
        id_type = &pair.second;
      } else if (pair.first == "id") {
        id = &pair.second;
      } else if (pair.first == "id_min_size") {
        id_min_size = &pair.second;
      } else if (pair.first == "id_max_size") {
        id_max_size = &pair.second;
      } else if (pair.first == "action") {
        action = &pair.second;
      } else {
        // Unknown keys are an error.
        return base::nullopt;
      }
    }

    if (!action || !IsNonEmptyString(*action) ||
        (operation && !IsNonEmptyString(*operation)) ||
        (rp_id && !IsNonEmptyString(*rp_id) &&
         !IsListOf(rp_id, IsNonEmptyString)) ||
        (device && !IsNonEmptyString(*device)) ||
        (id_type && !IsNonEmptyString(*id_type)) ||
        (id && !IsString(*id) && !IsListOf(id, IsString)) ||
        (id_min_size && !id_min_size->is_int()) ||
        (id_max_size && !id_max_size->is_int())) {
      return base::nullopt;
    }

    if ((id_min_size || id_max_size || id) && !id_type) {
      // If matches on the contents or size of an ID are given then the type
      // must also be matched.
      return base::nullopt;
    }

    if (!rp_id && !device) {
      // Filter is too broad. For safety this is disallowed, although one can
      // still explicitly use a wildcard.
      return base::nullopt;
    }

    FilterStep step;
    const std::string& action_str = action->GetString();
    if (action_str == "allow") {
      step.action = Action::ALLOW;
    } else if (action_str == "block") {
      step.action = Action::BLOCK;
    } else if (action_str == "no-attestation") {
      step.action = Action::NO_ATTESTATION;
    } else {
      return base::nullopt;
    }

    if (operation) {
      step.operation = operation->GetString();
    }
    if (rp_id) {
      step.rp_id = GetStringOrListOfStrings(rp_id);
    }
    if (device) {
      step.device = device->GetString();
    }
    if (id_type) {
      step.id_type = id_type->GetString();
    }
    if (id) {
      step.id = GetStringOrListOfStrings(id);
    }
    if (id_min_size) {
      const int v = id_min_size->GetInt();
      if (v < 0) {
        return base::nullopt;
      }
      step.id_min_size = v;
    }
    if (id_max_size) {
      const int v = id_max_size->GetInt();
      if (v < 0) {
        return base::nullopt;
      }
      step.id_max_size = v;
    }

    ret.emplace_back(std::move(step));
  }

  return ret;
}

const char* OperationToString(Operation op) {
  switch (op) {
    case Operation::MAKE_CREDENTIAL:
      return "mc";
    case Operation::GET_ASSERTION:
      return "ga";
  }
}

const char* IDTypeToString(IDType id_type) {
  switch (id_type) {
    case IDType::CREDENTIAL_ID:
      return "cred";
    case IDType::USER_ID:
      return "user";
  }
}

size_t g_testing_depth = 0;

struct CurrentFilter {
  base::Optional<std::vector<FilterStep>> steps;
  base::Optional<std::string> json;
};

CurrentFilter* GetCurrentFilter() {
  static base::NoDestructor<CurrentFilter> current_filter;
  return current_filter.get();
}

bool MaybeParseFilter(base::StringPiece json) {
  CurrentFilter* const current_filter = GetCurrentFilter();
  if (current_filter->json && json == *current_filter->json) {
    return true;
  }

  if (json.size() == 0) {
    current_filter->steps.reset();
    current_filter->json = "";
    return true;
  }

  current_filter->steps = ParseJSON(json);
  if (!current_filter->steps) {
    current_filter->json.reset();
    return false;
  }

  current_filter->json = json.as_string();
  return true;
}

}  // namespace

void MaybeInitialize() {
  if (g_testing_depth != 0) {
    return;
  }

  const std::string& json = kFilterJSON.Get();
  if (!MaybeParseFilter(json)) {
    FIDO_LOG(ERROR) << "Failed to parse filter JSON. Failing open.";
  }
}

Action Evaluate(
    Operation op,
    base::StringPiece rp_id,
    base::Optional<base::StringPiece> device,
    base::Optional<std::pair<IDType, base::span<const uint8_t>>> id) {
  CurrentFilter* const current_filter = GetCurrentFilter();
  if (!current_filter->steps) {
    return Action::ALLOW;
  }

  base::Optional<std::string> id_hex;
  if (id) {
    id_hex = base::HexEncode(id->second);
  }

  for (const auto& filter : *current_filter->steps) {
    if ((!filter.operation ||
         base::MatchPattern(OperationToString(op), *filter.operation)) &&
        (filter.rp_id.empty() ||
         std::any_of(filter.rp_id.begin(), filter.rp_id.end(),
                     [rp_id](const std::string& pattern) -> bool {
                       return base::MatchPattern(rp_id, pattern);
                     })) &&
        (!filter.device ||
         base::MatchPattern(device.value_or(""), *filter.device)) &&
        (!filter.id_type || (id && base::MatchPattern(IDTypeToString(id->first),
                                                      *filter.id_type))) &&
        (!filter.id_min_size ||
         (id && *filter.id_min_size <= id->second.size())) &&
        (!filter.id_max_size ||
         (id && *filter.id_max_size >= id->second.size())) &&
        (filter.id.empty() ||
         (id_hex && std::any_of(filter.id.begin(), filter.id.end(),
                                [&id_hex](const std::string& pattern) -> bool {
                                  return base::MatchPattern(*id_hex, pattern);
                                })))) {
      return filter.action;
    }
  }

  return Action::ALLOW;
}

ScopedFilterForTesting::ScopedFilterForTesting(base::StringPiece json)
    : previous_json_(GetCurrentFilter()->json) {
  g_testing_depth++;
  CHECK(g_testing_depth != 0);
  CHECK(MaybeParseFilter(json)) << json;
}

ScopedFilterForTesting::ScopedFilterForTesting(
    base::StringPiece json,
    ScopedFilterForTesting::PermitInvalidJSON)
    : previous_json_(GetCurrentFilter()->json) {
  g_testing_depth++;
  CHECK(g_testing_depth != 0);
  MaybeParseFilter(json);
}

ScopedFilterForTesting::~ScopedFilterForTesting() {
  CurrentFilter* const current_filter = GetCurrentFilter();
  current_filter->steps.reset();
  current_filter->json.reset();
  g_testing_depth--;

  if (previous_json_) {
    CHECK(MaybeParseFilter(*previous_json_));
  }
}

bool ParseForTesting(base::StringPiece json) {
  CHECK(base::JSONReader::Read(json, base::JSON_ALLOW_TRAILING_COMMAS)) << json;
  return MaybeParseFilter(json);
}

}  // namespace fido_filter
}  // namespace device
