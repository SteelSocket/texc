#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../texc_utils/http_request.h"

typedef struct {
    size_t index;

    size_t id;
    bool enabled;
} DataSqlRow;

DataSqlRow data_sql_row_from_request(Request *request, char **error);

DataSqlRow data_sql_row_from_csv(const char **csv_line);
