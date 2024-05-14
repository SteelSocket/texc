#include "keyboard.h"

#include "_keyboard_raw.h"

bool keyboard_is_pressed(int keycode) {
#ifdef _WIN32
    return GetAsyncKeyState(keycode) < 0;
#else
#error "not implemented"
#endif
}

void keyboard_press(int keycode) { _keyboard_raw_press(keycode); }

void keyboard_release(int keycode) { _keyboard_raw_release(keycode); }

void keyboard_press_release(int keycode) {
    keyboard_press(keycode);
    keyboard_release(keycode);
}

int keyboard_keycode_from_char(char character) {
#ifdef _WIN32
    return __win_keycode_from_char(character);
#else
#error "Not implemented"
#endif
}

void keyboard_nomod_type(char character) { _keyboard_raw_type(character); }

void keyboard_nomod_type_string(const char *str) {
    const char *s = str;
    while (*s != '\0') {
        keyboard_nomod_type(*s);
        s++;
#ifdef _WIN32
        // Few apps like web browsers need this
        // when we type the same characters twice in a sequence
        // as they do not seem to register such rapid presses
        if (*(s - 1) == *s)
            Sleep(KEYBOARD_RKEYPRESS_TIMEOUT);
#endif
    }
}

void keyboard_backspace(int delete_count) {
    for (int i = 0; i < delete_count; i++) {
        keyboard_press_release(KEYBOARD_BACKSPACE);
    }
}
