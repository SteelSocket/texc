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
#define KEYBOARD_NUMLOCK VK_NUMLOCK
#define KEYBOARD_CAPS_LOCK VK_CAPITAL

#define KEYBOARD_KEYCODE UINT
#define KEYBOARD_RAW_KEYCODE UINT
#else
#include <X11/X.h>
#include <X11/keysym.h>

#define KEYBOARD_BACKSPACE XK_BackSpace
#define KEYBOARD_TAB XK_Tab
#define KEYBOARD_RETURN XK_Return

#define KEYBOARD_LEFT_ARROW XK_Left
#define KEYBOARD_RIGHT_ARROW XK_Right
#define KEYBOARD_UP_ARROW XK_Up
#define KEYBOARD_DOWN_ARROW XK_Down

#define KEYBOARD_SHIFT XK_Shift_L
#define KEYBOARD_CONTROL XK_Control_L
#define KEYBOARD_NUMLOCK XK_Num_Lock
#define KEYBOARD_CAPS_LOCK XK_Caps_Lock

#define KEYBOARD_KEYCODE KeySym
#define KEYBOARD_RAW_KEYCODE KeyCode
#endif

#include <stdbool.h>

// _keyboard_raw_* functions are implemented in _keyboard_raw.h

/**
 * @brief Presses the raw keycode as per the platform
 *
 * @param keycode The platform specific keycode
 */
void _keyboard_raw_press(KEYBOARD_RAW_KEYCODE keycode);

/**
 * @brief Releases the raw keycode as per the platform
 *
 * @param keycode The platform specific keycode
 */
void _keyboard_raw_release(KEYBOARD_RAW_KEYCODE keycode);

/**
 * @brief Types the character as per the platform
 *
 * @param c character to type
 */
void _keyboard_raw_type(char c);

/**
 * @brief Checks if a key is pressed
 *
 * @param keycode KEYBOARD_* keycode
 * @return Boolean indicating if the key is pressed
 */
bool keyboard_is_pressed(KEYBOARD_KEYCODE keycode);

/**
 * @brief Checks if a key is toggled
 *
 * @param keycode KEYBOARD_* keycode
 * @return Boolean indicating if the key is toggled or not
 */
bool keyboard_is_toggled(KEYBOARD_KEYCODE keycode);

/**
 * @brief Presses a key
 *
 * @param keycode KEYBOARD_* keycode
 */
void keyboard_press(KEYBOARD_KEYCODE keycode);

/**
 * @brief Releases a key
 *
 * @param keycode KEYBOARD_* keycode
 */
void keyboard_release(KEYBOARD_KEYCODE keycode);

/**
 * @brief Presses and Releases a key
 *
 * @param keycode KEYBOARD_* keycode
 */
void keyboard_press_release(KEYBOARD_KEYCODE keycode);

/**
 * @brief Returns the keycode of given char
 *
 * @param character A string with a single character and null terminator
 * @return keycode
 */
int keyboard_keycode_from_char(const char *character);

/**
 * @brief Types a character without it being affected by the modifier keys
 *
 * @param character The character to type
 */
void keyboard_nomod_type(char character);

/**
 * @brief Types a string without it being affected by the modifier keys
 *
 * @param str The string to type
 */
void keyboard_nomod_type_string(const char *str);

/**
 * @brief Presses and Releases backspace as per parameter
 *
 * @param delete_count The amount of backspaces to press
 */
void keyboard_backspace(int delete_count);
