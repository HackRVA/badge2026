
# target_add_png_badge_asset
#
# This calls png-to-badge-asset and generates a .h file in the build dir.
# That file is then available to include
#
# e.g.
# if you have a png called `example_png_asset.png`
#
# In a CMakeLists.txt file, you can call
# ```cmake
# target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/example_png.png)
# ```
#
# in your c file:
# ```c
# #include "example_png_asset.h"
#
# ...
# FbImage4bit2(&example_png, 0);
# ...
#
# ````
#
# note: you can also add the `--nostatic`
# e.g.
# ```
# target_add_png_badge_asset(${PRODUCT} 4 ${CMAKE_CURRENT_LIST_DIR}/example_png.png EXTRA_FLAGS --nostatic)
# ```
function(target_add_png_badge_asset TARGET BITDEPTH PNG_FILE)
	# set_property(GLOBAL PROPERTY TARGET_MESSAGES OFF)
	cmake_parse_arguments(ARG "" "" "EXTRA_FLAGS" ${ARGN})

	get_filename_component(PNG_NAME "${PNG_FILE}" NAME_WE)
	string(TOUPPER "${PNG_NAME}_ASSET_H" GUARD_NAME)
	set(ASSET_H "${CMAKE_CURRENT_BINARY_DIR}/${PNG_NAME}_asset.h")

	add_custom_command(
		OUTPUT "${ASSET_H}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
		COMMAND ${CMAKE_COMMAND} -E echo "#ifndef ${GUARD_NAME}" > "${ASSET_H}"
		COMMAND ${CMAKE_COMMAND} -E echo "#define ${GUARD_NAME}" >> "${ASSET_H}"
		COMMAND $<TARGET_FILE:png-to-badge-asset> ${BITDEPTH} "${PNG_FILE}" ${ARG_EXTRA_FLAGS} >> "${ASSET_H}"
		COMMAND ${CMAKE_COMMAND} -E echo "#endif /* ${GUARD_NAME} */" >> "${ASSET_H}"
		DEPENDS png-to-badge-asset "${PNG_FILE}"
		COMMENT "generating asset: ${ASSET_H}"
		VERBATIM
	)

	add_library(${PNG_NAME}_asset_obj OBJECT "${ASSET_H}")
	add_dependencies(${PNG_NAME}_asset_obj ${PNG_NAME}_asset_obj)

	target_sources(${TARGET} PRIVATE $<TARGET_OBJECTS:${PNG_NAME}_asset_obj>)
	target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

	add_dependencies(${TARGET} ${PNG_NAME}_asset_obj)
endfunction()

function(badge_png_asset_add NAME BITDEPTH PNG_FILE)
	get_property(_have_bits GLOBAL PROPERTY ASSET_${NAME}_BITDEPTH)
	if(NOT _have_bits)
		set_property(GLOBAL PROPERTY ASSET_${NAME}_BITDEPTH "${BITDEPTH}")
	endif()

	get_property(_have_guard GLOBAL PROPERTY ASSET_${NAME}_GUARD)
	if(NOT _have_guard)
		string(TOUPPER "${NAME}_ASSET_H" _guard)
		set_property(GLOBAL PROPERTY ASSET_${NAME}_GUARD "${_guard}")
	endif()

	get_property(_list GLOBAL PROPERTY ASSET_${NAME}_PNG_FILES)
	list(APPEND _list "${PNG_FILE}")
	set_property(GLOBAL PROPERTY ASSET_${NAME}_PNG_FILES "${_list}")
endfunction()


function(badge_png_asset_build NAME TARGET)
	get_property(_bits GLOBAL PROPERTY ASSET_${NAME}_BITDEPTH)
	get_property(_guard GLOBAL PROPERTY ASSET_${NAME}_GUARD)
	get_property(_pngs GLOBAL PROPERTY ASSET_${NAME}_PNG_FILES)

	if(NOT _pngs)
		message(FATAL_ERROR "badge_png_asset_add(${NAME} ...) must be called at least once before badge_png_asset_build(${NAME})")
	endif()

	set(_out "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_asset.h")
	set(_deps png-to-badge-asset ${_pngs})

	set(_cmds
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"
		COMMAND ${CMAKE_COMMAND} -E echo "#ifndef ${_guard}" > "${_out}"
		COMMAND ${CMAKE_COMMAND} -E echo "#define ${_guard}" >> "${_out}"
	)
	foreach(p IN LISTS _pngs)
		list(APPEND _cmds
			COMMAND $<TARGET_FILE:png-to-badge-asset> ${_bits} "${p}" >> "${_out}"
		)
	endforeach()
	list(APPEND _cmds
		COMMAND ${CMAKE_COMMAND} -E echo "#endif /* ${_guard} */" >> "${_out}"
	)

	add_custom_command(
		OUTPUT "${_out}"
		${_cmds}
		DEPENDS ${_deps}
		COMMENT "Generating combined asset header: ${_out}"
		VERBATIM
	)

	add_library(${NAME}_asset_obj OBJECT "${_out}")

	target_sources(${TARGET}
		PRIVATE $<TARGET_OBJECTS:${NAME}_asset_obj>
	)
	target_include_directories(${TARGET}
		PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
	)
	add_dependencies(${TARGET} ${NAME}_asset_obj)
endfunction()
