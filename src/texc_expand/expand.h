#pragma once

#include <stdbool.h>

#include "../texc_tags/tags.h"

typedef struct {
    // Is the expand undoable to get original text
    bool is_undoable;
    // The amount of characters typed to perform the undo
    int typed_count;
} ExpandSettings;

ExpandSettings expand_text(size_t delete_len, Tag *tag);
