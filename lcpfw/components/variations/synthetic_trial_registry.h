// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VARIATIONS_SYNTHETIC_TRIAL_REGISTRY_H_
#define COMPONENTS_VARIATIONS_SYNTHETIC_TRIAL_REGISTRY_H_

#include <vector>

#include "base/feature_list.h"
#include "base/observer_list.h"
#include "components/variations/synthetic_trials.h"

namespace metrics {
class MetricsServiceAccessor;
}  // namespace metrics

namespace variations {

struct ActiveGroupId;
class FieldTrialsProvider;
class FieldTrialsProviderTest;
class SyntheticTrialRegistryTest;

namespace internal {
extern const base::Feature kExternalExperimentAllowlist;
}  // namespace internal

class SyntheticTrialRegistry {
 public:
  // Constructor that specifies whether the SyntheticTrialRegistry should use
  // an allowlist for external experiments. Some embedders such as WebLayer
  // do not run as Chrome and do not use the allowlist.
  // Note: The allowlist is enabled only if |kExternalExperimentAllowlist| is
  // also enabled, even if the parameter value is true. The default constructor
  // defaults to the feature state.
  explicit SyntheticTrialRegistry(bool enable_external_experiment_allowlist);

  SyntheticTrialRegistry();
  ~SyntheticTrialRegistry();

  // Adds an observer to be notified when the synthetic trials list changes.
  void AddSyntheticTrialObserver(SyntheticTrialObserver* observer);

  // Removes an existing observer of synthetic trials list changes.
  void RemoveSyntheticTrialObserver(SyntheticTrialObserver* observer);

  // Specifies the mode of RegisterExternalExperiments() operation.
  enum OverrideMode {
    // Previously-registered external experiment ids are overridden (replaced)
    // with the new list.
    kOverrideExistingIds,
    // Previously-registered external experiment ids are not overridden, but
    // new experiment ids may be added.
    kDoNotOverrideExistingIds,
  };

  // Registers a list of experiment ids coming from an external application.
  // The input ids are in the VariationID format.
  //
  // When |enable_external_experiment_allowlist| is true, the supplied ids must
  // have corresponding entries in the "ExternalExperimentAllowlist" (coming via
  // a feature param) to be applied. The allowlist also supplies the
  // corresponding trial name that should be used for reporting to UMA.
  //
  // When |enable_external_experiment_allowlist| is false, |fallback_study_name|
  // will be used as the trial name for all provided experiment ids.
  //
  // If |mode| is kOverrideExistingIds, this API clears previously-registered
  // external experiment ids, replacing them with the new list (which may be
  // empty). If |mode| is kDoNotOverrideExistingIds, any new ids that are not
  // already registered will be added, but existing ones will not be replaced.
  void RegisterExternalExperiments(const std::string& fallback_study_name,
                                   const std::vector<int>& experiment_ids,
                                   OverrideMode mode);

 private:
  friend metrics::MetricsServiceAccessor;
  friend FieldTrialsProvider;
  friend FieldTrialsProviderTest;
  friend SyntheticTrialRegistryTest;
  FRIEND_TEST_ALL_PREFIXES(SyntheticTrialRegistryTest, RegisterSyntheticTrial);
  FRIEND_TEST_ALL_PREFIXES(SyntheticTrialRegistryTest,
                           GetSyntheticFieldTrialActiveGroups);
  FRIEND_TEST_ALL_PREFIXES(VariationsCrashKeysTest, BasicFunctionality);

  // Registers a field trial name and group to be used to annotate a UMA report
  // with a particular Chrome configuration state.
  //
  // A UMA report will be annotated with this trial group if and only if all
  // events in the report were created after the trial is registered. Only one
  // group name may be registered at a time for a given trial_name. Only the
  // last group name that is registered for a given trial name will be recorded.
  // The values passed in must not correspond to any real field trial in the
  // code.
  //
  // The registered trials are not persisted to disk and will not be applied
  // after a restart.
  //
  // Note: Should not be used to replace trials that were
  // registered with RegisterExternalExperiments().
  void RegisterSyntheticFieldTrial(const SyntheticTrialGroup& trial_group);

  // Returns the study name corresponding to |experiment_id| from the allowlist
  // contained in |params| if the allowlist is enabled, otherwise returns
  // |fallback_study_name|. An empty string piece is returned when the
  // experiment is not in the allowlist.
  base::StringPiece GetStudyNameForExpId(const std::string& fallback_study_name,
                                         const base::FieldTrialParams& params,
                                         const std::string& experiment_id);

  // Returns a list of synthetic field trials that are older than |time|.
  void GetSyntheticFieldTrialsOlderThan(
      base::TimeTicks time,
      std::vector<ActiveGroupId>* synthetic_trials) const;

  // Notifies observers on a synthetic trial list change.
  void NotifySyntheticTrialObservers();

  // Whether the allowlist is enabled. Some configurations, like WebLayer
  // do not use the allowlist.
  bool enable_external_experiment_allowlist_ = true;

  // Field trial groups that map to Chrome configuration states.
  std::vector<SyntheticTrialGroup> synthetic_trial_groups_;

  // List of observers of |synthetic_trial_groups_| changes.
  base::ObserverList<SyntheticTrialObserver>::Unchecked
      synthetic_trial_observer_list_;
};

}  // namespace variations

#endif  // COMPONENTS_VARIATIONS_SYNTHETIC_TRIAL_REGISTRY_H_
