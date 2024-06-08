#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../texc_tags/tags.h"

typedef struct {
    bool ok;
    int cursor;

    bool is_undoable;
    bool is_casesensitive;

} MatchSettings;

MatchSettings match_text(Tag *match);

char *match_get_initializer(Tag *match);
