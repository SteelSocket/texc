#include "settings.h"
#include "ini.h"

#include "texc_utils/str.h"

#include <stdio.h>

const Settings __default_settings = {.rkey_delay = 120};

static int __handle_ini(void* user, const char* section, const char* name,
                        const char* value) {
    Settings *settings = (Settings *)value;

    if (str_eq(section, "expand_settings") && str_eq(name, "repeat_key_delay")) {
        settings->rkey_delay = atoi(value);
    }

    return 1;
}

Settings settings_init(const char *file_path) {
    FILE *file = fopen(file_path, "w");

    fprintf(file, "[expand_settings]\n");
    fprintf(file, "repeat_key_delay = %d\n", __default_settings.rkey_delay);

    fclose(file);

    return __default_settings;
}

Settings settings_load(const char* file_path) {
    Settings settings = __default_settings;
    ini_parse(file_path, __handle_ini, &settings);

    return settings;
}
