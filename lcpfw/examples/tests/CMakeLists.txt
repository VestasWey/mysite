


# all platform sources
RECURSE_PLATFORM_ALL_SRCS(tests_SRCS "${CMAKE_CURRENT_SOURCE_DIR}" true)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${tests_SRCS})

add_executable(tests_console ${tests_SRCS})

COMMON_SET_EXECUTABLE_TARGET_PROPERTIES(tests_console)

include_directories(${LCPFW_SOURCE_DIR}/my-base)
add_dependencies(tests_console ipc net base)
target_link_libraries(tests_console ipc net base)

# 测试用的项目不需要太严格的警告级别,直接移除公共编译参数中的“视警告为错误”
target_compile_options(tests_console PRIVATE /WX-)

# 没完全写完、不保证能正确编译的文件不参加编译，虽然不参加编译，但是在IDE还是能照常看到（在VS里没有减号标识，不完美）
set(ignore_SRCS 
    chromium_post_task_example.cpp
    post_task_example.cpp
    )      
set_source_files_properties(${ignore_SRCS} PROPERTIES HEADER_FILE_ONLY ON)