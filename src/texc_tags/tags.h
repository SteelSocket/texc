#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Tag {
    char *name;
    char *tag_source;
    char *text;

    struct Tag **tags;
    size_t tags_len;
} Tag;

char *__tag_source_minify(const char *s);

// ---------------------------------------------------------

Tag *tag_clone(Tag *tag);

// ---------------------------------------------------------

Tag *tag_get(Tag *tag, char *name);

Tag **tag_get_all(Tag *tag, char *name, int *count);

const char *tag_get_text(Tag *tag, char *name);

// ---------------------------------------------------------

void tag_insert(Tag *tag, Tag *child);

bool tag_insert_text(Tag *tag, const char *text);

Tag *tag_new(const char *name);

// ---------------------------------------------------------

Tag *tag_parse(const char *source, const char *tag_name);

char *tag_format(Tag *tag, bool format_root);

// ---------------------------------------------------------

void tag_free(Tag *tag);

void tag_print(Tag *tag);

// ---------------------------------------------------------
