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

#else
#error "Not implemented"
#endif

#include <stdbool.h>

/**
 * @brief Checks if a key is pressed
 *
 * @param keycode KEYBOARD_* keycode
 * @return Boolean indicating if the key is pressed
 */
bool keyboard_is_pressed(int keycode);

/**
 * @brief Checks if a key is toggled
 *
 * @param keycode KEYBOARD_* keycode
 * @return Boolean indicating if the key is toggled or not
 */
bool keyboard_is_toggled(int keycode);

/**
 * @brief Presses a key
 *
 * @param keycode KEYBOARD_* keycode
 */
void keyboard_press(int keycode);

/**
 * @brief Releases a key
 *
 * @param keycode KEYBOARD_* keycode
 */
void keyboard_release(int keycode);

/**
 * @brief Presses and Releases a key
 *
 * @param keycode KEYBOARD_* keycode
 */
void keyboard_press_release(int keycode);

/**
 * @brief Returns the keycode of given char
 *
 * @param character The character to convert to keycode
 * @return keycode
 */
int keyboard_keycode_from_char(char character);

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
