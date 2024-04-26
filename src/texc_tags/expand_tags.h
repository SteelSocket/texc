// Only included once in tagmap.c
// So all functions are implemented in the header
#pragma once

#include <stdint.h>

#include "../texc_expand/expand.h"
#include "../texc_expand/keyboard.h"

bool etag_press_char(void *data, void *settings) {
    ExpandSettings *ex_settings = (ExpandSettings *)settings;
    ex_settings->typed_count++;

    int keycode = keyboard_keycode_from_char(*((char *)data));
    keyboard_press(keycode);

    return true;
}

void etag_release_char(void *data, void *settings) {
    int keycode = keyboard_keycode_from_char(*((char *)data));
    keyboard_release(keycode);
}

bool etag_std_press(void *data, void *settings) {
    ExpandSettings *ex_settings = (ExpandSettings *)settings;
    ex_settings->is_undoable = false;
    ex_settings->typed_count++;

    keyboard_press((intptr_t)data);
    return true;
}

void etag_std_release(void *data, void *settings) {
    keyboard_release((intptr_t)data);
}
