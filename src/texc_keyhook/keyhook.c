#include "keyhook.h"
#include "_keyhook_raw.h"

#include <stdlib.h>

#include "../texc_data/data.h"
#include "../texc_utils/logger.h"

#include "../texc_expand/keyboard.h"
#include "../texc_match/keybuffer.h"

#include "../texc_expand/expand.h"
#include "../texc_match/match.h"

bool keyhook_is_expanding = false;

// --------------------------------
// Undo Variables
// --------------------------------
bool keyhook_try_undo = false;
char *keyhook_last_source = NULL;
int keyhook_expand_len = 0;

// --------------------------------
// Expand Variables
// --------------------------------
bool keyhook_try_expand = false;
MatchSettings keyhook_match_settings = {0};
Tag *keyhook_expand_tag = NULL;

void keyhook_expand_matched() {
    while (keybuffer_cursor_move(-1)) {
        keyboard_press_release(KEYBOARD_RIGHT_ARROW);
#ifdef _WIN32
        Sleep(data.settings.rkey_delay);
#endif
    }

    keyhook_is_expanding = true;
    int delete_count = keybuffer_size - keyhook_match_settings.cursor;

    //
    // For undo
    //
    char *last_source = malloc((delete_count + 1) * sizeof(char));
    memcpy(last_source, keybuffer + keyhook_match_settings.cursor,
           delete_count);
    last_source[delete_count] = '\0';

    ExpandSettings expand_settings =
        expand_text(delete_count, keyhook_expand_tag);

    if (expand_settings.is_undoable && keyhook_match_settings.is_undoable) {
        keyhook_expand_len = expand_settings.typed_count;
        keyhook_last_source = last_source;
    } else {
        free(last_source);
    }

    keyhook_is_expanding = false;
    keyhook_match_settings = (MatchSettings){0};
    keyhook_expand_tag = NULL;
}

bool keyhook_check_for_match(KeyEvent event) {
    int count;
    DataSqlRow *rows = data_sql_get("enabled = 1", &count);

    if (rows == NULL)
        return false;
    if (count == 0) {
        free(rows);
        return false;
    }

    for (int i = 0; i < count; i++) {
        ExpandText *exptext = data.exptexts[rows[i].index];

        Tag *match = exptext->match;
        Tag *expand = exptext->expand;

        MatchSettings match_settings = match_text(match);
        if (match_settings.ok && !keyhook_is_expanding) {
            keyhook_try_expand = true;
            keyhook_match_settings = match_settings;
            keyhook_expand_tag = expand;
            free(rows);
            return true;
        }
    }

    free(rows);
    return false;
}

void keyhook_handle_event(KeyEvent event) {
    if (event.character && !event.is_ctrldown) {
        if (event.is_keydown) {
            keyhook_raw_handle_keydown(event);
        } else {
            keyhook_raw_handle_keyup(event);
        }
        return;
    }

    // Ctrl + A, Ctrl + Z, Ctrl + ...
    if (event.character && event.is_ctrldown) {
        keyhook_reset();
        return;
    }

    // Left arrow
    if (keyboard_is_pressed(KEYBOARD_LEFT_ARROW)) {
        if (event.is_ctrldown) {
            keyhook_reset();
            return;
        }
        keybuffer_cursor_move(1);
        keyhook_reset_undo();
        return;
    }

    // Right arrow
    if (keyboard_is_pressed(KEYBOARD_RIGHT_ARROW)) {
        if (event.is_ctrldown) {
            keyhook_reset();
            return;
        }
        keybuffer_cursor_move(-1);
        keyhook_reset_undo();
        return;
    }

    // Up and Down arrow
    if (keyboard_is_pressed(KEYBOARD_DOWN_ARROW) ||
        keyboard_is_pressed(KEYBOARD_UP_ARROW)) {
        keyhook_reset();
        return;
    }
}

void keyhook_reset_undo() {
    if (keyhook_last_source != NULL) {
        free(keyhook_last_source);
        keyhook_last_source = NULL;
    }
    keyhook_expand_len = 0;
}

void keyhook_reset() {
    keyhook_reset_undo();
    keybuffer_clear();
}

void keyhook_run() {
    LOGGER_INFO("keyhook started");
    keyhook_reset();
    keyhook_raw_run();
    LOGGER_INFO("keyhook quit");
}
