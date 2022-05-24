/*
 @ 0xCCCCCCCC
*/

#include "common/app_features.h"

#include "base/files/file_util.h"
//#include "base/ini_parser.h"
#include "base/strings/string_number_conversions.h"

#include "common/app_context.h"

namespace {

using Feature = AppFeatures::Feature;

const base::FilePath::CharType kFeatureFileName[] = FILE_PATH_LITERAL("features");

AppFeatures::FeatureMap BuildFeatures()
{
    return {
        {Feature::TTSDanmaku, {"livehime.tts_danmaku", false}}
    };
}

// Returns true if value was parsed successfully, and `enabled` contains the parsed value.
// Returns false otherwise, and `enabled` won't be touched.
//bool ParseFeatureEnabled(const std::string& value, bool& enabled)
//{
//    int number = -1;
//    if (!base::StringToInt(value, &number) || number < 0) {
//        NOTREACHED() << "Suspicious feature value detected! value: " << value;
//        return false;
//    }
//
//    enabled = number != 0;
//
//    return true;
//}

}   // namespace

// The instance is leaked on purpose.
AppFeatures* AppFeatures::current_features_ = nullptr;

void AppFeatures::Init()
{
    DCHECK(!current_features_);

    current_features_ = new AppFeatures();
}

AppFeatures::AppFeatures()
    : features_(BuildFeatures())
{
    auto feature_path = base::MakeAbsoluteFilePath(
        AppContext::Current()->GetMainDirectory().Append(kFeatureFileName));

    std::string content;
    base::ReadFileToString(feature_path, &content);
    if (content.empty()) {
        return;
    }

    UpdateFeatureInfoFromFile(content);
}

bool AppFeatures::Enabled(Feature feature) const
{
    auto it = features_.find(feature);

    if (it == features_.cend()) {
        NOTREACHED() << "Unknown feature: " << feature;
        return false;
    }

    return it->second.second;
}

void AppFeatures::UpdateFeatureInfoFromFile(const std::string& content)
{
    /*base::DictionaryValueINIParser ini;
    ini.Parse(content);

    for (auto& feature_info : features_) {
        const std::string& key = feature_info.second.first;
        std::string value;
        bool enabled = false;
        if (ini.root().GetString(key, &value) && ParseFeatureEnabled(value, enabled)) {
            feature_info.second.second = enabled;
        }
    }*/
}
