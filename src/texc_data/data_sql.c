#include "data_sql.h"
#include <string.h>
#include "data.h"

#include "../texc_utils/array.h"
#include "../texc_utils/logger.h"

bool data_sql_init() {
    if (sqlite3_open(":memory:", &data.db)) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        return false;
    }

    const char *exptext_table =
        "CREATE TABLE expandtexts ("
        "idx INTEGER NOT NULL UNIQUE,"
        "id INTEGER NOT NULL UNIQUE"
        ")";

    if (sqlite3_exec(data.db, exptext_table, 0, 0, &data.db_error)) {
        LOGGER_ERROR(data.db_error);
        sqlite3_free(data.db_error);
        return false;
    }

    return true;
}

void data_sql_free() { sqlite3_close(data.db); }

int data_sql_missing_int(const char *column) {
    sqlite3_stmt *stmt;
    char *query_cmd;

    str_format(
        query_cmd,
        "WITH RECURSIVE all_ids(id) AS (VALUES(0) UNION ALL SELECT id + 1 FROM "
        "all_ids LIMIT 99999) SELECT id FROM all_ids WHERE NOT EXISTS (SELECT "
        "1 FROM expandtexts WHERE %s = all_ids.id) LIMIT 1",
        column);

    int rc = sqlite3_prepare_v2(data.db, query_cmd, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        free(query_cmd);
        return -1;
    }

    int required = -1;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        required = sqlite3_column_int(stmt, 0);
    }

    free(query_cmd);
    return required;
}

void data_sql_add(DataSqlRow row) {
    sqlite3_stmt *stmt;
    const char *insert_sql = "INSERT INTO expandtexts (idx, id) VALUES (?, ?)";

    int rc = sqlite3_prepare_v2(data.db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        return;
    }

    sqlite3_bind_int(stmt, 1, row.index);
    sqlite3_bind_int(stmt, 2, row.id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
    LOGGER_FORMAT_LOG(LOGGER_INFO,
                      "Added record to expandtexts table with id=%zd", row.id);
}

DataSqlRow *data_sql_get(const char *condition, int *size) {
    sqlite3_stmt *stmt;
    char *select_sql;

    if (condition != NULL) {
        str_format(select_sql, "SELECT * FROM expandtexts WHERE %s", condition);
    } else {
        select_sql = strdup("SELECT * FROM expandtexts");
    }

    int rc = sqlite3_prepare_v2(data.db, select_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        free(select_sql);
        return NULL;
    }

    DataSqlRow *rows = array_create(DataSqlRow);
    *size = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int idx = sqlite3_column_int(stmt, 0);
        int id = sqlite3_column_int(stmt, 1);

        DataSqlRow row = (DataSqlRow){.index = idx, .id = id};
        array_resize_add(rows, *size, row, DataSqlRow);
    }

    free(select_sql);
    return rows;
}

bool data_sql_delete(const char *condition) {
    sqlite3_stmt *stmt;
    char *delete_sql;

    if (condition != NULL) {
        str_format(delete_sql, "DELETE FROM expandtexts WHERE %s", condition);
    } else {
        delete_sql = strdup("DELETE FROM expandtexts");
    }

    int rc = sqlite3_prepare_v2(data.db, delete_sql, -1, &stmt, NULL);
    free(delete_sql);

    if (rc != SQLITE_OK) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

void data_sql_print() {
    printf("--------------------------------------------------\n");
    printf("--------------------------------------------------\n");
    int count;
    DataSqlRow *rows = data_sql_get(NULL, &count);
    if (rows != NULL) {
        for (int i = 0; i < count; i++) {
            DataSqlRow row = rows[i];
            printf("::%d, %d::\n", row.index, row.id);
        }

        free(rows);
    }

    printf("--------------------------------------------------\n");
    printf("--------------------------------------------------\n\n");
}
