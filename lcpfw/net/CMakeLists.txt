

# all platform sources
RECURSE_PLATFORM_ALL_SRCS(platform_COMMON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}" true)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${platform_COMMON_SRCS})

set(target_SRCS
  ${platform_COMMON_SRCS}
  )

add_library(net STATIC ${target_SRCS})

COMMON_SET_LIBRARY_TARGET_PROPERTIES(net)

include_directories(${LCPFW_SOURCE_DIR}/my-base)
add_dependencies(net base)
target_link_libraries(net base)

install_lcpfw_core(net)