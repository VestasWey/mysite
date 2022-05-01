# Copy from cef/cmake/cef_macros.cmake

#
# Shared macros.
#

# Print the current CEF configuration.
macro(PRINT_COMMON_CONFIG)
  message(STATUS "*** COMMON CONFIGURATION SETTINGS BEGIN ***")
  message(STATUS "Generator:                    ${CMAKE_GENERATOR}")
  message(STATUS "Platform:                     ${CMAKE_SYSTEM_NAME}")
  message(STATUS "Project architecture:         ${PROJECT_ARCH}")

  if(GEN_NINJA OR GEN_MAKEFILES)
    message(STATUS "Build type:                   ${CMAKE_BUILD_TYPE}")
  endif()

  if(OS_MACOSX)
    message(STATUS "Base SDK:                     ${CMAKE_OSX_SYSROOT}")
    message(STATUS "Target SDK:                   ${COMMON_TARGET_SDK}")
  endif()

  if(OS_WINDOWS)
    message(STATUS "Visual Studio ATL support:    ${USE_ATL}")
    message(STATUS "Windows SDK version:          ${CMAKE_SYSTEM_VERSION}")
  endif()

  message(STATUS "Standard libraries:           ${COMMON_STANDARD_LIBS}")

  message(STATUS "Compile defines:              ${COMMON_COMPILER_DEFINES}")
  message(STATUS "Compile defines (Debug):      ${COMMON_COMPILER_DEFINES_DEBUG}")
  message(STATUS "Compile defines (Release):    ${COMMON_COMPILER_DEFINES_RELEASE}")
  message(STATUS "C compile flags:              ${COMMON_COMPILER_FLAGS} ${CEF_C_COMPILER_FLAGS}")
  message(STATUS "C compile flags (Debug):      ${COMMON_COMPILER_FLAGS_DEBUG} ${CEF_C_COMPILER_FLAGS_DEBUG}")
  message(STATUS "C compile flags (Release):    ${COMMON_COMPILER_FLAGS_RELEASE} ${CEF_C_COMPILER_FLAGS_RELEASE}")
  message(STATUS "C++ compile flags:            ${COMMON_COMPILER_FLAGS} ${COMMON_CXX_COMPILER_FLAGS}")
  message(STATUS "C++ compile flags (Debug):    ${COMMON_COMPILER_FLAGS_DEBUG} ${COMMON_CXX_COMPILER_FLAGS_DEBUG}")
  message(STATUS "C++ compile flags (Release):  ${COMMON_COMPILER_FLAGS_RELEASE} ${COMMON_CXX_COMPILER_FLAGS_RELEASE}")
  message(STATUS "Exe link flags:               ${COMMON_LINKER_FLAGS} ${COMMON_EXE_LINKER_FLAGS}")
  message(STATUS "Exe link flags (Debug):       ${COMMON_LINKER_FLAGS_DEBUG} ${COMMON_EXE_LINKER_FLAGS_DEBUG}")
  message(STATUS "Exe link flags (Release):     ${COMMON_LINKER_FLAGS_RELEASE} ${COMMON_EXE_LINKER_FLAGS_RELEASE}")
  message(STATUS "Shared link flags:            ${COMMON_LINKER_FLAGS} ${COMMON_SHARED_LINKER_FLAGS}")
  message(STATUS "Shared link flags (Debug):    ${COMMON_LINKER_FLAGS_DEBUG} ${COMMON_SHARED_LINKER_FLAGS_DEBUG}")
  message(STATUS "Shared link flags (Release):  ${COMMON_LINKER_FLAGS_RELEASE} ${COMMON_SHARED_LINKER_FLAGS_RELEASE}")

  message(STATUS "*** COMMON CONFIGURATION SETTINGS END ***")
endmacro()

