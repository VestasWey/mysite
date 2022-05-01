cmake_minimum_required(VERSION 3.10)

# modify [
if(WIN32)
# windows need winsdk 10.0.19041.0, obs recommend.
set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
endif(WIN32)
# ]

# asm
#enable_language(ASM_NASM)
 
# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

# Project name.
project(vestas_site)

# cmake path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/cmake/Modules")

# 必要的包
find_package(Lcpfw REQUIRED)

# 引入公共配置，全名
include(common_config)

# 从当前版本配置文件读取程序版本号设置到LCPFW_VERSION，version号形如：1.0.0
lcpfw_read_profile("${CMAKE_SOURCE_DIR}/lcpfw/resources/version/VERSION" "LCPFW")
if(DEFINED ENV{LCPFW_MAJOR} AND DEFINED ENV{LCPFW_MINOR} AND DEFINED ENV{LCPFW_BUILD})
  set(LCPFW_VERSION $ENV{LCPFW_MAJOR}.$ENV{LCPFW_MINOR}.$ENV{LCPFW_BUILD})
  message(STATUS "lcpfw Version: ${LCPFW_VERSION}")
else()
  message(FATAL_ERROR "lcpfw/resources/version/VERSION not exists or data invalid, VERSION must at least contain thess fields:
    MAJOR=1
    MINOR=0
    BUILD=0
    ")
endif()

# 因为我们不会去改动CEF的源码，所以就不需要每次构建都重新编译cef-warpper,只需要引用CEF的二进制即可，
# 这里就不生成CEF的项目了，当需要重新编译libcef_dll_wrapper时可以在这里放开这个注释，以便直接在主项目里
# 构建libcef_dll_wrapper，也可以单独到cef目录去进行cmake构建，在cef自己的工程项目里去构建libcef_dll_wrapper，
# 构建完把libcef_dll_wrapper的二进制文件拷贝到cef\build\libcef_dll_wrapper\[Debug/Release]放置即可
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cef")
  #add_subdirectory(cef)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cef_proxy")
  #add_subdirectory(cef_proxy)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lcpfw")
  add_subdirectory(lcpfw)
  #add_subdirectory(lcpfw/main)
  #add_subdirectory(lcpfw/secret)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
  #add_subdirectory(resources/lcpfw)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lcpfw_browser")
  #add_subdirectory(lcpfw_browser)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
  #add_subdirectory(third_party/lua)
endif()

#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
#  add_subdirectory(tests/ffapi-study)
#  add_subdirectory(tests/exec-test)
#  add_subdirectory(tests/module-study)
#  add_subdirectory(tests/static-study)
#endif()

# Display configuration settings.
#PRINT_COMMON_CONFIG()
