#pragma once

#include "texc_utils/logger.h"

#include <stdbool.h>

typedef struct {
    // General Settings
    LogLevel log_level;

    // Match Settings
    bool reset_on_enter;

    // Expand Settings
    int repeat_delay;

} Settings;

extern const Settings settings_default;

/**
 * @brief Saves the settings to a ini file
 */
void settings_save(const char *file_path, Settings settings);

/**
 * @brief Loads the settings from a ini file
 */
Settings settings_load(const char *file_path);
