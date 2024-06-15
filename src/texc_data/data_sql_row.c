#include "data_sql_row.h"
#include "data_sql.h"

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

DataSqlRow *data_sql_row_from_csv(const char **csv_line) {
    char *match = csv_next_field(csv_line);
    char *expand = csv_next_field(csv_line);
    
    char *id = csv_next_field(csv_line);
    size_t i_id = data_sql_missing_int("id");

    if (id != NULL) {
        if (str_isnumber(id) && atoi(id) >= 0) {
            i_id = atoi(id);
        }
        free(id);
    }

    char *enabled = csv_next_field(csv_line);
    bool b_enabled = true;

    // set enabled to be true for all cases except when enabled is 0
    if (enabled != NULL) {
        if (str_eq(enabled, "0")) {
            b_enabled = false;
        }
        free(enabled);
    }

    DataSqlRow *row = malloc(sizeof(DataSqlRow));
    row->match = match;
    row->expand = expand;
    row->index = data_sql_missing_int("_index");
    row->id = i_id;
    row->enabled = b_enabled;

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