# Append platform specific sources to a list of sources.
macro(APPEND_PLATFORM_SOURCES name_of_list)
  if(OS_LINUX AND ${name_of_list}_LINUX)
    list(APPEND ${name_of_list} ${${name_of_list}_LINUX})
  endif()
  if(OS_POSIX AND ${name_of_list}_POSIX)
    list(APPEND ${name_of_list} ${${name_of_list}_POSIX})
  endif()
  if(OS_WINDOWS AND ${name_of_list}_WINDOWS)
    list(APPEND ${name_of_list} ${${name_of_list}_WINDOWS})
  endif()
  if(OS_MACOSX AND ${name_of_list}_MACOSX)
    list(APPEND ${name_of_list} ${${name_of_list}_MACOSX})
  endif()
endmacro()


# Determine the target output directory based on platform and generator.
# 确定各模块自己的构建输出目录（cmake生成目录中模块目录下的Debug/Release目录），
# 返回：
#   COMMON_TARGET_OUT_DIR，模块构建的输出目录
macro(DETERMINE_COMMON_TARGET_OUT_DIR)
  if(GEN_NINJA OR GEN_MAKEFILES)
    # By default Ninja and Make builds don't create a subdirectory named after
    # the configuration.
    set(COMMON_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

    # Output binaries (executables, libraries) to the correct directory.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${COMMON_TARGET_OUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${COMMON_TARGET_OUT_DIR})
  else()
    set(COMMON_TARGET_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
  endif()
endmacro()


# Copy a list of files from one directory to another. Relative files paths are maintained.
# The path component of the source |file_list| will be removed.
macro(COPY_FILES target file_list source_dir target_dir)
  foreach(FILENAME ${file_list})
    set(source_file ${source_dir}/${FILENAME})
    get_filename_component(target_name ${FILENAME} NAME)
    set(target_file ${target_dir}/${target_name})

    string(FIND ${source_file} "$<CONFIGURATION>" _pos)
    if(NOT ${_pos} EQUAL -1)
      # Must test with an actual configuration directory.
      string(REPLACE "$<CONFIGURATION>" "Release" existing_source_file ${source_file})
      if(NOT EXISTS ${existing_source_file})
        string(REPLACE "$<CONFIGURATION>" "Debug" existing_source_file ${source_file})
      endif()
    else()
      set(existing_source_file ${source_file})
    endif()

    if(IS_DIRECTORY ${existing_source_file})
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_file}" "${target_file}"
        VERBATIM
        )
    else()
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source_file}" "${target_file}"
        VERBATIM
        )
    endif()
  endforeach()
endmacro()


# 从source_dir目录里将文件列表file_list中的文件拷贝到target_dir目录
macro(COPY_SOURCE_DIR_FILES target file_list source_dir target_dir)
  foreach(FILENAME ${file_list})
    set(source_file ${source_dir}/${FILENAME})
    get_filename_component(target_name ${FILENAME} NAME)
    set(target_file ${target_dir}/${target_name})

    if(IS_DIRECTORY ${source_file})
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND echo "deploying directory \"${source_file}\" to \"${target_file}\""
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_file}" "${target_file}"
        VERBATIM
        )
    else()
      add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND echo "deploying file \"${source_file}\" to \"${target_file}\""
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source_file}" "${target_file}"
        VERBATIM
        )
    endif()
  endforeach()
endmacro()


