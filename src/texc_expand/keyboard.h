#pragma once

#ifdef _WIN32
#include <windows.h>

#define KEYBOARD_BACKSPACE VK_BACK
#define KEYBOARD_TAB VK_TAB
#define KEYBOARD_RETURN VK_RETURN

#define KEYBOARD_LEFT_ARROW VK_LEFT
#define KEYBOARD_RIGHT_ARROW VK_RIGHT
#define KEYBOARD_UP_ARROW VK_UP
#define KEYBOARD_DOWN_ARROW VK_DOWN

#define KEYBOARD_SHIFT VK_SHIFT
#define KEYBOARD_CONTROL VK_CONTROL

#else
#error "Not implemented"
#endif

#include "../texc_tags/tags.h"

bool keyboard_is_pressed(int keycode);

void keyboard_press(int keycode);

void keyboard_release(int keycode);

void keyboard_press_release(int keycode);

int keyboard_keycode_from_char(char character);

void keyboard_nomod_type(char character);

void keyboard_nomod_type_string(const char *str);

void keyboard_backspace(int delete_count);
