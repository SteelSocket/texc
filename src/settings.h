#pragma once

#include "texc_utils/logger.h"

typedef struct {
    int rkey_delay;
    LogLevel log_level;

} Settings;

extern const Settings settings_default;

void settings_save(const char *file_path, Settings settings);

Settings settings_load(const char *file_path);
