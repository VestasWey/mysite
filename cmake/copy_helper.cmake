# copy from obs-studio\cmake\copy_helper.cmake


# 在需要的地方调用cmake可执行文件执行该cmake脚本，作用是将${INPUT}文件拷贝到${OUTPUT}。
# 例如在一个目标的构建命令里设置：
# add_custom_command(TARGET ${target} POST_BUILD
#	COMMAND "${CMAKE_COMMAND}"
#		"-DCONFIG=$<CONFIGURATION>"
#		"-DTARGET_CONFIGS=${target_configs}"
#		"-DINPUT=${source}"
#		"-DOUTPUT=${dest}"
#		-P "${CMAKE_SOURCE_DIR}/cmake/copy_helper.cmake"
#	VERBATIM)


if(NOT EXISTS "${INPUT}")
	return()
endif()

set(_do_pass FALSE)
foreach(target ${TARGET_CONFIGS})
	if(target STREQUAL "${CONFIG}" OR target STREQUAL "ALL")
		set(_do_pass TRUE)
	endif()
endforeach()

if(NOT _do_pass)
	return()
endif()

file(COPY "${INPUT}" DESTINATION "${OUTPUT}")
