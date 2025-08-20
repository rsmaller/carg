set(CARG_PROJECT_VERSION "1.0")
set(CARG_PROJECT_REVISION "0")

cmake_minimum_required(VERSION 3.11.0)
set(CMAKE_CXX_STANDARD 23)
project(carg-example VERSION ${CARG_PROJECT_VERSION})
project(carg VERSION ${CARG_PROJECT_VERSION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/include)

IF(Example STREQUAL "threadsafe")
    add_executable(carg-example ${CMAKE_SOURCE_DIR}/example/example_ts.c)
ELSE()
    add_executable(carg-example ${CMAKE_SOURCE_DIR}/example/example.c)
ENDIF()

IF (Link STREQUAL "static")
    add_library(carg STATIC ${CMAKE_SOURCE_DIR}/src/carg_impl.c)
    target_compile_definitions(carg PRIVATE CARG_STATIC)
ELSE()
    add_library(carg SHARED ${CMAKE_SOURCE_DIR}/src/carg_impl.c)
ENDIF()

target_compile_definitions(carg PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(carg-example carg)

IF (MSVC)
    target_compile_definitions(carg-example PRIVATE _CRT_SECURE_NO_WARNINGS)
    set(CMAKE_EXE_LINKER_FLAGS "/IGNORE:4099 /IGNORE:4098 /IGNORE:4996")
ENDIF()

IF (Sanitize STREQUAL "address")
    IF (MSVC)
        target_compile_options(carg-example PRIVATE /fsanitize=address)
    ELSE()
        target_compile_options(carg-example PRIVATE -g -fsanitize=address -fno-omit-frame-pointer -O0)
        target_link_options(carg-example PRIVATE -fsanitize=address)
    ENDIF()
ENDIF()

# CMake Package Generator

install(TARGETS carg
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY include/
        DESTINATION include/carg/
)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ryan Smaller")
set(CPACK_PACKAGE_NAME "libcarg")
set(CPACK_PACKAGE_VERSION ${CARG_PROJECT_VERSION}-${CARG_PROJECT_REVISION})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_amd64)
set(CPACK_PACKAGE_DESCRIPTION "A lightweight command line argument parser written in C.")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")

include(CPack)