#copy from obs-studio/cmake/Modules/ObsHelpers.cmake

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
else()
	set(_lib_suffix 32)
endif()


# mac下链接framework，针对某些framework（例如cef）名称带空格的，xcode会拆分从而导致无法找到库的问题，
# 这个函数会对路径中的空格进行处理，即为空格添加转义符标识‘\’，以便于xcode正确保留路径中的空格符
function(lcpfw_link_framework target framework_full_path)
	if(NOT APPLE)
		return()
	endif()

	get_filename_component(_fw_dir ${framework_full_path} DIRECTORY)
	get_filename_component(_fw_name ${framework_full_path} NAME_WE)
	string(REPLACE " " "\\ " _fw ${_fw_name})
	set_property(TARGET ${target} PROPERTY LINK_FLAGS "-F ${_fw_dir} -framework ${_fw}")
endfunction()


# 从指定配置文件读取内容，并将配置项的keyword附加指定前缀后写入环境变量ENV中
# 典型场景是读取“resources\lcpfw\version\VERSION”中记录的程序版本信息，然后将读取到的信息设置到“LCPFW_VERSION”变量
function(lcpfw_read_profile cfg_file cfg_prefix)
	if(NOT EXISTS "${cfg_file}")
		message(WARNING "config file '${cfg_file}' not exists.")
		return()
	endif()

	file(READ ${cfg_file} raw_cfg_str)
	#message(STATUS "raw_cfg_str: ${raw_cfg_str}")
	
	string(REPLACE "\r\n" "\n" multiline_cfg_str ${raw_cfg_str})
	#message(STATUS "multiline_cfg_str: ${multiline_cfg_str}")
	
	string(REPLACE "\n" ";" cfg_list ${multiline_cfg_str})
	#message(STATUS "cfg_list: ${cfg_list}")

	foreach(cfg_item ${cfg_list})
		# split k v
		string(REPLACE "=" ";" cfg_item "${cfg_item}")
		#message(STATUS "cfg_item: ${cfg_item}")
		list(LENGTH cfg_item item_len)
		if(${item_len} MATCHES 2)
			list(GET cfg_item 0 k)
			list(GET cfg_item 1 v)
			#message(STATUS "k: ${k},  v: ${v}")
			set(k ${cfg_prefix}_${k})
			set(ENV{${k}} ${v})
		else()
			message(WARNING "config file '${cfg_file}' item ${cfg_item} not in 'k=v' format.")
		endif()
	endforeach()
endfunction()


# 向target添加一个将src文件拷贝到dest路径的命令
function(lcpfw_helper_copy_dir target target_configs source dest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}"
			"-DCONFIG=$<CONFIGURATION>"
			"-DTARGET_CONFIGS=${target_configs}"
			"-DINPUT=${source}"
			"-DOUTPUT=${dest}"
			-P "${CMAKE_SOURCE_DIR}/cmake/copy_helper.cmake"
		VERBATIM)
endfunction()


# 使用生成器表达式，根据target产出文件，将其pdb文件拷贝至dest目录
function(lcpfw_debug_copy_helper target dest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}"
			"-DCONFIG=$<CONFIGURATION>"
			"-DFNAME=$<TARGET_FILE_NAME:${target}>"
			"-DINPUT=$<TARGET_FILE_DIR:${target}>"
			"-DOUTPUT=${dest}"
			-P "${CMAKE_SOURCE_DIR}/cmake/copy_on_debug_helper.cmake"
		VERBATIM)
endfunction()


# 将target的产出的pdb文件拷贝至部署目录，根据target的分类决定部署目录中的具体子目录
# 核心的/主要的(ttype=="CORE")直接放置在根目录下，插件类(ttype=="PLUGIN")的放置在plugins子目录下，与obs的插件的部署目录obs-plugins区分开
function(install_lcpfw_pdb ttype target)
	if(NOT MSVC)
		return()
	endif()
	
	if(APPLE)
		set(_bit_suffix "")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_bit_suffix "x64/")
	else()
		set(_bit_suffix "")
	endif()

	if("${ttype}" STREQUAL "PLUGIN")
		lcpfw_debug_copy_helper(${target} "${COMMON_DEPLOYMENT_DIR}/${_bit_suffix}plugins")
	else()
		lcpfw_debug_copy_helper(${target} "${COMMON_DEPLOYMENT_DIR}")
	endif()
endfunction()


# 将主要的核心target（直接放置在部署目录根目录下的目标）的产出拷贝到部署目录，包括其pdb文件
function(install_lcpfw_core target)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:${target}>"
			"${COMMON_DEPLOYMENT_DIR}/$<TARGET_FILE_NAME:${target}>"
		VERBATIM)

	install_lcpfw_pdb(CORE ${target})
endfunction()


# 将插件类的target（放置在部署目录子目录plugins下的目标）的产出拷贝到部署目录，包括其pdb文件
function(install_lcpfw_plugin target)
	if(APPLE)
		set(_bit_suffix "")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_bit_suffix "x64/")
	else()
		set(_bit_suffix "")
	endif()

	set_target_properties(${target} PROPERTIES
		PREFIX "")

	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:${target}>"
			"${COMMON_DEPLOYMENT_DIR}/plugins/${_bit_suffix}$<TARGET_FILE_NAME:${target}>"
		VERBATIM)

	install_lcpfw_pdb(PLUGIN ${target})
endfunction()


# 将相对当前脚本文件路径的源目录以指定的相对路径拷贝到部署目录
function(install_lcpfw_data target datasrc datadest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/${datasrc}" "${COMMON_DEPLOYMENT_DIR}/${datadest}"
		VERBATIM)
endfunction()


# 将绝对路径的源目录以指定的相对路径拷贝到部署目录
function(install_lcpfw_data_from_abs_path target datasrc datadest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_directory
			"${datasrc}" "${COMMON_DEPLOYMENT_DIR}/${datadest}"
		VERBATIM)
endfunction()


# 将相对当前脚本文件路径的单个文件以指定的相对路径拷贝到部署目录
function(install_lcpfw_data_file target datafile datadest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E make_directory
			"${COMMON_DEPLOYMENT_DIR}/${datadest}"
		VERBATIM)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"${CMAKE_CURRENT_SOURCE_DIR}/${datafile}" "${COMMON_DEPLOYMENT_DIR}/${datadest}"
		VERBATIM)
endfunction()


# 将绝对路径的单个文件以指定的相对路径拷贝到部署目录
function(install_lcpfw_data_file_from_abs_path target datafile datadest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E make_directory
			"${COMMON_DEPLOYMENT_DIR}/${datadest}"
		VERBATIM)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"${datafile}" "${COMMON_DEPLOYMENT_DIR}/${datadest}"
		VERBATIM)
endfunction()


# 将目标产出以指定的相对路径拷贝到部署目录
function(install_lcpfw_datatarget target datadest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:${target}>"
			"${COMMON_DEPLOYMENT_DIR}/${datadest}/$<TARGET_FILE_NAME:${target}>"
		VERBATIM)
endfunction()


# 拷贝插件类target的产出以及它当前目录下的指定源目录到插件部署目录
function(install_lcpfw_plugin_with_data target datasrc)
	install_lcpfw_plugin(${target})
	install_lcpfw_data(${target} "${datasrc}" "data/${target}")
endfunction()
