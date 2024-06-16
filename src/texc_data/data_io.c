#include "data_io.h"
#include "data.h"
#include "data_sql.h"
#include "data_sql_row.h"

#include "../texc_utils/csv.h"
#include "../texc_utils/logger.h"
#include "../texc_utils/path.h"

#include <stdio.h>

char *__get_save_file() {
    char *data_dir = data_get_dir();
    char *save_path = path_join(data_dir, "matches.csv");
    free(data_dir);
    return save_path;
}

char *__expandtext_as_csv(DataSqlRow *row) {
    char *ret;
    str_format(ret, "%s,%s,%zd,%d\n", row->match, row->expand, row->id,
               row->enabled);
    return ret;
}

char *data_io_expandtexts_as_csv(const char *select_condition) {
    int count;
    DataSqlRow **rows = data_sql_get(NULL, &count);
    if (rows == NULL)
        return NULL;

    char *csv_string;
    str_mcpy(csv_string, "match,expand,id,enabled\n");

    for (int i = 0; i < count; i++) {
        DataSqlRow *row = rows[i];
        char *csv_line = __expandtext_as_csv(row);

        str_rcat(csv_string, csv_line);

        data_sql_row_free(row);
        free(csv_line);
    }

    free(rows);
    return csv_string;
}

void data_io_save() {
    char *save_path = __get_save_file();
    FILE *file = fopen(save_path, "w");
    free(save_path);

    char *csv_string = data_io_expandtexts_as_csv(NULL);
    if (csv_string) {
        fprintf(file, "%s", csv_string);
        free(csv_string);
        LOGGER_INFO("expandtexts saved to file successfully");
    }

    fclose(file);
}

char *__load_csv_table(CSVTable *table) {
    char *error;

    int pos_table[4] = {-1, -1, -1, -1};
    for (int c = 0; c < table->col_count; c++) {
        char *header = table->fields[0][c];

        if (str_eq(header, "match"))
            pos_table[ROW_MATCH_IDX] = c;
        else if (str_eq(header, "expand"))
            pos_table[ROW_EXPAND_IDX] = c;
        else if (str_eq(header, "id"))
            pos_table[ROW_ID_IDX] = c;
        else if (str_eq(header, "enabled"))
            pos_table[ROW_ENABLE_IDX] = c;
    }

    for (int r = 1; r < table->row_count; r++) {
        char **csv_row = table->fields[r];
        DataSqlRow *row = data_sql_row_from_csv(csv_row, pos_table, &error);

        if (row == NULL) {
            return error;
        }

        expandtext_add(row);
        data_sql_row_free(row);
    }

    return NULL;
}

void data_io_load() {
    char *save_path = __get_save_file();
    char *contents = path_read_all(save_path);
    free(save_path);

    if (contents == NULL)
        return;

    CSVTable *table = csv_read(contents);
    free(contents);

    if (table == NULL) {
        return;
    }
    char *error = __load_csv_table(table);
    csv_free(table);

    if (error != NULL) {
        LOGGER_ERROR(error);
        LOGGER_INFO("failed to load expandtexts from file");
        return;
    }

    LOGGER_INFO("expandtexts loaded from file successfully");
}
