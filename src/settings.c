#include "settings.h"
#include "ini.h"

#include "texc_utils/logger.h"
#include "texc_utils/str.h"

#include <stdio.h>

const Settings settings_default = {.rkey_delay = 120, .log_level = LOGGER_LEVEL_INFO};

#define IN_GENERAL_SECTION(sec, name, key) \
    str_eq(sec, "general_settings") && str_eq(name, key)
#define IN_EXPAND_SECTION(sec, name, key) \
    str_eq(sec, "expand_settings") && str_eq(name, key)

int __parse_log_level(const char* value) {
    if (str_eq(value, "INFO"))
        return LOGGER_LEVEL_INFO;
    else if (str_eq(value, "WARN"))
        return LOGGER_LEVEL_WARNING;
    else if (str_eq(value, "ERROR"))
        return LOGGER_LEVEL_ERROR;
    else
        return -1;
}

const char* __get_log_level(LogLevel level) {
    switch (level) {
        case LOGGER_LEVEL_ERROR:
            return "ERROR";
        case LOGGER_LEVEL_WARNING:
            return "WARNING";
        case LOGGER_LEVEL_INFO:
        default:
            return "INFO";
    }
}

static int __handle_ini(void* user, const char* section, const char* name,
                        const char* value) {
    Settings* settings = (Settings*)user;

    if (IN_EXPAND_SECTION(section, name, "repeat_key_delay")) {
        if (str_isnumber(value))
            settings->rkey_delay = atoi(value);
        else
            return 0;

    } else if (IN_GENERAL_SECTION(section, name, "log_level")) {
        int level = __parse_log_level(value);
        if (level != -1)
            settings->log_level = (LogLevel) level;
        else
            return 0;
    } else {
        return 0;
    }

    return 1;
}

void settings_save(const char* file_path, Settings settings) {
    FILE* file = fopen(file_path, "w");

    fprintf(file, "[general_settings]\n");
    fprintf(file, "log_level = %s\n", __get_log_level(settings.log_level));
    fprintf(file, "\n");

    fprintf(file, "[expand_settings]\n");
    fprintf(file, "repeat_key_delay = %d\n", settings.rkey_delay);

    fclose(file);
}

Settings settings_load(const char* file_path) {
    Settings settings = settings_default;
    int error = ini_parse(file_path, __handle_ini, &settings);

    if (error <= 0)
        // Overwrite the file with the default settings along with parsed values
        settings_save(file_path, settings);

#ifndef NDEBUG
    settings.log_level = LOGGER_LEVEL_INFO;
#endif

    return settings;
}
