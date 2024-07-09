cmake_minimum_required(VERSION 3.29.0)
project(texc VERSION 0.4.0)

set(EXTERNAL_INCLUDE_DIRS "" CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_compile_definitions(TEXC_VERSION="${PROJECT_VERSION}")

add_subdirectory(external)
include_directories("${EXTERNAL_INCLUDE_DIRS}")

# Build all sub modules
add_subdirectory(src)

add_executable(texc
    src/main.c

    src/expandtext.c
    src/settings.c

    src/args.c
    src/subcmds.c
)

target_link_libraries(texc
    sqlite3
    inih

    texc_utils
    texc_data
    texc_http
    texc_tags

    texc_expand
    texc_match
    texc_keyhook
)

if(MSVC)
    target_compile_options(texc PRIVATE /W4)
else()
    target_compile_options(texc PRIVATE -Wall -Wextra)
endif()

# CPack configuration
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

if (WIN32 AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
endif()
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ")
endif()

install(TARGETS texc DESTINATION .)
include(CPack)
