

# build png-to-badge-asset as a tool that
#  can be used during the build process
cmake_minimum_required(VERSION 3.12)
project(badge_tools  C)

find_package(PNG REQUIRED)

add_executable(png-to-badge-asset
  png-to-badge-asset.c
)

target_sources(png-to-badge-asset
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../source/hal/png_utils.c
)

target_include_directories(png-to-badge-asset
  PRIVATE
    ${CMAKE_SOURCE_DIR}/source/hal
)

target_link_libraries(png-to-badge-asset
  PRIVATE
    PNG::PNG
)

set_target_properties(png-to-badge-asset PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)

