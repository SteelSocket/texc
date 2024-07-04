#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern char *keybuffer;

extern size_t keybuffer_size;
/**
 * The offset from the last character of the keybuffer
 * to where the current insert cursor is.
 */
extern size_t keybuffer_cursor;

/**
 * @brief Prints the keybuffer
 */
void keybuffer_print();

/**
 * @brief Moves the keybuffer_cursor
 *
 * @param offset The offset amount. + if the cursor is moved left, - if the
 * cursor is moved right
 * @return Boolean indicating success
 */
bool keybuffer_cursor_move(int offset);

/**
 * @brief Clears all characters in keybuffer and resets the keybuffer_cursor
 */
void keybuffer_clear();

/**
 * @brief Pushes a single character to the keybuffer
 *
 * @param c The character to push
 */
void keybuffer_push(char c);

/**
 * @brief Pops the character before the keybuffer_cursor
 */
void keybuffer_pop();

/**
 * @brief Appends a string to the keybuffer
 *
 * @param str The string to append with the keybuffer
 */
void keybuffer_append(char *str);

/**
 * @brief Replaces the keybuffer with the given string
 *
 * @param pop_len The number of characters to pop
 * @param rstr The replace string
 */
void keybuffer_replace(size_t pop_len, char *rstr);
