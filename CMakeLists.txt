cmake_minimum_required(VERSION 3.29.0)
project(texc VERSION 0.1.1)

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