# 从指定目录中获取适用于当前平台的全部源文件（所有平台无关的源文件+仅当前平台相关的源文件）
# 多数情况下只要实际的文件系统中源文件的放置规则统一（文件名带平台标识后缀或者置于平台标识目录中的文件），
# 那么用这个宏就能一口气把当前平台所依赖的全部源文件获取完 
macro(RECURSE_PLATFORM_ALL_SRCS result_list source_dir is_recurse)
  # file(GLOB)通配符表达式无法针对词组（只能单个字符）进行文件过滤，
  # 所以需要先用file(GLOB)把指定目录下的源文件都枚举出来，
  # 然后再用list(FILTER EXCLUDE REGEX)把平台相关的源码去掉，
  # 从而得到全平台使用的源文件
  if(${is_recurse})
    file(GLOB_RECURSE ${result_list}  
      LIST_DIRECTORIES false
      #RELATIVE ${source_dir}
      "${source_dir}/*.*"
      )
  else()
    file(GLOB ${result_list}  
      LIST_DIRECTORIES false
      #RELATIVE ${source_dir}
      "${source_dir}/*.*"
      )
  endif()
  
  # 根据当前平台，确定过滤标识
  if(OS_WINDOWS)
    set(PLATFORM_FILE_FILTER_FLAG "_mac|_linux|_posix")
    set(PLATFORM_DIR_FILTER_FLAG "mac|linux|posix")
  elseif(OS_MACOSX)
    set(PLATFORM_FILE_FILTER_FLAG "_win|_linux")
    set(PLATFORM_DIR_FILTER_FLAG "win|linux")
  elseif(OS_LINUX)
    set(PLATFORM_FILE_FILTER_FLAG "_linux|_posix")
    set(PLATFORM_DIR_FILTER_FLAG "linux|posix")
  endif()

  #message(STATUS "result_list: ${result_list}")
  # 去掉以特殊后缀标识的平台相关的源文件
  list(FILTER ${result_list} EXCLUDE REGEX
    "(.?)(${PLATFORM_FILE_FILTER_FLAG})\\.(c|cc|cpp|h|mm|asm)"
  )
  # 去掉置于平台相关目录中的源文件
  # 之所以这里采用的方式是先获取全量文件列表，再过滤平台无关文件，而不是直接获取平台相关文件，是由于这样不好做。后续可以研究一波是否可做
  list(FILTER ${result_list} EXCLUDE REGEX
    "(^|/)(${PLATFORM_DIR_FILTER_FLAG})/(.?)"
  )
endmacro()

# 仅获取指定目录的直属文件列表，不继续向下递归
macro(GET_PLATFORM_ALL_SRCS result_list source_dir)
  RECURSE_PLATFORM_ALL_SRCS(${result_list} ${source_dir} false)
endmacro()


# 仅获取指定目录中"专用"于当前平台的源文件列表（文件名带平台标识后缀或者置于平台标识目录中的文件）
macro(RECURSE_PLATFORM_SPECIAL_SRCS result_list source_dir is_recurse)
  # 根据当前平台，确定过滤标识
  if(OS_WINDOWS)
    set(PLATFORM_FILTER_FLAGS "win")
  elseif(OS_MACOSX)
    set(PLATFORM_FILTER_FLAGS "mac" "posix")
  elseif(OS_LINUX)
    set(PLATFORM_FILTER_FLAGS "linux" "posix")
  endif()
  
  foreach(PLATFORM_FILTER_FLAG ${PLATFORM_FILTER_FLAGS})
    if(${is_recurse})
      file(GLOB_RECURSE file_list
        LIST_DIRECTORIES false
        #RELATIVE ${source_dir}

        # 平台后缀
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.c"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.cc"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.cpp"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.h"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.mm"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.asm"

        # 平台目录
        "${source_dir}/*${PLATFORM_FILTER_FLAG}/*"
        )
      #message(STATUS "file_list: ${file_list}")
      list(APPEND ${result_list} ${file_list})
    else()
      file(GLOB file_list
        LIST_DIRECTORIES false
        #RELATIVE ${source_dir}

        # 平台后缀
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.c"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.cc"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.cpp"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.h"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.mm"
        "${source_dir}/*_${PLATFORM_FILTER_FLAG}.asm"

        # 平台目录
        "${source_dir}/*${PLATFORM_FILTER_FLAG}/*"
        )
      #message(STATUS "file_list: ${file_list}")
      list(APPEND ${result_list} ${file_list})
    endif()
  endforeach()
endmacro()

# 仅获取指定目录中"专用"于当前平台的直属源文件列表，不继续向下递归
macro(GET_PLATFORM_SPECIAL_SRCS result_list source_dir)
  RECURSE_PLATFORM_SPECIAL_SRCS(${result_list} ${source_dir} false)
endmacro()


