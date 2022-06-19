

# all platform sources
RECURSE_PLATFORM_ALL_SRCS(platform_COMMON_SRCS "${LCPFW_SOURCE_DIR}/my-base" true)
source_group(TREE "${LCPFW_SOURCE_DIR}/my-base" FILES ${platform_COMMON_SRCS})

set(target_SRCS
  ${platform_COMMON_SRCS}
  )

add_library(base STATIC ${target_SRCS})

COMMON_SET_LIBRARY_TARGET_PROPERTIES(base)

target_link_directories(base PRIVATE "${LCPFW_THIRDPARTY_DIR}/libcurl/lib")
target_link_libraries(base Winmm.lib)

install_lcpfw_core(base)