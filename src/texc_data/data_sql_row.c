#include "data_sql_row.h"
#include "data_sql.h"

#include "../texc_utils/csv.h"
#include "../texc_utils/str.h"

DataSqlRow data_sql_row_from_request(Request *request, char **error) {
    const char *enabled = request_get_query(request, "enabled");
    bool b_enabled = true;

    if (str_eq(enabled, "false")) {
        b_enabled = false;
    } else if (!str_eq(enabled, "true")) {
        *error = strdup("enabled param must be a boolean value");
        return (DataSqlRow){0};
    }

    return (DataSqlRow){
        .index = 0,
        .id = data_sql_missing_int("id"),
        .enabled = b_enabled,
    };
}

DataSqlRow data_sql_row_from_csv(const char **csv_line) {
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

    return (DataSqlRow){.index = 0, .id = i_id, .enabled = b_enabled};
}
