#pragma once

char *csv_next_field(const char **line);

char *csv_to_field(const char *str);

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
#endif
