# 这里应该定义一些共用的功能和项目设置

# 是否全局指定，强制所有项目的编译生成都输出到统一的目录。
# 不要强制所有项目都生成到同一目录下，不灵活。应该在需要将tagart的输出部署到同一目录的项目里采用copy的方式将其构建生成拷贝出来
if(FORCE_OUTPUT_DIRECTORY)
    # 设置executable项目输出目录，不需要带上Debug/Release，windows下cmake自己会根据build_type附加子目录，
    # 即cmake内部会有配套的CMAKE_RUNTIME_OUTPUT_DIRECTORY_<CONFIG>变量
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")

    # 设置lib的输出目录，dll项目产生的.lib文件也会在这里，对应的.dll和.pdb在CMAKE_RUNTIME_OUTPUT_DIRECTORY里
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/lib")

    # CMAKE_LIBRARY_OUTPUT_DIRECTORY是设置什么的？
    #set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dll")
    #set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/dll")
    #set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/dll")
endif(FORCE_OUTPUT_DIRECTORY)


# 基本采用CEF的cef_variables.cmake、cef_macros.cmake配置
include(common_variables)
include(common_macros)
