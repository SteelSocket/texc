#include "data_io.h"
#include "data.h"
#include "data_sql.h"

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

char *__expandtext_as_csv(ExpandText *exptext, DataSqlRow row) {
    char *csv_match = csv_to_field(exptext->match->tag_source);
    char *csv_expand = csv_to_field(exptext->expand->tag_source);

    char *ret;
    str_format(ret, "%s,%s,%zd\n", csv_match, csv_expand, row.id);

    free(csv_match);
    free(csv_expand);
    return ret;
}

char *data_io_expandtexts_as_csv(const char *select_condition) {
    int count;
    DataSqlRow *rows = data_sql_get(NULL, &count);
    if (rows == NULL)
        return NULL;

    char *csv_string;
    str_mcpy(csv_string, "match,expand,id\n");

    for (int i = 0; i < count; i++) {
        DataSqlRow row = rows[i];
        ExpandText *exptext = data.exptexts[row.index];
        char *csv_line = __expandtext_as_csv(exptext, row);

        str_rcat(csv_string, csv_line);

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
        char *match_src = csv_next_field((const char **)&line);
        char *expand_src = csv_next_field((const char **)&line);
        char *id_src = csv_next_field((const char **)&line);

        DataSqlRow attrs = (DataSqlRow){.id = atoi(id_src)};
        char *error = expandtext_add_from_src(match_src, expand_src, attrs);

        if (error != NULL) {
            LOGGER_WARNING(error);
            free(error);
        }

        free(match_src);
        free(expand_src);
        free(id_src);
    }

    LOGGER_INFO("expandtexts loaded from file successfully");
    free(contents);
}
