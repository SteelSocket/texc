#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "data_sql_row.h"

/**
 * @brief Initializes the database and creates the sqlite table
 *
 * @return Boolean representing success
 */
bool data_sql_init();

/**
 * @brief Frees the sqlite database
 */
void data_sql_free();

/**
 * @brief Adds a row to sqlite table
 */
void data_sql_add(DataSqlRow *row);

/**
 * @brief Returns a the table rows as string table
 *
 * @param columns The columns to select
 * @param condition The condition for select
 * @param row_count The number of row retrieved
 * @param col_count The number of columns retrieved
 * @return string table
 */
char ***data_sql_get_raw(const char *columns, const char *condition,
                         int *row_count, int *col_count);

/**
 * @brief Returns the table rows
 *
 * @param condition The condition for select
 * @param count The row count
 * @return array of rows
 */
DataSqlRow **data_sql_get_row(const char *condition, int *count);

/**
 * @brief Gets the next missing int from a column
 *
 * @param column The column to get the missing int from
 * @return The missing int or -1 on error
 */
int data_sql_missing_int(const char *column);

/**
 * @brief Deletes the row(s) from the table
 *
 * @param condition The condition for the delete
 * @return The number of rows deleted or -1 on error
 */
int data_sql_delete(const char *condition);

