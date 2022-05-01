# copy from obs-studio\cmake\copy_on_debug_helper.cmake

# 在需要的地方调用cmake可执行文件执行该cmake脚本，作用是将${INPUT}文件拷贝到${OUTPUT}。
# 例如在一个目标的构建命令里设置，使用生成器表达式，根据target产出文件，将其pdb文件拷贝至dest目录：
# add_custom_command(TARGET ${target} POST_BUILD
#	COMMAND "${CMAKE_COMMAND}"
#		"-DCONFIG=$<CONFIGURATION>"
#		"-DFNAME=$<TARGET_FILE_NAME:${target}>"
#		"-DINPUT=$<TARGET_FILE_DIR:${target}>"
#		"-DOUTPUT=${dest}"
#		-P "${CMAKE_SOURCE_DIR}/cmake/copy_on_debug_helper.cmake"
#	VERBATIM)
#

string(REGEX REPLACE "\\.(dll|exe)$" ".pdb" FNAME "${FNAME}")

# obs是针对项目的debug/release模式区分对待，只有debug下才拷贝文件，不然直接删除文件，以便保持release下构建输出的清爽。
# 我们这里改一下，任何项目模式都拷贝文件，并且不删除输出文件
if(EXISTS "${INPUT}/${FNAME}")
	file(COPY "${INPUT}/${FNAME}" DESTINATION "${OUTPUT}")
endif()

#if(CONFIG STREQUAL Debug OR CONFIG STREQUAL RelWithDebInfo)
#	file(COPY "${INPUT}/${FNAME}" DESTINATION "${OUTPUT}")
#elseif(EXISTS "${OUTPUT}/${FNAME}")
#	file(REMOVE "${OUTPUT}/${FNAME}")
#endif()
