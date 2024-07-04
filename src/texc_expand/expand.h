#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../texc_tags/tags.h"

typedef struct {
    // Is the expand undoable to get original text
    bool is_undoable;
    // The amount of characters typed to perform the undo
    size_t typed_count;
} ExpandSettings;

/**
 * @brief Deletes the match and types the expand tag
 *
 * @param delete_len The length of the match text
 * @param tag The expand tag
 * @return settigns related to the expand
 */
ExpandSettings expand_text(size_t delete_len, Tag *tag);
