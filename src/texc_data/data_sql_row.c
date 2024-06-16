#include "data_sql_row.h"
#include "data_sql.h"

#include "../texc_utils/array.h"
#include "../texc_utils/csv.h"
#include "../texc_utils/str.h"

DataSqlRow *data_sql_row_from_request(Request *request, char **error) {
    const char *match = request_get_query(request, "match");
    if (match == NULL) {
        *error = strdup("match param is not given");
        return NULL;
    }

    const char *expand = request_get_query(request, "expand");
    if (expand == NULL) {
        *error = strdup("expand param is not given");
        return NULL;
    }

    const char *enabled = request_get_query(request, "enabled");
    bool b_enabled = true;

    if (str_eq(enabled, "false")) {
        b_enabled = false;
    } else if (!str_eq(enabled, "true")) {
        *error = strdup("enabled param must be a boolean value");
        return NULL;
    }

    DataSqlRow *row = malloc(sizeof(DataSqlRow));
    row->index = data_sql_missing_int("_index");
    row->match = strdup(match);
    row->expand = strdup(expand);
    row->id = data_sql_missing_int("id");
    row->enabled = b_enabled;

    return row;
}

DataSqlRow *data_sql_row_from_csv(char **csv_row, int *pos_table,
                                  char **error) {
    char *match = csv_row[pos_table[ROW_MATCH_IDX]];
    char *expand = csv_row[pos_table[ROW_EXPAND_IDX]];
    char *id = csv_row[pos_table[ROW_ID_IDX]];
    char *enabled = csv_row[pos_table[ROW_ENABLE_IDX]];

    // TODO Impl default values for missing fields
    if (match == NULL) {
        *error = strdup("\"match\" csv field is NULL");
        return NULL;
    } else if (expand == NULL) {
        *error = strdup("\"expand\" csv field is NULL");
        return NULL;
    } else if (id == NULL) {
        *error = strdup("\"id\" csv field is NULL");
        return NULL;
    } else if (enabled == NULL) {
        *error = strdup("\"enabled\" csv field is NULL");
        return NULL;
    }

    DataSqlRow *row = malloc(sizeof(DataSqlRow));
    row->index = data_sql_missing_int("_index");
    row->match = strdup(match);
    row->expand = strdup(expand);
    row->id = atoi(id);
    row->enabled = (bool)atoi(enabled);

    return row;
}

DataSqlRow *data_sql_row_from_stmt(sqlite3_stmt *stmt) {
    int idx = sqlite3_column_int(stmt, 0);

    const char *match = (const char *)sqlite3_column_text(stmt, 1);
    const char *expand = (const char *)sqlite3_column_text(stmt, 2);
    int id = sqlite3_column_int(stmt, 3);
    int enabled = sqlite3_column_int(stmt, 4);

    DataSqlRow *row = malloc(sizeof(DataSqlRow));
    row->match = strdup(match);
    row->expand = strdup(expand);
    row->index = idx;
    row->id = id;
    row->enabled = enabled;

    return row;
}

void data_sql_row_free(DataSqlRow *row) {
    free(row->match);
    free(row->expand);
    free(row);
}
