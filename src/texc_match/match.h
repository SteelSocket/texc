#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../texc_tags/tags.h"

typedef struct {
    // Boolean indicating if the match is successful
    bool ok;
    // The cursor offset from the final character
    int cursor;

    // Whether the match is undoable after the expand
    bool is_undoable;
    bool __is_casesensitive;

} MatchSettings;

/**
 * @brief Checks if the keybuffer matches the ExpandText->match
 *
 * @param match The match tag from ExpandText
 * @return The settings related to the match
 */
MatchSettings match_text(Tag *match);

/**
 * @brief Gets the match initializer of the match tag.
 * The match initializer is the last character of the match and is used
 * to filter out the correct matches in keyhook.c.
 *
 * @param match The source string of the match tag
 * @return string containing all the characters by which the match is initialized
 */
char *match_get_initializer(const char *match);
