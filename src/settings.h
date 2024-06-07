#pragma once

typedef struct {
    int rkey_delay;
} Settings;

Settings settings_init(const char *file_path);

Settings settings_load(const char *file_path);
