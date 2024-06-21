#pragma once

#include <stdbool.h>

#include "../texc_match/match.h"

typedef struct {
    char character;
    int keycode;

    bool is_keydown;
    bool is_ctrldown;

} KeyEvent;

// Boolean representing if the keyhook is currently expanding
extern bool keyhook_is_expanding;

// Boolean representing whether to try to do a undo
extern bool keyhook_try_undo;
// String containing the last match string. Used for undo.
extern char *keyhook_last_source;
// The length of the expanded text. Used for undo
extern size_t keyhook_expand_len;

// The expand tag of the current match
extern Tag *keyhook_expand_tag;
// Boolean representing whether to try to do a expand based on current match
extern bool keyhook_try_expand;
// The settings of the current match
extern MatchSettings keyhook_match_settings;

// ------------------------------------------------------------------
// keyhook_raw_* functions are implemented in _keyhook_raw.h
// They are platform specific
// ------------------------------------------------------------------


/**
 * @brief Runs the raw keyhook
 */
void keyhook_raw_run();

/**
 * @brief Closes the raw keyhook
 */
void keyhook_raw_quit();

/**
 * @brief Handles the keydown event
 */
void keyhook_raw_handle_keydown(KeyEvent event);

/**
 * @brief Hadnles the keyup event
 */
void keyhook_raw_handle_keyup(KeyEvent event);

/**
 * @brief Handles the key event
 */
void keyhook_handle_event(KeyEvent event);


/**
 * @brief Checks if any of the ExpandTexts matches with the keybuffer
 * @return Boolean representing success
 */
bool keyhook_check_for_match(KeyEvent event);

/**
 * @brief Expands the matched ExpandText
 */
void keyhook_expand_matched();

/**
 * @brief Resets the undo on backspace feature
 */
void keyhook_reset_undo();

/**
 * @brief Clears the keybuffer and resets the undo
 */
void keyhook_reset();

/**
 * @brief Runs the keyhook (Blocking)
 */
void keyhook_run();
