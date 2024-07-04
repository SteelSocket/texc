#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t row_count;
    size_t col_count;
    char ***fields;
} CSVTable;

char *csv_next_field(const char **line);

char *csv_into_field(const char *str);

CSVTable *csv_read(const char *csv_str);

void csv_free(CSVTable *table);

#ifdef UTILS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#include "str.h"

char *csv_next_field(const char **line) {
    char *read = strdup(*line);
    char *write = read;

    if (*read == '\0') {
        free(read);
        return NULL;
    }

    bool in_dquotes = false;
    int i = 0;

    while (1) {
        // -------------------------
        // Handle Double quoutes
        // -------------------------
        if (*read == '"') {
            read++;
            if (*read != '"') {
                in_dquotes = !in_dquotes;
            }
            (*line)++;
        }
        // -------------------------

        if (*read == '\0') {
            write[i] = '\0';
            return write;
        }

        if (!in_dquotes) {
            if (*read == '\r') {
                if (*(read + 1) == '\n') {
                    write[i] = '\0';
                    (*line) += 3;
                    return write;
                }

            } else if (*read == ',' || *read == '\n') {
                write[i] = '\0';
                (*line)++;
                return write;
            }
        }

        write[i] = *(read++);
        i++;
        (*line)++;
    }

    free(read);
    return NULL;
}

char *csv_to_field(const char *str) {
    int dq_count = str_count(str, '"');
    char *parsed_str = malloc((strlen(str) + dq_count + 1) * sizeof(char));

    const char *read = str;
    bool should_surround = dq_count != 0;
    int i = 0;

    while (*read != '\0') {
        if (*read == ',' || *read == '\n' || *read == '\r') {
            should_surround = true;
        }

        if (*read == '"') {
            parsed_str[i++] = *read;
        }

        parsed_str[i++] = *(read++);
    }
    parsed_str[i] = '\0';

    char *field;
    if (!should_surround) {
        free(parsed_str);
        str_format(field, "%s", str);
        return field;
    }

    str_format(field, "\"%s\"", parsed_str);
    free(parsed_str);
    return field;
}

CSVTable *csv_read(const char *csv_str) {
    char *csv_copy = strdup(csv_str);

    char *save_ptr;
    char *line = strtok_r(csv_copy, "\n", &save_ptr);

    if (!line) {
        free(csv_copy);
        return NULL;
    }

    CSVTable *table = malloc(sizeof(CSVTable));
    table->row_count = 0;
    table->col_count = 0;

    table->fields = array_create(char **);
    table->fields[table->row_count++] = array_create(char *);

    while (1) {
        char *field = csv_next_field((const char **)&line);
        if (field == NULL)
            break;

        array_resize_add(table->fields[0], table->col_count, field, char *);
    }

    while ((line = strtok_r(NULL, "\n", &save_ptr))) {
        array_increase_size(table->fields, table->row_count, sizeof(char **));
        table->fields[table->row_count] =
            malloc(sizeof(char *) * table->col_count);

        for (int i = 0; i < table->col_count; i++) {
            char *field = csv_next_field((const char **)&line);
            table->fields[table->row_count][i] = field;
        }
        table->row_count++;
    }

    free(csv_copy);
    return table;
}

void csv_free(CSVTable *table) {
    for (int r = 0; r < table->row_count; r++) {
        for (int c = 0; c < table->col_count; c++) {
            free(table->fields[r][c]);
        }
        free(table->fields[r]);
    }
    free(table->fields);
    free(table);
}

#endif
