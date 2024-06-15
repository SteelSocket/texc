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

void data_io_load() {
    char *save_path = __get_save_file();
    char *contents = path_read_all(save_path);
    free(save_path);

    if (contents == NULL)
        return;

    char *save_ptr;

    // Remove the first line which is the header section
    char *line = strtok_r(contents, "\n", &save_ptr);
    while ((line = strtok_r(NULL, "\n", &save_ptr))) {
        DataSqlRow *row = data_sql_row_from_csv((const char **)&line);
        char *error = expandtext_add(row);
        data_sql_row_free(row);

        if (error != NULL) {
            LOGGER_WARNING(error);
            free(error);
        }
    }

    LOGGER_INFO("expandtexts loaded from file successfully");
    free(contents);
}
