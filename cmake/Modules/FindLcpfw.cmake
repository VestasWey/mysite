
# lcpfw directory paths.
set(LCPFW_SOURCE_DIR     "${CMAKE_SOURCE_DIR}/lcpfw")

# windows/linux统一部署目录，项目的核心产出拷贝到部署目录，调试时以部署目录为工作目录，安装包封包时从部署目录选取发布文件
set(COMMON_DEPLOYMENT_DIR "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# 中间产物目录，存放诸如编译日志、.obj文件等
set(COMMON_INTERMEDIATE_DIR "${CMAKE_BINARY_DIR}/global_intermediate")
# "${CMAKE_BINARY_DIR}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/Plugins/org_test_plugins/"

# 中间产物目录中存放程序资源的目录，目录中放置的是项目生成时、编译时要引用到，然后生成、构建、编译过程中有可能会变化的文件
# 比如资源ID定义头文件、版本信息文件等。
# 在进行cmake初始配置时会从源码目录拷贝占位文件到中间目录，以便对资源文件有依赖的项目能正确通过cmake配置和生成
set(RES_DEPLOYMENT_DIR ${COMMON_INTERMEDIATE_DIR}/lcpfw)


# 将资源头文件拷贝到中间目录，
# 以便于后续的项目在cmake的时候能正确的找到磁盘上的资源ID定义.h文件
message(STATUS "copy ${LCPFW_SOURCE_DIR}/resources/lcpfw/grit/*.h to: ${RES_DEPLOYMENT_DIR}/grit")
set(GRIT_FILES 
  ${LCPFW_SOURCE_DIR}/resources/grit/generated_resources.h 
  ${LCPFW_SOURCE_DIR}/resources/grit/theme_resources.h
  )
foreach(fname ${GRIT_FILES})
    file(COPY ${fname} DESTINATION ${RES_DEPLOYMENT_DIR}/grit)
endforeach()


if(OS_WINDOWS)
    # Windows下把版本信息模板文件以正式版本信息的后缀（.rc）拷贝到中间目录，
    # 以便于后续的项目在cmake的时候能正确的找到磁盘上的.rc文件
    message(STATUS "copy ${CMAKE_SOURCE_DIR}/resources/lcpfw/version/win.rc.in to: ${RES_DEPLOYMENT_DIR}")

    set(DEST_WINRC_FILES lcpfw.rc lcpfw_main_lib.rc)
    set(SRC_WINRC_IN_FILE win.rc.in)
    set(SRC_WINRC_FILE ${LCPFW_SOURCE_DIR}/resources/lcpfw/version/${SRC_WINRC_IN_FILE})
    foreach(FILENAME ${DEST_WINRC_FILES})
        #message(STATUS "copy ${SRC_WINRC_IN_FILE} to: ${RES_DEPLOYMENT_DIR}/${FILENAME}")
        file(COPY ${SRC_WINRC_FILE} DESTINATION ${RES_DEPLOYMENT_DIR})
        file(RENAME ${RES_DEPLOYMENT_DIR}/${SRC_WINRC_IN_FILE} ${RES_DEPLOYMENT_DIR}/${FILENAME})
    endforeach()
endif()


#
# CEF
#

# CEF directory paths.
set(CEF_SOURCE_DIR                  "${CMAKE_SOURCE_DIR}/cef")
set(CEF_RESOURCE_DIR                "${CEF_SOURCE_DIR}/Resources")
set(CEF_BINARY_DIR                  "${CEF_SOURCE_DIR}/$<CONFIGURATION>")
set(CEF_BINARY_DIR_DEBUG            "${CEF_SOURCE_DIR}/Debug")
set(CEF_BINARY_DIR_RELEASE          "${CEF_SOURCE_DIR}/Release")
# 之所以把libcef_dll_wrapper二进制文件放到${CEF_SOURCE_DIR}/build/libcef_dll_wrapper目录，
# 是方便obs-browser直接Find，obs-studio\plugins\obs-browser\FindCEF.cmake就是按照这个路径找libcef_dll_wrapper库的
set(CEF_WARPPER_BINARY_DIR          "${CEF_SOURCE_DIR}/build/libcef_dll_wrapper/$<CONFIGURATION>")
set(CEF_WARPPER_BINARY_DIR_DEBUG    "${CEF_SOURCE_DIR}/build/libcef_dll_wrapper/Debug")
set(CEF_WARPPER_BINARY_DIR_RELEASE  "${CEF_SOURCE_DIR}/build/libcef_dll_wrapper/Release")

# CEF library paths.
if(OS_LINUX)
  set(CEF_LIB_DEBUG   "${CEF_BINARY_DIR_DEBUG}/libcef.so")
  set(CEF_LIB_RELEASE "${CEF_BINARY_DIR_RELEASE}/libcef.so")
  
  # List of CEF binary files.
  set(CEF_BINARY_FILES
    chrome-sandbox
    libcef.so
    libEGL.so
    libGLESv2.so
    natives_blob.bin
    snapshot_blob.bin
    v8_context_snapshot.bin
    swiftshader
    )

  # List of CEF resource files.
  set(CEF_RESOURCE_FILES
    cef.pak
    cef_100_percent.pak
    cef_200_percent.pak
    cef_extensions.pak
    devtools_resources.pak
    icudtl.dat
    locales
    )
elseif(OS_WINDOWS)
  set(CEF_LIB_DEBUG                 "${CEF_BINARY_DIR_DEBUG}/libcef.lib")
  set(CEF_LIB_RELEASE               "${CEF_BINARY_DIR_RELEASE}/libcef.lib")
  set(CEF_WARPPER_LIB_DEBUG         "${CEF_WARPPER_BINARY_DIR_DEBUG}/libcef_dll_wrapper.lib")
  set(CEF_WARPPER_LIB_RELEASE       "${CEF_WARPPER_BINARY_DIR_RELEASE}/libcef_dll_wrapper.lib")
  
  # List of CEF binary files.
  set(CEF_BINARY_FILES
    chrome_elf.dll
    d3dcompiler_47.dll
    libcef.dll
    libEGL.dll
    libGLESv2.dll
    natives_blob.bin
    snapshot_blob.bin
    v8_context_snapshot.bin
    swiftshader
    )
    
  # List of CEF resource files.
  set(CEF_RESOURCE_FILES
    cef.pak
    cef_100_percent.pak
    cef_200_percent.pak
    cef_extensions.pak
    devtools_resources.pak
    icudtl.dat
    locales
    )
elseif(OS_MACOSX)
  set(CEF_LIB_DEBUG                 "${CEF_BINARY_DIR_DEBUG}/Chromium Embedded Framework.framework")
  set(CEF_LIB_RELEASE               "${CEF_BINARY_DIR_RELEASE}/Chromium Embedded Framework.framework")
  set(CEF_WARPPER_LIB_DEBUG         "${CEF_WARPPER_BINARY_DIR_DEBUG}/libcef_dll_wrapper.a")
  set(CEF_WARPPER_LIB_RELEASE       "${CEF_WARPPER_BINARY_DIR_RELEASE}/libcef_dll_wrapper.a")
endif()

# 根据项目配置(debug/release)同一个变量自动指向正确的链接库
if(NOT OS_MACOSX)
  set(CEF_LIB
      optimized ${CEF_LIB_RELEASE}
      debug ${CEF_LIB_DEBUG})
  set(CEF_WARPPER_LIB
      optimized ${CEF_WARPPER_LIB_RELEASE}
      debug ${CEF_WARPPER_LIB_DEBUG})
else()
  #set(CEF_LIB
  #  $<$<CONFIG:Debug>:${CEF_LIB_DEBUG}>
  #  $<$<CONFIG:Release>:${CEF_LIB_RELEASE}>)
  set(CEF_LIB
    ${CEF_LIB_RELEASE})
  set(CEF_WARPPER_LIB
    $<$<CONFIG:Debug>:${CEF_WARPPER_LIB_DEBUG}>
    $<$<CONFIG:Release>:${CEF_WARPPER_LIB_RELEASE}>)
endif()

# CEF include files
macro(ADD_CEF_INCLUDE_DIRECTORIES target)
  target_include_directories(${target} PRIVATE ${CEF_SOURCE_DIR})
endmacro()


#
# thirdparty
#

set(LCPFW_THIRDPARTY_DIR     "${LCPFW_SOURCE_DIR}/third_party")
# boost
#set(LCPFW_BOOST_DIR          "${LCPFW_THIRDPARTY_DIR}/boost_lite")
# openssl
#set(LCPFW_OPENSSL_DIR        "${LCPFW_THIRDPARTY_DIR}/openssl")
# openssl
#set(LCPFW_OPENSSL_102_DIR    "${LCPFW_THIRDPARTY_DIR}/openssl-1.0.2h")


include(LcpfwHelpers)
