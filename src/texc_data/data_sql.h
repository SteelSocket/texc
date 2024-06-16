#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "data_sql_row.h"

bool data_sql_init();

void data_sql_free();

void data_sql_add(DataSqlRow *row);

char ***data_sql_get_raw(const char *columns, const char *condition,
                         int *row_count, int *col_count);

DataSqlRow **data_sql_get_row(const char *condition, int *size);

int data_sql_missing_int(const char *column);

int data_sql_delete(const char *condition);

void data_sql_print();
