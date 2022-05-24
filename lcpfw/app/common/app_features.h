
#include <unordered_map>

//#include "base/basictypes.h"
#include "base/logging.h"

class AppFeatures {
public:
    enum Feature : size_t {
        TTSDanmaku,
    };

    // (keyname, enabled)
    using FeatureInfo = std::pair<std::string, bool>;
    using FeatureMap = std::unordered_map<Feature, FeatureInfo>;

    ~AppFeatures() = default;

    // Be sure to call after the AppContext has been initialized.
    static void Init();

    static AppFeatures* current()
    {
        DCHECK(current_features_);
        return current_features_;
    }

    bool Enabled(Feature feature) const;

private:
    AppFeatures();

    void UpdateFeatureInfoFromFile(const std::string& content);

    DISALLOW_COPY_AND_ASSIGN(AppFeatures);

private:
    FeatureMap features_;

    static AppFeatures* current_features_;
};

