// Only included once in keyboard.c
// So all functions are implemented in the header
#pragma once

#include "keyboard.h"

#ifdef _WIN32

#include <windows.h>
#include <winuser.h>

INPUT __win_create_input(int wscan, DWORD flags) {
    INPUT input;

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = wscan;
    input.ki.dwFlags = flags;
    input.ki.time = 0;

    return input;
}

void _keyboard_raw_press(KEYBOARD_RAW_KEYCODE scan_code) {
    INPUT input =
        __win_create_input(scan_code, KEYEVENTF_SCANCODE);
    SendInput(1, &input, sizeof(INPUT));
}

void _keyboard_raw_release(KEYBOARD_RAW_KEYCODE scan_code) {
    INPUT input = __win_create_input(scan_code,
                                     KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
    SendInput(1, &input, sizeof(INPUT));
}

void _keyboard_raw_type(char c) {
    INPUT input = __win_create_input(c, KEYEVENTF_UNICODE);
    SendInput(1, &input, sizeof(INPUT));
}

#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Inlcude keyhook.h for Display *__keyhook_display;
#include "../texc_keyhook/keyhook.h"

void _keyboard_raw_press(KEYBOARD_RAW_KEYCODE key_code) {
    XTestFakeKeyEvent(__keyhook_display, key_code, True, 0);
    XFlush(__keyhook_display);
}

void _keyboard_raw_release(KEYBOARD_RAW_KEYCODE key_code) {
    XTestFakeKeyEvent(__keyhook_display, key_code, False, 0);
    XFlush(__keyhook_display);
}

const char __sym_map[255] = {
    ['`'] = '`',   ['-'] = '-',  ['='] = '=',   ['['] = '[',  [']'] = ']',
    ['\\'] = '\\', [';'] = ';',  ['\''] = '\'', [','] = ',',  ['.'] = '.',
    ['/'] = '/',

    [' '] = ' ',   ['~'] = '`',  ['!'] = '1',   ['@'] = '2',  ['#'] = '3',
    ['$'] = '4',   ['%'] = '5',  ['^'] = '6',   ['&'] = '7',  ['*'] = '8',
    ['('] = '9',   [')'] = '0',  ['_'] = '-',   ['+'] = '=',  ['{'] = '[',
    ['}'] = ']',   ['|'] = '\\', [':'] = ';',   ['"'] = '\'', ['<'] = ',',
    ['>'] = '.',   ['?'] = '/',
};

void __kb_release_modifier(char (*keys)[32], KeySym sym) {
    KeyCode xkeycode = XKeysymToKeycode(__keyhook_display, sym);

    if (keyboard_is_pressed(sym)) {
        keyboard_release(sym);
        (*keys)[xkeycode / 8] |= (1 << (xkeycode % 8));
    }
}

void __kb_release_modifiers(char (*keys)[32]) {
    __kb_release_modifier(keys, XK_Shift_L);
    __kb_release_modifier(keys, XK_Shift_R);
    __kb_release_modifier(keys, XK_Control_L);
    __kb_release_modifier(keys, XK_Control_R);
    __kb_release_modifier(keys, XK_Alt_L);
    __kb_release_modifier(keys, XK_Alt_R);
    __kb_release_modifier(keys, XK_Super_L);
    __kb_release_modifier(keys, XK_Super_R);
}

void __kb_reapply_modifier(char keys[32], KeySym sym) {
    KeyCode xkeycode = XKeysymToKeycode(__keyhook_display, sym);
    int reapplied = keys[xkeycode / 8] & (1 << (xkeycode % 8));

    if (reapplied) {
        keyboard_press(sym);
    }
}

void __kb_reapply_modifiers(char keys[32]) {
    __kb_reapply_modifier(keys, XK_Shift_L);
    __kb_reapply_modifier(keys, XK_Shift_R);
    __kb_reapply_modifier(keys, XK_Control_L);
    __kb_reapply_modifier(keys, XK_Control_R);
    __kb_reapply_modifier(keys, XK_Alt_L);
    __kb_reapply_modifier(keys, XK_Alt_R);
    __kb_reapply_modifier(keys, XK_Super_L);
    __kb_reapply_modifier(keys, XK_Super_R);
}

void _keyboard_raw_type(char c) {
    char keysym_char = c;
    bool should_shift = false;

    bool shifted = keyboard_is_pressed(KEYBOARD_SHIFT);
    bool caps_lock = keyboard_is_toggled(KEYBOARD_CAPS_LOCK);

    if (isalpha(c)) {
        keysym_char = tolower(c);
        should_shift = (isupper(c) && !caps_lock) || (islower(c) && caps_lock);
    } else if (!isalnum(c)) {
        keysym_char = __sym_map[(int)c];
        if (keysym_char != c)
            should_shift = true;
    }

    char *unicode = (char *)malloc(8 * sizeof(char));
    sprintf(unicode, "U%04x", keysym_char);
    KeySym sym = XStringToKeysym(unicode);
    free(unicode);

    char keys[32] = {0};

    __kb_release_modifiers(&keys);
    if (should_shift) {
        keyboard_press(KEYBOARD_SHIFT);
    } else if (shifted) {
        keyboard_release(KEYBOARD_SHIFT);
    }

    keyboard_press_release(sym);

    if (should_shift) {
        keyboard_release(KEYBOARD_SHIFT);
    }
    __kb_reapply_modifiers(keys);
}
#endif
