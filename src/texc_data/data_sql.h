#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    size_t index;
    size_t id;
} DataSqlRow;

bool data_sql_init();

void data_sql_free();

void data_sql_add(DataSqlRow row);

DataSqlRow *data_sql_get(const char *condition, int *size);

int data_sql_missing_int(const char *column);

bool data_sql_delete(const char *condition);

void data_sql_print();
