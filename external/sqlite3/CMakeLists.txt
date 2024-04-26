add_library(sqlite3
    sqlite3.c
    sqlite3.h
    sqlite3ext.h
)

list(APPEND EXTERNAL_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/external/sqlite3/")
set(EXTERNAL_INCLUDE_DIRS "${EXTERNAL_INCLUDE_DIRS}" CACHE INTERNAL "")
