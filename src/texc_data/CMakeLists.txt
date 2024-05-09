add_library(texc_data
    data.c
    data_sql.c
    data_sql_row.c
    data_io.c
)

target_link_libraries(texc_data sqlite3)
