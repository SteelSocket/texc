#include "settings.h"
#include "ini.h"

#include "texc_utils/array.h"
#include "texc_utils/logger.h"
#include "texc_utils/str.h"

#include <stdio.h>

const Settings settings_default = {
    .log_level = LOGGER_LEVEL_INFO,
    .reset_on_enter = true,
    .repeat_delay = 120,
};

#define FORMAT_ERROR(var, sec, name, msg) \
    str_format(var, "In section \"%s\" at key \"%s\" : %s", sec, name, msg)

int __parse_log_level(const char *value) {
    if (str_eq(value, "INFO"))
        return LOGGER_LEVEL_INFO;
    else if (str_eq(value, "WARN"))
        return LOGGER_LEVEL_WARNING;
    else if (str_eq(value, "ERROR"))
        return LOGGER_LEVEL_ERROR;
    else
        return -1;
}

const char *__get_log_level(LogLevel level) {
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

int __parse_bool_str(const char *value) {
    if (str_eq(value, "true"))
        return 1;
    else if (str_eq(value, "false"))
        return 0;
    else
        return -1;
}

const char *__get_bool_str(bool value) { return value ? "true" : "false"; }

typedef struct {
    Settings settings;

    size_t error_count;
    char **errors;

    bool overwrite_settings;

} __INISettings;

int __handle_internal(const char *key, const char *value,
                      __INISettings *isettings) {
    if (str_eq(key, "version")) {
        if (str_eq(value, TEXC_VERSION))
            isettings->overwrite_settings = false;
        return 1;
    }
    return 0;
}

int __handle_general(const char *key, const char *value,
                     __INISettings *isettings) {
    if (str_eq(key, "log_level")) {
        int level = __parse_log_level(value);
        if (level != -1) {
            isettings->settings.log_level = (LogLevel)level;
        } else {
            char *error;
            FORMAT_ERROR(error, "general_settings", key,
                         "The given value is not a valid log level");
            array_resize_add(isettings->errors, isettings->error_count, error,
                             char *);
            return 0;
        }
        return 1;
    }
    return 0;
}

int __handle_match(const char *key, const char *value,
                   __INISettings *isettings) {
    if (str_eq(key, "reset_on_enter")) {
        int bool_value = __parse_bool_str(value);
        if (bool_value == -1) {
            char *error;
            FORMAT_ERROR(error, "match_settings", key,
                         "The given value is not a boolean");
            array_resize_add(isettings->errors, isettings->error_count, error,
                             char *);

            return 0;
        }
        isettings->settings.reset_on_enter = bool_value;
        return 1;
    }
    return 0;
}

int __handle_expand(const char *key, const char *value,
                    __INISettings *isettings) {
    if (str_eq(key, "repeat_key_delay")) {
        if (str_isnumber(value) && atoi(value) >= 0) {
            isettings->settings.repeat_delay = atoi(value);
        } else {
            char *error;
            FORMAT_ERROR(error, "expand_settings", key,
                         "The given delay is not a positive integer");
            array_resize_add(isettings->errors, isettings->error_count, error,
                             char *);
            return 0;
        }
        return 1;
    }
    return 0;
}

static int __handle_ini(void *user, const char *section, const char *key,
                        const char *value) {
    __INISettings *isettings = (__INISettings *)user;

    char ***errors = &isettings->errors;

    if (str_eq(section, "__internal__")) {
        return __handle_internal(key, value, isettings);
    } else if (str_eq(section, "general_settings")) {
        return __handle_general(key, value, isettings);
    } else if (str_eq(section, "match_settings")) {
        return __handle_match(key, value, isettings);
    } else if (str_eq(section, "expand_settings")) {
        return __handle_expand(key, value, isettings);
    }

    char *error;
    FORMAT_ERROR(error, section, key, "The given key is not a valid setting");
    array_resize_add(*errors, isettings->error_count, error, char *);
    return 0;
}

void settings_save(const char *file_path, Settings settings) {
    FILE *file = fopen(file_path, "w");

    fprintf(file, "[__internal__]\n");
    fprintf(file, "version = %s\n", TEXC_VERSION);
    fprintf(file, "\n");

    fprintf(file, "[general_settings]\n");
    fprintf(file, "log_level = %s\n", __get_log_level(settings.log_level));
    fprintf(file, "\n");

    fprintf(file, "[match_settings]\n");
    fprintf(file, "reset_on_enter = %s\n",
            __get_bool_str(settings.reset_on_enter));
    fprintf(file, "\n");

    fprintf(file, "[expand_settings]\n");
    fprintf(file, "repeat_key_delay = %d\n", settings.repeat_delay);

    fclose(file);
}

Settings settings_load(const char *file_path) {
    __INISettings isettings = {
        .settings = settings_default,
        .error_count = 0,
        .errors = array_create(char **),
        .overwrite_settings = true,
    };
    ini_parse(file_path, __handle_ini, &isettings);
    Settings settings = isettings.settings;

#ifndef NDEBUG
    settings.log_level = LOGGER_LEVEL_INFO;
#endif
    logger_set_level(settings.log_level);

    if (isettings.overwrite_settings) {
        settings_save(file_path, settings);
        LOGGER_WARNING(
            "settings.ini version and texc version do not match. Overwriting "
            "settings.ini with given texc version's settings");
    }

    if (isettings.error_count) {
        for (size_t i = 0; i < isettings.error_count; i++) {
            LOGGER_WARNING(isettings.errors[i]);
            free(isettings.errors[i]);
        }
        LOGGER_ERROR(
            "Detected errors in loading settings.ini, using default values for "
            "invalid keys");
    }
    free(isettings.errors);

    return settings;
}
