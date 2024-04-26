#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strtok_r strtok_s
#endif

#define str_eq(str1, str2) (strcmp(str1, str2) == 0)

#define str_format(string, format, ...)                                \
    do {                                                               \
        string = malloc((snprintf(NULL, 0, format, __VA_ARGS__) + 1) * \
                        sizeof(char));                                 \
        sprintf(string, format, __VA_ARGS__);                          \
    } while (0)

#define str_rformat(string, format, ...)                                \
    do {                                                                \
        string = realloc(                                               \
            string,                                                     \
            (snprintf(NULL, 0, "%s" format, string, __VA_ARGS__) + 1) * \
                sizeof(char));                                          \
        sprintf(string, "%s" format, string, __VA_ARGS__);              \
    } while (0)

#define str_mcpy(string, src)                              \
    do {                                                   \
        string = malloc((strlen(src) + 1) * sizeof(char)); \
        strcpy(string, src);                               \
    } while (0)

#define str_rcat(string, src)                                                \
    do {                                                                     \
        string = realloc(string,                                             \
                         (strlen(string) + strlen(src) + 1) * sizeof(char)); \
        strcat(string, src);                                                 \
    } while (0)

int str_count(const char *source, char c);

void str_strip(char *source, const char *delimeter);

bool str_isnumber(const char *source);

#ifdef UTILS_IMPLEMENTATION

int str_count(const char *source, char c) {
    int count = 0;
    while ((source = strchr(source, c)) != NULL) {
        count++;
        source++;
    }
    return count;
}

void str_strip(char *source, const char *delimeter) {
    char *dst = source;

    while (*source != '\0') {
        if (strchr(delimeter, *source) != NULL) {
            source++;
            continue;
        }
        *dst = *source;
        dst++;
        source++;
    }
    *dst = '\0';
}

bool str_isnumber(const char *source) {
    if (*source == '\0')
        return false;

    if (*source == '-') {
        source++;
        if (*source == '\0')
            return false;
    }

    while (*source != '\0') {
        if (!isdigit(*source))
            return false;
        source++;
    }

    return true;
}

#endif
