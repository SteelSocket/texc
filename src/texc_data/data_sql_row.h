#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sqlite3.h"

#include "../texc_utils/http_request.h"


typedef struct {
    size_t index;

    char *match;
    char *expand;
    size_t id;
    bool enabled;
} DataSqlRow;

DataSqlRow *data_sql_row_from_request(Request *request, char **error);

DataSqlRow *data_sql_row_from_csv(const char **csv_line);

DataSqlRow *data_sql_row_from_stmt(sqlite3_stmt *stmt);

void data_sql_row_free(DataSqlRow *row);
