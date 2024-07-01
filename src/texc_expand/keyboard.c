#include "keyboard.h"
#include "_keyboard_raw.h"

#include "../texc_data/data.h"

#ifndef _WIN32
#include <X11/XKBlib.h>
#endif

bool keyboard_is_pressed(KEYBOARD_KEYCODE keycode) {
#ifdef _WIN32
    return GetAsyncKeyState(keycode) < 0;
#else
    KeyCode xkeycode = XKeysymToKeycode(__keyhook_display, keycode);
    char keys[32] = {0};
    XQueryKeymap(__keyhook_display, keys);
    return keys[xkeycode / 8] & (1 << (xkeycode % 8));
#endif
}

bool keyboard_is_toggled(KEYBOARD_KEYCODE keycode) {
#ifdef _WIN32
    return GetKeyState(keycode) & 1;
#else
    unsigned int state;
    XkbGetIndicatorState(__keyhook_display, XkbUseCoreKbd, &state);

    switch (keycode) {
        case XK_Caps_Lock:
            return state & 1;
        case XK_Num_Lock:
            return state & 2;
        case XK_Scroll_Lock:
            return state & 4;
        default:
            return false;
    }
#endif
}

void keyboard_press(KEYBOARD_KEYCODE keycode) {
#ifdef _WIN32
    _keyboard_raw_press(MapVirtualKeyA(keycode, 0));
#else
    _keyboard_raw_press(XKeysymToKeycode(__keyhook_display, keycode));
#endif
}

void keyboard_release(KEYBOARD_KEYCODE keycode) {
#ifdef _WIN32
    _keyboard_raw_release(MapVirtualKeyA(keycode, 0));
#else
    _keyboard_raw_release(XKeysymToKeycode(__keyhook_display, keycode));
#endif
}

void keyboard_press_release(KEYBOARD_KEYCODE keycode) {
    keyboard_press(keycode);
    keyboard_release(keycode);
}

int keyboard_keycode_from_char(const char *character) {
#ifdef _WIN32
    return LOBYTE(VkKeyScan(*character));
#else
    return XStringToKeysym(character);
#endif
}

void keyboard_nomod_type(char character) { _keyboard_raw_type(character); }

void keyboard_nomod_type_string(const char *str) {
    const char *s = str;
    while (*s != '\0') {
        keyboard_nomod_type(*s);
        s++;
        // Few apps like web browsers need this
        // when we type the same characters twice in a sequence
        // as they do not seem to register such rapid presses
        if (*(s - 1) == *s)
#ifdef _WIN32
            Sleep(data.settings.repeat_delay);
#else
            usleep(data.settings.repeat_delay * 1000);
#endif
    }
}

void keyboard_backspace(int delete_count) {
    for (int i = 0; i < delete_count; i++) {
        keyboard_press_release(KEYBOARD_BACKSPACE);
    }
}
