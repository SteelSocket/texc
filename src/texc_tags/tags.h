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

char *__tag_source_minify(const char *s);

// ---------------------------------------------------------

Tag *tag_clone(Tag *tag);

// ---------------------------------------------------------

void tag_insert(Tag *tag, Tag *child);

Tag *tag_new(const char *name);

// ---------------------------------------------------------

Tag *tag_parse(const char *source, const char *tag_name);

// ---------------------------------------------------------

void tag_free(Tag *tag);

// ---------------------------------------------------------
