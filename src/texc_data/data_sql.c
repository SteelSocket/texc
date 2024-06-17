#include "data_sql.h"
#include "data.h"
#include "data_sql_row.h"

#include <string.h>

#include "../texc_match/match.h"

#include "../texc_utils/array.h"
#include "../texc_utils/logger.h"

void __row_bind_column(sqlite3_stmt *stmt, DataSqlRow *row) {
    sqlite3_bind_int(stmt, 1, row->index);

    sqlite3_bind_text(stmt, 2, row->match, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, row->expand, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, row->id);
    sqlite3_bind_int(stmt, 5, row->enabled);
    sqlite3_bind_text(stmt, 6, row->group, -1, SQLITE_STATIC);

    char *minit = match_get_initializer(row->match);
    if (minit != NULL) {
        sqlite3_bind_text(stmt, 7, minit, -1, SQLITE_STATIC);
        free(minit);
    } else {
        sqlite3_bind_null(stmt, 7);
    }
}

bool data_sql_init() {
    if (sqlite3_open(":memory:", &data.db)) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        return false;
    }

    const char *exptext_table =
        "CREATE TABLE expandtexts ("
        "_index INTEGER NOT NULL UNIQUE,"

        "match TEXT NOT NULL,"
        "expand TEXT NOT NULL,"
        "id INTEGER NOT NULL UNIQUE,"
        "enabled INTEGER NOT NULL CHECK (enabled IN (0, 1)),"
        "\"group\" TEXT NOT NULL,"

        "__match_init TEXT"
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

void data_sql_add(DataSqlRow *row) {
    sqlite3_stmt *stmt;
    const char *insert_sql =
        "INSERT INTO expandtexts VALUES (?, ?, ?, ?, ?, ?, ?)";

    int rc = sqlite3_prepare_v2(data.db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        return;
    }

    __row_bind_column(stmt, row);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
    LOGGER_FORMAT_LOG(LOGGER_INFO,
                      "Added record to expandtexts table with id=%zd", row->id);
}

char ***data_sql_get_raw(const char *columns, const char *condition,
                         int *row_count, int *col_count) {
    sqlite3_stmt *stmt;
    char *select_sql;

    if (columns == NULL)
        columns = "match, expand, id, enabled, \"group\"";

    if (condition == NULL) {
        str_format(select_sql, "SELECT %s FROM expandtexts", columns);
    } else {
        str_format(select_sql, "SELECT %s FROM expandtexts WHERE %s", columns, condition);
    }


    int rc = sqlite3_prepare_v2(data.db, select_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        free(select_sql);
        return NULL;
    }

    char ***raw_rows = array_create(char **);

    *row_count = 0;
    *col_count = sqlite3_column_count(stmt);

    // Create the header row
    raw_rows[(*row_count)++] = malloc(sizeof(char *) * *col_count);
    for (int i = 0; i < *col_count; i++) {
        raw_rows[0][i] = strdup(sqlite3_column_name(stmt, i));
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        array_increase_size(raw_rows, *row_count, sizeof(char **));

        int i = (*row_count)++;
        raw_rows[i] = malloc(sizeof(char *) * *col_count);

        for (int j = 0; j < *col_count; j++) {
            char *field = strdup(sqlite3_column_text(stmt, j));
            raw_rows[i][j] = field;
        }
    }

    sqlite3_finalize(stmt);
    free(select_sql);
    return raw_rows;

}

DataSqlRow **data_sql_get_row(const char *condition, int *size) {
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

    DataSqlRow **rows = array_create(DataSqlRow *);
    *size = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        DataSqlRow *row = data_sql_row_from_stmt(stmt);
        array_resize_add(rows, *size, row, DataSqlRow *);
    }

    sqlite3_finalize(stmt);
    free(select_sql);
    return rows;
}

int data_sql_delete(const char *condition) {
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
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOGGER_ERROR(sqlite3_errmsg(data.db));
        sqlite3_finalize(stmt);
        return -1;
    }

    int delete_count = sqlite3_changes(data.db);
    sqlite3_finalize(stmt);

    return delete_count;
}

void data_sql_print() {
    printf("--------------------------------------------------\n");
    printf("--------------------------------------------------\n");
    int count;
    DataSqlRow **rows = data_sql_get_row(NULL, &count);
    if (rows != NULL) {
        for (int i = 0; i < count; i++) {
            DataSqlRow *row = rows[i];
            printf("::%zd, %zd::\n", row->index, row->id);
            data_sql_row_free(row);
        }

        free(rows);
    }

    printf("--------------------------------------------------\n");
    printf("--------------------------------------------------\n\n");
}
