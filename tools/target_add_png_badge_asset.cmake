
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

	add_custom_target(${PNG_NAME}_asset_gen DEPENDS "${ASSET_H}")

	add_library(${PNG_NAME}_asset_obj OBJECT "${ASSET_H}")
	add_dependencies(${PNG_NAME}_asset_obj ${PNG_NAME}_asset_gen)

	target_sources(${TARGET} PRIVATE $<TARGET_OBJECTS:${PNG_NAME}_asset_obj>)
	add_dependencies(${TARGET} ${PNG_NAME}_asset_gen)

	target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()
