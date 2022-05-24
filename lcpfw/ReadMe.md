GN工具链以及build配置基本都来源于chromium源码，版本信息为：
    
    CEF version is:
    4430, 2021/5/21 3:17:13,
    commit:3c44b04c4ebefee6e49bf1fed8ab1bff6f737da1

    Chromium version is:
    90.0.4430.93, 2021/4/24 8:08:45,
    commit:f46f037392518e6b7665d3f3f626d6d80cad1c2d
chromium源码是从CEF-4430版本依赖的chromium-90.0.4430.93版本的源码中直接截取的，目录包括：
    build
        config
        toolchain
        win

代码截取、编译方式
    base
    看base目录下的 BUILD.gn文件，查看库的deps都依赖了哪些库，然后把这些直接肉眼就能看到的依赖库都拷贝过来；
    在这个过程中，顺带手的把 testonly=true 的目标、unittests 的目标、xxx_tests的目标 都注释掉，不需要构建这种测试目标；
    base库在msvc下的编译改造相对没那么麻烦，如果只是使用base库，那么禁用base库的trace跟踪以便解除对third_party/perfetto的依赖，不生成单元测试目标，减少代码依赖，
    然后主要修改一些模板实例化的问题即可；如果后续还会使用其他的库（比如net库，其仍然依赖perfetto，且范围很广，无法轻松去除依赖）则无需禁用，只需要对perfetto库的些许代码进行一些msvc兼容性修改即可。
    
    
构建
    Windows
    gn gen out/32bit/Debug --args="target_cpu=\"x86\" is_debug=true" --ide=vs2019 --sln=lcpfw --winsdk=10.0.19041.0
    autoninja -C out/32bit/Debug app