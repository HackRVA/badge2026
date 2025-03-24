set(DUKTAPE_VERSION "2.7.0")
set(DUKTAPE_URL "https://duktape.org/duktape-${DUKTAPE_VERSION}.tar.xz")
set(DUKTAPE_TAR "${CMAKE_BINARY_DIR}/duktape-${DUKTAPE_VERSION}.tar.xz")
set(DUKTAPE_SOURCE_DIR "${CMAKE_BINARY_DIR}/duktape-${DUKTAPE_VERSION}")

if(NOT EXISTS ${DUKTAPE_TAR})
    file(DOWNLOAD ${DUKTAPE_URL} ${DUKTAPE_TAR})
endif()

if(NOT EXISTS ${DUKTAPE_SOURCE_DIR})
    execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xvf ${DUKTAPE_TAR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endif()

file(GLOB DUKTAPE_SRC "${DUKTAPE_SOURCE_DIR}/src/*.c")

add_library(duktape STATIC ${DUKTAPE_SRC})
# target_include_directories(duktape PUBLIC "${DUKTAPE_SOURCE_DIR}/src")
# target_include_directories(${PRODUCT} PRIVATE "${DUKTAPE_SOURCE_DIR}/src")
# target_link_libraries(${PRODUCT} duktape)
