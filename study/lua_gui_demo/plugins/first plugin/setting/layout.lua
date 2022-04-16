--按照gui提供的ID初始值进行设置
IDC_STATIC_TITLE = 1001;
IDC_CHECK_ENABLE = 1002;
IDC_RADIO_FIRST = 1003;
IDC_RADIO_SECOND = 1004;
IDC_STATIC_STATUE = 1005;
IDC_EDIT_INPUT = 1006;
IDC_BUTTON_OK = 1007;
IDC_BUTTON_CANCEL = 1008;

local label_cx = 200;
local label_cy = 30;

title = "test plugin settings";
icon = "";
width = 480;
height = 500;
layout = {
    { type = "static", id = IDC_STATIC_TITLE, text = "this is a static ctrl", x = 10, y = 10, width = label_cx, height = label_cy },
    { type = "check", id = IDC_CHECK_ENABLE, text = "this is a checkbox ctrl", x = 10, y = 50, width = label_cx, height = label_cy },
    { type = "radio", id = IDC_RADIO_FIRST, text = "this is a radio ctrl 0", x = 30, y = 90, width = label_cx, height = label_cy },
    { type = "radio", id = IDC_RADIO_SECOND, text = "this is a radio ctrl 1", x = 30, y = 130, width = label_cx, height = label_cy },
    { type = "edit", id = IDC_EDIT_INPUT, text = "this is a edit ctrl", x = 10, y = 170, width = 450, height = 100 },
    { type = "button", id = IDC_BUTTON_OK, text = "ok", x = 100, y = 280, width = 100, height = 40 },
    { type = "button", id = IDC_BUTTON_CANCEL, text = "cancel", x = 220, y = 280, width = 100, height = 40 },
    { type = "static", id = IDC_STATIC_STATUE, text = "", x = 10, y = 330, width = 460, height = 40 }
};