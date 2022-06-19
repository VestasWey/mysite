#pragma once


enum MenuCommandId
{
    IDC_SHOW,
    IDC_ABOUT,
    IDC_EXIT,
};

int GetMenuItemStringId(MenuCommandId);
base::string16 GetMenuItemString(MenuCommandId);