# 递归遍历指定的source_dir目录，获取指定后缀的所有文件列表，
# 取得文件相对于source_dir的相对路径，将该路径拼接到指定的目标目录路径，将目标路径列表返回
macro(RECURSE_FILES_FROM_SRCDIR result_list source_dir regex_file_ext is_recurse)
  if(${is_recurse})
    file(GLOB_RECURSE source_files 
      LIST_DIRECTORIES false
      "${source_dir}/*.*"
      )
  else()
    file(GLOB source_files 
      LIST_DIRECTORIES false
      "${source_dir}/*.*"
      )
  endif()

  # list filter 的方式来进行过滤没试出来怎么写这个正则才有用，暂且逐个匹配吧
  # list sub-command FILTER does not recognize operator compression_utils.lib
  #list(FILTER ${source_files} INCLUDE REGEX
  #  "(.?)\\.${regex_file_ext}$"
  #)

  # 逐个文件匹配后缀过滤条件
  #message(STATUS "source_files: ${source_files}")
  foreach(source_file ${source_files})
    string(REGEX MATCH "(.*)\\.${regex_file_ext}" rm ${source_file})
    #message(STATUS "${source_file}  REGEX: ${rm}")
    if(rm)
      list(APPEND ${result_list}
        ${source_file}
        )
    endif()
  endforeach()
endmacro()

# 将指定后缀的文件从source_dir递归拷贝到dst_dir，将目标目录的全部文件列表返回
macro(RECURSE_FILES_FROM_SRCDIR_TO_DSTDIR result_list source_dir dst_dir regex_file_ext is_recurse)
  if(${is_recurse})
    file(GLOB_RECURSE source_files 
      LIST_DIRECTORIES false
      RELATIVE ${source_dir}
      "${source_dir}/*.*"
      )
  else()
    file(GLOB source_files 
      LIST_DIRECTORIES false
      RELATIVE ${source_dir}
      "${source_dir}/*.*"
      )
  endif()

  # list filter 的方式来进行过滤没试出来怎么写这个正则才有用，暂且逐个匹配吧
  # list sub-command FILTER does not recognize operator compression_utils.lib
  #list(FILTER ${source_files} INCLUDE REGEX
  #  "(.?)\\.${regex_file_ext}$"
  #)

  # 逐个文件匹配后缀过滤条件
  #message(STATUS "source_files: ${source_files}")
  foreach(source_file ${source_files})
    string(REGEX MATCH "(.*)\\.${regex_file_ext}" rm ${source_file})
    #message(STATUS "${source_file}  REGEX: ${rm}")
    if(rm)
      list(APPEND ${result_list}
        ${dst_dir}/${source_file}
        )
    endif()
  endforeach()
endmacro()


#
# Linux macros.
#

if(OS_LINUX)

  # Use pkg-config to find Linux libraries and update compiler/linker variables.
  macro(COMMON_FIND_LINUX_LIBRARIES libraries)
    # Read pkg-config info into variables.
    execute_process(COMMAND pkg-config --cflags ${libraries} OUTPUT_VARIABLE FLL_CFLAGS)
    execute_process(COMMAND pkg-config --libs-only-L --libs-only-other ${libraries} OUTPUT_VARIABLE FLL_LDFLAGS)
    execute_process(COMMAND pkg-config --libs-only-l ${libraries} OUTPUT_VARIABLE FLL_LIBS)

    # Strip leading and trailing whitepspace.
    STRING(STRIP "${FLL_CFLAGS}"  FLL_CFLAGS)
    STRING(STRIP "${FLL_LDFLAGS}" FLL_LDFLAGS)
    STRING(STRIP "${FLL_LIBS}"    FLL_LIBS)

    # Convert to a list.
    separate_arguments(FLL_CFLAGS)
    separate_arguments(FLL_LDFLAGS)
    separate_arguments(FLL_LIBS)

    # Update build variables.
    list(APPEND COMMON_C_COMPILER_FLAGS    ${FLL_CFLAGS})
    list(APPEND COMMON_CXX_COMPILER_FLAGS  ${FLL_CFLAGS})
    list(APPEND COMMON_EXE_LINKER_FLAGS    ${FLL_LDFLAGS})
    list(APPEND COMMON_SHARED_LINKER_FLAGS ${FLL_LDFLAGS})
    list(APPEND COMMON_STANDARD_LIBS       ${FLL_LIBS})
  endmacro()

  # Set SUID permissions on the specified executable.
  macro(SET_LINUX_SUID_PERMISSIONS target executable)
    add_custom_command(
      TARGET ${target}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo ""
      COMMAND ${CMAKE_COMMAND} -E echo "*** Run the following command manually to set SUID permissions ***"
      COMMAND ${CMAKE_COMMAND} -E echo "EXE=\"${executable}\" && sudo -- chown root:root $EXE && sudo -- chmod 4755 $EXE"
      COMMAND ${CMAKE_COMMAND} -E echo ""
      VERBATIM
      )
  endmacro()

