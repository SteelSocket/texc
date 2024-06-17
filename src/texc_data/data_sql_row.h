#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sqlite3.h"

#include "../texc_utils/csv.h"
#include "../texc_utils/http_request.h"

#define ROW_MATCH_IDX 0
#define ROW_EXPAND_IDX 1
#define ROW_ID_IDX 2
#define ROW_ENABLE_IDX 3
#define ROW_GROUP_IDX 4

typedef struct {
    size_t index;

    char *match;
    char *expand;

    size_t id;
    bool enabled;
    char *group;
} DataSqlRow;

DataSqlRow *data_sql_row_from_request(Request *request, char **error);

DataSqlRow *data_sql_row_from_csv(char **csv_row, int *pos_table, char **error);

DataSqlRow *data_sql_row_from_stmt(sqlite3_stmt *stmt);

void data_sql_row_free(DataSqlRow *row);
