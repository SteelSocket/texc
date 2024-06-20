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

Tag *tag_parse(const char *source, const char *tag_name);

// ---------------------------------------------------------

void tag_free(Tag *tag);

// ---------------------------------------------------------