endif(OS_LINUX)


#
# Mac OS X macros.
#

if(OS_MACOSX)

  # Manually process and copy over resource files.
  # resource_list 文件列表，完整相对路径
  # prefix_list 相对目录列表
  # source_dir 源文件目录，即resource_list/prefix_list中的相对路径的原点路径
  # app_path .app的绝对路径
  macro(COPY_MACOSX_RESOURCES resource_list prefix_list target source_dir app_path)
    foreach(FILENAME ${resource_list})
      # Remove one or more prefixes from the source paths.
      set(TARGET_FILENAME "${FILENAME}")
      foreach(PREFIX ${prefix_list})
        string(REGEX REPLACE "^.*${PREFIX}" "" TARGET_FILENAME ${TARGET_FILENAME})
      endforeach()

      # Determine the absolute source and target paths.
      set(TARGET_PATH "${app_path}/Contents/Resources/${TARGET_FILENAME}")
      if(IS_ABSOLUTE ${FILENAME})
        set(SOURCE_PATH ${FILENAME})
      else()
        set(SOURCE_PATH "${source_dir}/${FILENAME}")
      endif()

      if(${FILENAME} MATCHES ".xib$")
        # Change the target file extension.
        string(REGEX REPLACE ".xib$" ".nib" TARGET_PATH ${TARGET_PATH})

        get_filename_component(TARGET_DIRECTORY ${TARGET_PATH} PATH)
        add_custom_command(
          TARGET ${target}
          POST_BUILD
          # Create the target directory.
          COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_DIRECTORY}"
          # Compile the XIB file to a NIB.
          COMMAND /usr/bin/ibtool --output-format binary1 --compile "${TARGET_PATH}" "${SOURCE_PATH}"
          VERBATIM
          )
      elseif(NOT ${TARGET_FILENAME} STREQUAL "Info.plist")
        # Copy the file as-is.
        add_custom_command(
          TARGET ${target}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE_PATH}" "${TARGET_PATH}"
          VERBATIM
          )
      endif()
    endforeach()
  endmacro()

endif(OS_MACOSX)


#
# Windows macros.
#

