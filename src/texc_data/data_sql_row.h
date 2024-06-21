#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sqlite3.h"

#include "../texc_utils/http_request.h"

#define ROW_MATCH_IDX 0
#define ROW_EXPAND_IDX 1
#define ROW_ID_IDX 2
#define ROW_ENABLE_IDX 3
#define ROW_GROUP_IDX 4

typedef struct {
    // Private field, not saved to .csv files
    size_t index;

    char *match;
    char *expand;

    size_t id;
    bool enabled;
    char *group;
} DataSqlRow;

/**
 * @brief Creates a DataSqlRow from http request
 *
 * @param request The http request
 * @param error The error message
 * @return DataSqlRow or NULL on error
 */
DataSqlRow *data_sql_row_from_request(Request *request, char **error);

/**
 * @brief Creates a DataSqlRow from csv line
 *
 * @param csv_row The csv line
 * @param pos_table The array of index of the row header
 * @param error The error message
 * @return DataSqlRow or NULL on error
 */
DataSqlRow *data_sql_row_from_csv(char **csv_row, int *pos_table, char **error);

/**
 * @brief Creates a DataSqlRow from select query
 *
 * @param stmt The sqlite select query
 * @return DataSqlRow or NULL on error
 */
DataSqlRow *data_sql_row_from_stmt(sqlite3_stmt *stmt);

/**
 * @brief Frees the memory of the DataSqlRow
 */
void data_sql_row_free(DataSqlRow *row);
