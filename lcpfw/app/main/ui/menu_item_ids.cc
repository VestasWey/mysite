#include "menu_item_ids.h"

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/l10n/l10n_util.h"

const std::map<MenuCommandId, int> kIdStrMap{
    {IDC_SHOW, 1},
    {IDC_ABOUT, 1},
    {IDC_EXIT, 1},
};

int GetMenuItemStringId(MenuCommandId id)
{
    auto iter = kIdStrMap.find(id);
    DCHECK(iter != kIdStrMap.end());
    if (iter != kIdStrMap.end())
    {
        return iter->second;
    }
    return -1;
}

base::string16 GetMenuItemString(MenuCommandId id)
{
    int strid = GetMenuItemStringId(id);
    if (-1 != strid)
    {
        return l10n_util::GetStringUTF16(strid);
    }
    return {};
}
