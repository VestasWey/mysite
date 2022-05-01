

add_subdirectory(tests)

set(exe_SRCS
    examples_main.cpp
)
add_executable(views_examples_exe WIN32 ${exe_SRCS})

COMMON_SET_EXECUTABLE_TARGET_PROPERTIES(views_examples_exe)
install_lcpfw_core(views_examples_exe)