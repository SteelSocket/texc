#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Tag {
    char *name;
    char *text;

    struct Tag **tags;
    size_t tags_len;
} Tag;

// ---------------------------------------------------------

/**
 * @brief Parse a string to a Tag *
 *
 * @param source The source string
 * @param tag_name The name of the root tag
 * @return The parsed tag or NULL if error
 */
Tag *tag_parse(const char *source, const char *tag_name);

// ---------------------------------------------------------

/**
 * @brief Frees a Tag* from memory
 */
void tag_free(Tag *tag);

// ---------------------------------------------------------
