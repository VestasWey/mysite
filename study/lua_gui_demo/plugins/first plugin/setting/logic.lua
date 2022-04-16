--引用另一个lua文件的方式dofile、require
--[[以dofile方式“引用”需要得到目标文件的绝对路径，并且会执行一遍脚本，为了避免执行不必要的操作，
则目标文件里面应该仅仅只是定义变量/函数，不应有任何函数调用操作
]]--
local info = debug.getinfo(1, "S"); -- 第二个参数 "S" 表示仅返回 source,short_src等字段， 其他还可以 "n", "f", "I", "L"等 返回不同的字段信息 
local path = info.source;
path = string.sub(path, 2, -1) -- 去掉开头的"@"    
path = string.match(path, "^.*\\") -- 捕获最后一个 "/" 之前的部分 就是我们最终要的目录部分
dofile(path .. "layout.lua");  -- 虽然可以“引用”，但需要加载并执行整份脚本

--[[以require方式能真正引用文件，无需执行一遍。但根据lua的检索顺序，相应lua文件必须放在宿主程序路径下，
如果文件本身在一个多层级的目录中则整个目录必须在宿主程序目录下，并且lua文件内部需要确切知道自己与宿主
程序的完整相对路径，今后若是稍微改动一下目录结构，则必须一一修改其内部全部的lua文件代码
]]--
--require("plugins\\test plugin\\layout");   -- 直接引用lua文件需要确切知道/获取目标文件与宿主exe的相对路径，不会执行脚本


gui_proxy = require("lua_gui_proxy");
local widget = nil;

local cort = coroutine.create(function (str)

        widget:Edit_AppendText(IDC_EDIT_INPUT, string.format("cort start '%s'\r\n", str));

        local rs = coroutine.yield("yield first time");
        widget:Edit_AppendText(IDC_EDIT_INPUT, string.format("yield param = '%s'\r\n", rs));
        
        return "cort end"
    end)

local function resumecort(str)
    local state, ret = coroutine.resume(cort, str);
    widget:Edit_AppendText(IDC_EDIT_INPUT, string.format("resume state = %s, ret = '%s'\r\n", 
        state and "true" or "false", ret));
end

function main(lua)
    widget = gui_proxy:CreateWidget(lua);
    print("lua lua lua lua lua");

    --ret = widget:DoModal(lua);
    --widget = nil;
    --collectgarbage("collect");

    ret = widget:Create();
    
    return ret;
end

function OnKickIdle()
    if widget ~= nil then
        if widget:IsDlgButtonChecked(IDC_CHECK_ENABLE) == true then
            widget:SetDlgItemEnable(IDC_RADIO_FIRST, true);
            widget:SetDlgItemEnable(IDC_RADIO_SECOND, true);
        else
            widget:SetDlgItemEnable(IDC_RADIO_FIRST, false);
            widget:SetDlgItemEnable(IDC_RADIO_SECOND, false);
        end
    end
end

function OnCommand(id, code)
    if widget ~= nil then
        if id == IDC_BUTTON_OK then
                    
            local cb = widget:IsDlgButtonChecked(IDC_CHECK_ENABLE);
            local r1 = widget:IsDlgButtonChecked(IDC_RADIO_FIRST);
            local r2 = widget:IsDlgButtonChecked(IDC_RADIO_SECOND);
            local edit_text = widget:Edit_GetText(IDC_EDIT_INPUT);

            --gui_proxy:MessageBox(string.format("checkbox=%s, radiofirst=%s, radiosecond=%s, edittext='%s'", 
            --    cb and "true" or "false", 
            --    r1 and "true" or "false", 
            --    r2 and "true" or "false", 
            --    edit_text));

            resumecort("resume");

        elseif id == IDC_BUTTON_CANCEL then
            --widget:EndDialog(2);
            widget:DestroyWindow();
        elseif id == IDCANCEL then
            gui_proxy:MessageBox("cancel button click");
        else
            --gui_proxy:MessageBox(string.format("lua OnCommand(%d, %d)", id, code));
        end
    end
end

function OnMouseMove(x, y)
    if widget ~= nil then
        widget:SetDlgItemText(IDC_STATIC_STATUE, string.format("%d, %d", x, y));
    end
end

function OnMouseExit()
    if widget ~= nil then
        widget:SetDlgItemText(IDC_STATIC_STATUE, "mouse leave");
    end
end

