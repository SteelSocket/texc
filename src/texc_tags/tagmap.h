#pragma once

#include <stdbool.h>

#include "tags.h"

typedef enum {
    // No option
    TAGMAP_FLAGS_NONE = 0,
    // If specified Tag must not contain children
    //
    // If unspecified the tag must contain children
    TAGMAP_FLAGS_STANDALONE = 1 << 0,

    // If specified Tag must contain text or no children
    //
    // If unspecified behaves as TAGMAP_FLAGS_NONE
    TAGMAP_FLAGS_TEXT_ONLY = 1 << 1,

} TagMapFlags;

typedef struct {
    const char *tag_name;
    TagMapFlags flags;
    void *data;

    // Called at the start of the tag
    // @param 1 TagMap->data
    // @param 2 ExpandSettings or MatchSettings
    // @return true if tag_exit method needs to be called
    bool (*tag_enter)(void *, void *);

    // Called at the end of the tag if tag_enter returns true
    // @param 1 TagMap->data
    // @param 2 ExpandSettings or MatchSettings
    void (*tag_exit)(void *, void *);

    // Validates the tag and returns the error (if any)
    // @param 1 the raw tag
    // @return error message (must be freed)
    char *(*validate)(Tag *);

} TagMap;

const TagMap tagmap_get(const char *tag_name, bool is_match);

char *tagmap_validate(Tag *tag, bool is_match);
