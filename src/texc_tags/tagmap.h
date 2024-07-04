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
    char *(*tag_validate)(Tag *);

    // Returns Character(s) representation of tag
    // Is used in match tags only
    // @param 1 TagMap->data
    // @param 2 MatchSettings
    // @return the character(s) representation of the tag
    const char *(*tag_char)(void *, void *);

} TagMap;

/**
 * @brief Gets the TagMap of the given tag name
 *
 *
 * @param tag_name The name of the tag
 * @param is_match Boolean indicating if the tag belongs to match tags or expand
 * tags
 * @return The tagmap of the given tag. This function does not fail due to
 * tagmap_validate().
 */
const TagMap tagmap_get(const char *tag_name, bool is_match);

/**
 * @brief Checks if the given tag contains valid match or expand tags
 *
 * @param tag The root match/expand tag
 * @param is_match Boolean indicating if the tag belongs to match tags or expand
 * tags
 * @return error message
 */
char *tagmap_validate(Tag *tag, bool is_match);
