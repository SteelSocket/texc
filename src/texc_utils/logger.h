#pragma once

#include "str.h"

typedef enum {
    LOGGER_LEVEL_INFO,
    LOGGER_LEVEL_WARNING,
    LOGGER_LEVEL_ERROR,
} LogLevel;

void logger_init(const char *log_file);

void logger_set_level(LogLevel level);

#ifndef __FUNCTION_NAME__
#ifdef _WIN32
#define __FUNCTION_NAME__ __FUNCTION__
#else
#define __FUNCTION_NAME__ __func__
#endif
#endif

#ifndef __FILE_NAME__
#ifdef _WIN32
#define __FILE_NAME__ \
    (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILE_NAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#endif

void __logger_log(const char *message, const char *file_name, int line_no,
                  const char *func_name, LogLevel level);

#define LOGGER_FORMAT_LOG(LOGGER_MACRO, message, ...) \
    do {                                              \
        char *_f;                                     \
        str_format(_f, message, __VA_ARGS__);         \
        LOGGER_MACRO(_f);                             \
        free(_f);                                     \
    } while (0)

#define LOGGER_INFO(message)                                          \
    __logger_log(message, __FILE_NAME__, __LINE__, __FUNCTION_NAME__, \
                 LOGGER_LEVEL_INFO)
#define LOGGER_ERROR(message)                                         \
    __logger_log(message, __FILE_NAME__, __LINE__, __FUNCTION_NAME__, \
                 LOGGER_LEVEL_ERROR)
#define LOGGER_WARNING(message)                                       \
    __logger_log(message, __FILE_NAME__, __LINE__, __FUNCTION_NAME__, \
                 LOGGER_LEVEL_WARNING)

#ifdef UTILS_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static FILE *__log_file;
static bool __is_in_background;
static LogLevel __logger_level = LOGGER_LEVEL_INFO;

void logger_init(const char *log_file) {
    if (log_file == NULL) {
        __log_file = NULL;
        return;
    }

    __log_file = fopen(log_file, "w");
#ifdef _WIN32
    __is_in_background = getenv("TEXC_BACKGROUND") != NULL;
#else
    pid_t this_pgid = getpgrp();
    pid_t f_pgid = tcgetpgrp(STDOUT_FILENO);  // Get foreground process group ID
    __is_in_background = this_pgid != f_pgid;
#endif
}

void logger_set_level(LogLevel level) { __logger_level = level; }

void __logger_log(const char *message, const char *file_name, int line_no,
                  const char *func_name, LogLevel level) {
    if (__logger_level > level)
        return;

    char *log_level;
    switch (level) {
        case LOGGER_LEVEL_INFO: {
            log_level = "INFO";
            break;
        }
        case LOGGER_LEVEL_WARNING: {
            log_level = "WARNING";
            break;
        }
        case LOGGER_LEVEL_ERROR: {
            log_level = "ERROR";
            break;
        }
    }

    time_t raw_time;
    time(&raw_time);
    struct tm *time_info = localtime(&raw_time);

    char log_time[26];
    strftime(log_time, 26, "%H:%M:%S", time_info);

    if (!__is_in_background)
        printf("%s: [%s] %s:%d (%s) -> \"%s\"\n", log_level, log_time,
               file_name, line_no, func_name, message);

    if (__log_file != NULL)
        fprintf(__log_file, "%s: [%s] %s:%d (%s) -> \"%s\"\n", log_level,
                log_time, file_name, line_no, func_name, message);

    fflush(stdout);
}
#endif