if(OS_WINDOWS)

  # deprecate，使用LcpfwHelpers的install_lcpfw_core/install_lcpfw_plugin更佳。
  # windows下根据目标名将目标产出的pdb、dll、exe、lib从目标构建目录拷贝到部署目录，
  # 在调用此宏之前必须先调用DETERMINE_COMMON_TARGET_OUT_DIR宏，因为这里需要用到变量COMMON_TARGET_OUT_DIR
  macro(COPY_TARGET_BUILD_FILES_TO_DEPLOYMENT_DIR target)
    DETERMINE_COMMON_TARGET_OUT_DIR()
    get_target_property(target_type ${target} TYPE)
    if (target_type STREQUAL "EXECUTABLE")
      # Process executable target
      list(APPEND DEPLOYMENT_RUNTIME_FILES
        ${target}.exe
        ${target}.pdb
      )
    elseif(target_type STREQUAL "SHARED_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY")
      list(APPEND DEPLOYMENT_RUNTIME_FILES
        ${target}.dll
        ${target}.pdb
      )
      list(APPEND DEPLOYMENT_LIB_FILES
        ${target}.lib
      )      
    elseif(target_type STREQUAL "STATIC_LIBRARY")
      list(APPEND DEPLOYMENT_LIB_FILES
        ${target}.lib
      )
    endif ()
      
    if(DEFINED DEPLOYMENT_RUNTIME_FILES)
      COPY_SOURCE_DIR_FILES("${target}" "${DEPLOYMENT_RUNTIME_FILES}" "${COMMON_TARGET_OUT_DIR}" "${COMMON_DEPLOYMENT_DIR}")
    endif()
    if(DEFINED DEPLOYMENT_LIB_FILES)
      COPY_SOURCE_DIR_FILES("${target}" "${DEPLOYMENT_LIB_FILES}" "${COMMON_TARGET_OUT_DIR}" "${COMMON_DEPLOYMENT_DIR}/lib")
    endif()
  endmacro()


  # Add CEF manifest files to an executable target.
  macro(ADD_WINDOWS_CEF_MANIFEST target extension)
    add_custom_command(
      TARGET ${target}
      POST_BUILD
      COMMAND "mt.exe" -nologo
              -manifest \"${CEF_SOURCE_DIR}/tests/cefclient/resources/win/cefclient.exe.manifest\" \"${CEF_SOURCE_DIR}/tests/cefclient/resources/win/compatibility.manifest\"
              -outputresource:"${COMMON_TARGET_OUT_DIR}/${target}.${extension}"\;\#1
      COMMENT "Adding CEF manifest..."
      )
  endmacro()


  # 使用预编译头文件，并生成pch预编译文件，参数如下：
  #   USE_MSVC_PCH        ：宏名字
  #   target          ：项目名称
  #   PCH_HEADER_FILE     ：预编译头文件名称（stdafx.h）
  #   PCH_SOURCE_FILE     ：预编译源文件名称（stdafx.cpp）
  macro(USE_MSVC_PCH target PCH_HEADER_FILE PCH_SOURCE_FILE)
    if(MSVC)
      # 获取预编译头文件的文件名，通常是stdafx
      get_filename_component(PCH_NAME ${PCH_HEADER_FILE} NAME_WE)
    
      # 生成预编译文件的路径
      if(CMAKE_CONFIGURATION_TYPES)
        # 如果有配置选项（Debug/Release），路径添加以及配置选项
        set(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH/${CMAKE_CFG_INTDIR}")
      else(CMAKE_CONFIGURATION_TYPES)
        set(PCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/PCH")
      endif(CMAKE_CONFIGURATION_TYPES)
  
      # 创建预编译文件的路径
      #file(make_directory "${PCH_DIR}")
  
      # 设置项目属性，使用预编译头文件
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS 
        "/Yu${PCH_HEADER_FILE} /FI${PCH_HEADER_FILE} /Fp${PCH_DIR}/${PCH_NAME}.pch")
  
      # 预编译源文件（stdafx.cpp）设置属性，创建预编译文件
      set_source_files_properties(${PCH_SOURCE_FILE} PROPERTIES 
        COMPILE_FLAGS "/Yc${PCH_HEADER_FILE}")
    
      # 把预编译文件寄到清除列表
      set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES ${PCH_DIR}/${PCH_NAME}.pch)
    endif(MSVC)
  endmacro()


  # 用MSVC工具链的ml.exe对asm汇编文件进行编译
  macro(COMPILE_NASM_ASM_FILES target NASM_ASM_FILES OUTPUT_FILES)
    if(MSVC)
      # 获取当前使用的VC工具链目录
      get_filename_component(VC_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    
      foreach(FILE ${NASM_ASM_FILES})
        get_filename_component(FILE_NAME ${FILE} NAME)
        get_filename_component(BASE_NAME ${FILE} NAME_WE)
        
        set(OUTPUT_TARGET ${CMAKE_CURRENT_BINARY_DIR}/${target}.dir/${BASE_NAME}.obj)
        set(COMPILE_TARGET ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)${BASE_NAME}.obj)
        add_custom_command(
          OUTPUT ${OUTPUT_TARGET}
          COMMAND ${VC_DIR}/ml.exe /safeseh /nologo /c /Fo ${COMPILE_TARGET} ${FILE}
          # OUTPUT用不了生成器表达式，COMMAND可以，那就只能在COMMAND里生成后拷贝到OUTPUT，以便于增量编译
          COMMAND ${CMAKE_COMMAND} -E copy_if_different "${COMPILE_TARGET}" "${OUTPUT_TARGET}"
          DEPENDS ${FILE}
          #COMMENT "Assembling ${FILE_NAME}"
          )
        list(APPEND ${OUTPUT_FILES} ${OUTPUT_TARGET})
      endforeach()

    endif(MSVC)
  endmacro()

