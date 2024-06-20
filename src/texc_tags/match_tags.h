// Only included once in tagmap.c
// So all functions are implemented in the header
#pragma once

#include <stdint.h>

#include "../texc_match/keybuffer.h"
#include "../texc_match/match.h"

bool mtag_key_check(void *data, void *settings) {
    MatchSettings *ma_settings = (MatchSettings *)settings;
    char *c = (char *)data;

    ma_settings->is_undoable = false;
    ma_settings->cursor--;
    if (ma_settings->cursor < 0) {
        ma_settings->ok = false;
        return false;
    }

    if (*c != keybuffer[ma_settings->cursor]) {
        ma_settings->ok = false;
    }

    return false;
}

const char *mtag_key_char(void *data, void *settings) {
    MatchSettings *ma_settings = (MatchSettings *)settings;
    return (char *)data;
}

bool mtag_tcase_enter(void *data, void *settings) {
    MatchSettings *ma_settings = (MatchSettings *)settings;
    ma_settings->is_casesensitive = !ma_settings->is_casesensitive;

    return true;
}

void mtag_tcase_exit(void *data, void *settings) {
    MatchSettings *ma_settings = (MatchSettings *)settings;
    ma_settings->is_casesensitive = !ma_settings->is_casesensitive;
}
