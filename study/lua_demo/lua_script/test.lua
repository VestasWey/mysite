local lp = require("lua_proxy");

local info = debug.getinfo(1, "S") -- 第二个参数 "S" 表示仅返回 source,short_src等字段， 其他还可以 "n", "f", "I", "L"等 返回不同的字段信息  
  
for k,v in pairs(info) do  
        print(k, ":", v)  
end  
  
local path = info.source
print(path);
path = string.sub(path, 2, -1) -- 去掉开头的"@"    
path = string.match(path, "^.*/") -- 捕获最后一个 "/" 之前的部分 就是我们最终要的目录部分 
print(path);

print("---------------lua begin---------------");

print(os.date());
print("lua average " .. lp:average(1,2,3,4, "asd"));
print("lua print_str " .. lp:print_str("lua_proxy"));

print("lua print_str table param " .. lp:print_str{name="asd"});

--local proxy_obj = lp:CreateCLuaProxy();
--print(proxy_obj);
--proxy_obj:SayHello();
--proxy_obj = nil;
--
--local proxy_obj1 = lp:CreateCLuaProxy();
--print(proxy_obj1);
--proxy_obj1.SayHello(proxy_obj1);
--proxy_obj1 = nil;

proxy_raw_obj = lp:NewCLuaProxy();

g_member = "study";
--table的key是混合类型值
g_table = 
{ 
    name = "usopp", 
    mail = "xxx@qq.com", 
    [1] = 1,
    [3] = 3,
    [10101] = {index=10101, desc="机枪", profession={1,3,5}, slot=1, consumable=nil},
    [1.5] = "key=1.5"
};
--table的key是连续的整形值，即通常意义的数组
g_vector = { 1, 1.5, "string" };
--table的key是混合类型值的lua将此table当成哈希表，c++将此当成数组，索引为10101，10102，20101
weapons =
{
    [10101] = {index=10101, desc="机枪", profession={1,3,5}, slot=1, consumable=nil},
    [10102] = {index=10102, desc="能量枪", profession={1}, slot=2, consumable=nil},
    [20101] = {index=20101, desc="榴弹", profession={2}, slot=3, consumable=true},
};

g_table.ext = "ext";
print(g_table.name);
print(g_table["mail"]);
print(g_table[10101]);
print(g_table[1.5]);

g_vector[4] = "asd";
print(g_vector[4]);

function add (a,b)    
    print("add p_a is ", a);
    print("add p_b is ", b);
    return a+b;
end

function attch_proxy(cppobj)
    local obj = lp.AttchCLuaProxy(cppobj);
    obj:SayHello();
    print(obj);
    obj = nil;
    collectgarbage("collect");--proxy_obj = nil之后不会立即gc，调用collectgarbage立即gc
end

local sum = add(11, 22);
print("add sum is ", sum);

if lua_call_cpp_fn then
    local ret0, ret1 = lua_call_cpp_fn(1, 2, 3);
    print("lua_call_cpp_fn sum is ", ret0, ret1);
end

--遍历哈希表
if g_table then
    print("g_table members begin");
    for k,v in pairs(g_table) do 
        print("k = ",k," v = ",v)
    end
    print("g_table members end");
end

--遍历数组
if g_vector then
    print("g_vector members");
    for i = 1, #g_vector do
        print("v[" , i, "]", g_vector[i]);
    end

    for i,v in ipairs(g_vector) do
        print(i, v);
    end
    print("g_vector members end");
end

if weapons then
    print("weapons members begin");
    for k,v in pairs(weapons) do 
        print("k = ",k," v = ",v)
    end
    print("weapons members end");
end

function check_cpp_global()
    --数组遍历
    if cpp_vector then
        print("cpp_vector members begin");
        for i = 1, #cpp_vector do
            print("v[" , i, "]", cpp_vector[i]);
        end
        print("cpp_vector members end");
    end

    --map遍历
    if cpp_map then
        print("cpp_map members begin");
        for k,v in pairs(cpp_map) do 
            print("k = ",k," v = ",v)
        end
        print("cpp_map members end");
    end
end

--collectgarbage("collect");--proxy_obj = nil之后不会立即gc，调用collectgarbage立即gc

print("---------------lua end---------------");
