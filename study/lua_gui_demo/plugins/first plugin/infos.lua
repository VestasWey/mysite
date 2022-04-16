
IDM_OPEN = 2001;
IDM_END = 2002;

name = "test plugin(&T)";

menus = {
    { id = IDM_OPEN, text = "Open(&O)", enable = true },
    {},
    { id = IDM_END, text = "disable item", enable = false }
};

function OnMenuSelected(id)
    if id == IDM_OPEN then
        return "setting";
    elseif id == IDM_END then
    end

    print("not impl item = " .. id);
    return "";
end