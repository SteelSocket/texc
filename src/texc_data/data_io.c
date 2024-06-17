#include "data_io.h"
#include "data.h"
#include "data_sql.h"
#include "data_sql_row.h"

#include "../texc_utils/csv.h"
#include "../texc_utils/logger.h"
#include "../texc_utils/path.h"

#include <stdio.h>

char *__get_save_file(const char *group) {
    char *data_dir = data_get_dir();
    char *save_path;
    str_format(save_path, "%s" PATH_SEPERATOR "matches" PATH_SEPERATOR "%s.csv", data_dir, group);

    free(data_dir);
    return save_path;
}

char *__expandtext_as_csv(DataSqlRow *row) {
    char *ret;
    str_format(ret, "%s,%s,%zd,%d\n", row->match, row->expand, row->id,
               row->enabled);
    return ret;
}

char *data_io_expandtexts_as_csv(const char *columns,
                                 const char *select_condition) {
    int row_count, col_count;
    char ***rows =
        data_sql_get_raw(columns, select_condition, &row_count, &col_count);
    if (rows == NULL) {
        return NULL;
    }

    char *csv_string = malloc(sizeof(char));
    csv_string[0] = '\0';

    for (int r = 0; r < row_count; r++) {
        for (int c = 0; c < col_count; c++) {
            if (c + 1 != col_count)
                str_rformat(csv_string, "%s,", rows[r][c]);
            else
                str_rformat(csv_string, "%s\n", rows[r][c]);
            free(rows[r][c]);
        }
        free(rows[r]);
    }
    free(rows);

    return csv_string;
}

void data_io_save_group(const char *group) {
    char *save_path = __get_save_file(group);
    FILE *file = fopen(save_path, "w");
    free(save_path);

    char *cond;
    str_format(cond, "\"group\" = '%s'", group);

    char *csv_string = data_io_expandtexts_as_csv("match,expand,id,enabled", cond);
    free(cond);

    if (csv_string) {
        fprintf(file, "%s", csv_string);
        free(csv_string);
        LOGGER_INFO("expandtexts saved to file successfully");
    }

    fclose(file); 
}

void __delete_empty_group(char ***groups, int row_count) {
    char *data_dir = data_get_dir();
    char *match_dir = path_join(data_dir, "matches");

    int file_count;
    char **files = path_listdir(match_dir, &file_count);

    for (int f=0; f < file_count; f++) {
        bool found = false;
        char *file = files[f];
        // Replace . from .csv to get the group name
        int final_dot = strlen(file) - 4;
        file[final_dot] = '\0';

        for (int r=1; r < row_count; r++) {
            if (str_eq(file, groups[r][0])) {
                found = true;
                break;
            }
        }

        file[final_dot] = '.';
        if (!found) {
            char *file_path = path_join(match_dir, file);
            remove(file_path);
            free(file_path);
        }
        free(file);
    }

    free(files);
    free(match_dir);
    free(data_dir);
}

void data_io_save() {
    int row_count, col_count;
    char ***groups = data_sql_get_raw("DISTINCT \"group\"", NULL, &row_count, &col_count);

    __delete_empty_group(groups, row_count);

    // free the column header
    free(groups[0][0]);
    free(groups[0]);

    for (int r=1; r < row_count; r++) {
        data_io_save_group(groups[r][0]);

        free(groups[r][0]);
        free(groups[r]);
    }
    free(groups);
}

char *__load_csv_table(CSVTable *table, const char *group) {
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
        row->group = strdup(group);

        if (row == NULL) {
            return error;
        }

        expandtext_add(row);
        data_sql_row_free(row);
    }

    return NULL;
}

void __load_group(const char *group) {
    char *save_path = __get_save_file(group);
    char *contents = path_read_all(save_path);
    free(save_path);

    if (contents == NULL)
        return;

    CSVTable *table = csv_read(contents);
    free(contents);

    if (table == NULL) {
        return;
    }
    char *error = __load_csv_table(table, group);
    csv_free(table);

    if (error != NULL) {
        LOGGER_ERROR(error);
        LOGGER_INFO("failed to load expandtexts from file");
        return;
    }

    LOGGER_INFO("expandtexts loaded from file successfully");
}

void data_io_load() {
    char *data_dir = data_get_dir();
    char *match_dir = path_join(data_dir, "matches");

    int count;
    char **files = path_listdir(match_dir, &count);

    for (int i = 0; i < count; i++) {
        char *match_file = files[i];
        char *dot = strrchr(match_file, '.');
        if (dot != NULL) {
            *dot = '\0';
            __load_group(match_file);
        }

        free(match_file);
    }

    free(files);
    free(data_dir);
    free(match_dir);
}