endif(OS_WINDOWS)


#
# Target configuration macros.
#

# Add a logical target that can be used to link the specified libraries into an
# executable target.
# 构建一个逻辑目标，这个目标仅仅只是链接其他库，不具备任何代码，相当于把多个库整合到一个库以供其他目标依赖
macro(COMMON_ADD_LOGICAL_TARGET target debug_lib release_lib)
  add_library(${target} ${COMMON_LIBTYPE} IMPORTED)
  set_target_properties(${target} PROPERTIES
    IMPORTED_LOCATION "${release_lib}"
    IMPORTED_LOCATION_DEBUG "${debug_lib}"
    IMPORTED_LOCATION_RELEASE "${release_lib}"
    )
endmacro()

macro(COMMON_ADD_LOGICAL_TARGET_MODE target mode_lib)
  COMMON_ADD_LOGICAL_TARGET(${target} ${mode_lib} ${mode_lib})
endmacro()


# Set common target properties. Use SET_LIBRARY_TARGET_PROPERTIES() or
# SET_EXECUTABLE_TARGET_PROPERTIES() instead of calling this macro directly.
macro(COMMON_SET_BASE_PROPERTIES target)
  # Compile flags.
  target_compile_options(${target} PRIVATE ${COMMON_COMPILER_FLAGS} ${COMMON_CXX_COMPILER_FLAGS})
  target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:${COMMON_COMPILER_FLAGS_DEBUG} ${COMMON_CXX_COMPILER_FLAGS_DEBUG}>)
  target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:${COMMON_COMPILER_FLAGS_RELEASE} ${COMMON_CXX_COMPILER_FLAGS_RELEASE}>)

  # Compile definitions.
  target_compile_definitions(${target} PRIVATE ${COMMON_COMPILER_DEFINES})
  target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:${COMMON_COMPILER_DEFINES_DEBUG}>)
  target_compile_definitions(${target} PRIVATE $<$<CONFIG:Release>:${COMMON_COMPILER_DEFINES_RELEASE}>)
  
  # 附加包含目录，根目录.
  target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})

  # 附加包含目录，根目录.
  target_include_directories(${target} PRIVATE ${LCPFW_SOURCE_DIR})

  # 附加包含目录，包含自己模块所在根目录.
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  # Linker flags.
  if(COMMON_LINKER_FLAGS)
    string(REPLACE ";" " " _flags_str "${COMMON_LINKER_FLAGS}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS ${_flags_str})
  endif()
  if(COMMON_LINKER_FLAGS_DEBUG)
    string(REPLACE ";" " " _flags_str "${COMMON_LINKER_FLAGS_DEBUG}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS_DEBUG ${_flags_str})
  endif()
  if(COMMON_LINKER_FLAGS_RELEASE)
    string(REPLACE ";" " " _flags_str "${COMMON_LINKER_FLAGS_RELEASE}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS_RELEASE ${_flags_str})
  endif()

  if(OS_MACOSX)
    # Set Xcode target properties.
    set_target_properties(${target} PROPERTIES
      XCODE_ATTRIBUTE_ALWAYS_SEARCH_USER_PATHS                    NO
      XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD                 "gnu++11"   # -std=gnu++11
      XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME                     NO          # -fno-objc-link-runtime
      XCODE_ATTRIBUTE_CLANG_WARN_OBJC_MISSING_PROPERTY_SYNTHESIS  YES         # -Wobjc-missing-property-synthesis
      XCODE_ATTRIBUTE_COPY_PHASE_STRIP                            NO
      XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING[variant=Release]        YES         # -Wl,-dead_strip
      XCODE_ATTRIBUTE_GCC_C_LANGUAGE_STANDARD                     "c99"       # -std=c99
      XCODE_ATTRIBUTE_GCC_CW_ASM_SYNTAX                           NO          # No -fasm-blocks
      XCODE_ATTRIBUTE_GCC_DYNAMIC_NO_PIC                          NO
      XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS                   NO          # -fno-exceptions
      XCODE_ATTRIBUTE_GCC_ENABLE_CPP_RTTI                         NO          # -fno-rtti
      XCODE_ATTRIBUTE_GCC_ENABLE_PASCAL_STRINGS                   NO          # No -mpascal-strings
      XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN              YES         # -fvisibility-inlines-hidden
      XCODE_ATTRIBUTE_GCC_OBJC_CALL_CXX_CDTORS                    YES         # -fobjc-call-cxx-cdtors
      XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN                  YES         # -fvisibility=hidden
      XCODE_ATTRIBUTE_GCC_THREADSAFE_STATICS                      NO          # -fno-threadsafe-statics
      XCODE_ATTRIBUTE_GCC_TREAT_WARNINGS_AS_ERRORS                YES         # -Werror
      XCODE_ATTRIBUTE_GCC_VERSION                                 "com.apple.compilers.llvm.clang.1_0"
      XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_NEWLINE              YES         # -Wnewline-eof
      XCODE_ATTRIBUTE_USE_HEADERMAP                               NO
      OSX_ARCHITECTURES_DEBUG                                     "${CMAKE_OSX_ARCHITECTURES}"
      OSX_ARCHITECTURES_RELEASE                                   "${CMAKE_OSX_ARCHITECTURES}"
      )
  endif()
endmacro()

# Set library-specific properties.
macro(COMMON_SET_LIBRARY_TARGET_PROPERTIES target)
  COMMON_SET_BASE_PROPERTIES(${target})

  # Shared library linker flags.
  if(COMMON_SHARED_LINKER_FLAGS)
    string(REPLACE ";" " " _flags_str "${COMMON_SHARED_LINKER_FLAGS}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS ${_flags_str})
  endif()
  if(COMMON_SHARED_LINKER_FLAGS_DEBUG)
    string(REPLACE ";" " " _flags_str "${COMMON_SHARED_LINKER_FLAGS_DEBUG}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS_DEBUG ${_flags_str})
  endif()
  if(COMMON_SHARED_LINKER_FLAGS_RELEASE)
    string(REPLACE ";" " " _flags_str "${COMMON_SHARED_LINKER_FLAGS_RELEASE}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS_RELEASE ${_flags_str})
  endif()
endmacro()

# Set executable-specific properties.
macro(COMMON_SET_EXECUTABLE_TARGET_PROPERTIES target)
  COMMON_SET_BASE_PROPERTIES(${target})

  # Executable linker flags.
  if(COMMON_EXE_LINKER_FLAGS)
    string(REPLACE ";" " " _flags_str "${COMMON_EXE_LINKER_FLAGS}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS ${_flags_str})
  endif()
  if(COMMON_EXE_LINKER_FLAGS_DEBUG)
    string(REPLACE ";" " " _flags_str "${COMMON_EXE_LINKER_FLAGS_DEBUG}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS_DEBUG ${_flags_str})
  endif()
  if(COMMON_EXE_LINKER_FLAGS_RELEASE)
    string(REPLACE ";" " " _flags_str "${COMMON_EXE_LINKER_FLAGS_RELEASE}")
    set_property(TARGET ${target} PROPERTY LINK_FLAGS_RELEASE ${_flags_str})
  endif()
endmacro()
