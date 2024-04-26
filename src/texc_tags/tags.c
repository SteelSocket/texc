#include "tags.h"

#include "../texc_utils/array.h"
#include "../texc_utils/str.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------

Tag *tag_clone(Tag *tag) {
    Tag *ctag = malloc(sizeof(Tag));

    ctag->name = strdup(tag->name);
    if (tag->text != NULL)
        ctag->text = strdup(tag->text);
    else
        ctag->text = NULL;

    if (tag->tag_source != NULL)
        ctag->tag_source = strdup(tag->tag_source);
    else
        ctag->tag_source = NULL;

    ctag->tags_len = tag->tags_len;
    ctag->tags = NULL;
    if (tag->tags_len) {
        ctag->tags = malloc(ctag->tags_len * sizeof(Tag *));
        for (size_t i = 0; i < tag->tags_len; i++) {
            ctag->tags[i] = tag_clone(tag->tags[i]);
        }
    }

    return ctag;
}

// ---------------------------------------------------------

Tag *tag_get(Tag *tag, char *name) {
    if (str_eq(tag->name, name)) {
        return tag;
    }

    for (size_t i = 0; i < tag->tags_len; i++) {
        Tag *rtag = tag_get(tag->tags[i], name);
        if (rtag != NULL)
            return rtag;
    }
    return NULL;
}

void __tag_get_all(Tag *tag, char *name, int *count, Tag ***array) {
    if (str_eq(tag->name, name)) {
        array_resize_add(*array, *count, tag, Tag *);
    }

    for (size_t i = 0; i < tag->tags_len; i++) {
        __tag_get_all(tag->tags[i], name, count, array);
    }
}

Tag **tag_get_all(Tag *tag, char *name, int *count) {
    Tag **tag_array = array_create(Tag *);
    *count = 0;

    __tag_get_all(tag, name, count, &tag_array);

    if (*count == 0) {
        free(tag_array);
        return NULL;
    }
    return tag_array;
}

const char *tag_get_text(Tag *tag, char *name) {
    Tag *ctag = tag_get(tag, name);
    if (ctag == NULL)
        return NULL;
    if (ctag->text == NULL)
        return NULL;
    return ctag->text;
}

// ---------------------------------------------------------

char *__tokenize(char *src, char delimeter, char **save) {
    if (src == NULL) {
        if (*save == NULL || **save == '\0')
            return NULL;
        src = *save;
    }

    char *start = strchr(src, delimeter);

    while (start != NULL) {
        if (*(start + 1) != delimeter) {
            *start = '\0';
            start++;

            *save = start;
            return src;
        }
        start = strchr(start + 2, delimeter);
    }
    *save = NULL;
    return src;
}

// ---------------------------------------------------------

char *__tag_brackets_strip(const char *text) {
    char *read = strdup(text);
    char *write = read;
    char last_ignored = '\0';

    while (*text != '\0') {
        if (!last_ignored && (*text == '<' || *text == '>')) {
            last_ignored = *text;
            text++;
            continue;
        } else if (last_ignored) {
            if (last_ignored != *text) {
                last_ignored = '\0';
                continue;
            }
            last_ignored = '\0';
        }

        *(write++) = *(text++);
    }
    *write = '\0';
    return read;
}

void __tag_insert_raw(Tag *tag, const char *name, const char *text) {
    char *ctext = (text != NULL) ? __tag_brackets_strip(text) : NULL;

    Tag *ctag = malloc(sizeof(Tag));

    ctag->name = strdup(name);
    ctag->text = ctext;
    ctag->tag_source = NULL;

    ctag->tags = NULL;
    ctag->tags_len = 0;

    tag_insert(tag, ctag);
}

void __tag_insert_text(Tag *tag, const char *text) {
    __tag_insert_raw(tag, "text", text);
    tag->tags[tag->tags_len - 1]->tag_source = strdup(text);
}

// ---------------------------------------------------------

bool __is_whitespace(char c) {
    return (c == ' ' || c == '\r' || c == '\n' || c == '\t');
}

char *__tag_source_minify(const char *s) {
    char *minified = malloc(sizeof(char) * (strlen(s) + 1));
    size_t index = 0;

    bool inside_tag = false;
    bool text_started = false;

    size_t lineno = 0;
    int tag_start_lineno = -1;
    int text_start_lineno = -1;

    bool last_insert_text = false;

    while (*s != '\0') {
        char c = *(s++);
        bool whitespace = __is_whitespace(c);

        if (c == '\n') {
            if (text_started) {
                text_started = false;
                continue;
            }
            lineno++;
        }

        if ((c == '<' || c == '>') && (c == *s)) {
            minified[index++] = c;
            minified[index++] = *(s++);
            continue;
        }

        if (!inside_tag) {
            if (whitespace && (tag_start_lineno != lineno && !text_started ||
                               text_start_lineno != lineno))
                continue;

            if (c == '<') {
                inside_tag = true;
                text_started = false;
                last_insert_text = false;
                tag_start_lineno = lineno;
            } else {
                if (last_insert_text && !text_started) {
                    minified[index++] = ' ';
                }
                last_insert_text = true;
                text_started = true;
                text_start_lineno = lineno;
            }

            minified[index++] = c;

        } else {
            if (c == '>') {
                inside_tag = false;
            }

            minified[index++] = c;
        }
    }

    minified[index++] = '\0';
    return minified;
}

// ---------------------------------------------------------

void __tag_parse_tag(Tag *tag, const char *name, char **save) {
    char *end_name;
    str_format(end_name, "</%s>", name);

    char *end_tag = strstr(*save, end_name);

    if (end_tag == NULL) {
        free(end_name);
        __tag_insert_raw(tag, name, NULL);
        return;
    }
    *end_tag = '\0';

    Tag *ctag = tag_parse(*save, name);
    if (ctag != NULL) {
        tag_insert(tag, ctag);
    }

    *end_tag = '<';
    *save += (end_tag - *save) + (strlen(end_name));

    free(end_name);
}

Tag *__tag_parse(size_t max_len, const char *raw_str, const char *tag_name) {
    char *copy_str = __tag_source_minify(raw_str);

    Tag *tag = malloc(sizeof(Tag));
    tag->name = (tag_name == NULL) ? strdup("root") : strdup(tag_name);
    tag->text = NULL;

    tag->tag_source = strdup(copy_str);
    tag->tags = array_create(Tag *);
    tag->tags_len = 0;

    char *save;
    char *before_tag = __tokenize(copy_str, '<', &save);

    while (before_tag != NULL) {
        if (strlen(before_tag)) {
            __tag_insert_text(tag, before_tag);
        }

        char *tag_name = __tokenize(NULL, '>', &save);

        if (tag_name == NULL || str_eq(tag_name, "")) {
            break;
        }
        str_strip(tag_name, "\n\t\r");

        if (!*tag_name == '\0' && *tag_name != '/') {
            if (save == NULL) {
                tag_free(tag);
                return NULL;
            }
            __tag_parse_tag(tag, tag_name, &save);
        } else {
            if (save == NULL)
                break;
        }

        before_tag = __tokenize(NULL, '<', &save);
    }

    free(copy_str);
    return tag;
}

Tag *tag_parse(const char *source, const char *tag_name) {
    char *striped = __tag_brackets_strip(source);
    size_t max_len = str_count(source, '<') + 2 - (str_count(striped, '<') * 2);
    free(striped);

    if (max_len == 2) {
        Tag *tag = malloc(sizeof(Tag));

        tag->tag_source = strdup(source);
        tag->name = (tag_name == NULL) ? strdup("text") : strdup(tag_name);
        tag->text = __tag_brackets_strip(source);

        tag->tags = NULL;
        tag->tags_len = 0;

        return tag;
    }
    return __tag_parse(max_len, source, tag_name);
}

// ---------------------------------------------------------

void __add_indent(char **s, int indent) {
    if (indent <= 0)
        return;
    *s = realloc(*s, strlen(*s) + 1 + (indent * 4));
    for (size_t i = 0; i < indent * 4; i++) {
        strcat(*s, " ");
    }
}

void __tag_format(Tag *tag, char **s, int depth) {
    bool closing_tag = false;
    bool same_line_format = (tag->text != NULL)
                                ? strlen(tag->text) < 30 && tag->tags_len == 0
                                : false;

    __add_indent(s, depth);

    if (!str_eq(tag->name, "text")) {
        if (same_line_format) {
            str_rformat(*s, "<%s>", tag->name);
        } else {
            str_rformat(*s, "<%s>\n", tag->name);
        }
    }

    if (tag->text != NULL) {
        // Use the raw text contained in tag->tag_source
        // Instead of the parsed tag->text

        if (same_line_format) {
            str_rformat(*s, "%s", tag->tag_source);
        } else {
            __add_indent(s, depth + 1);
            str_rformat(*s, "%s\n", tag->tag_source);
        }

        closing_tag = true;
    }

    if (tag->tags_len) {
        closing_tag = true;
        for (size_t i = 0; i < tag->tags_len; i++) {
            __tag_format(tag->tags[i], s, depth + 1);
        }
    }

    if (closing_tag) {
        if (!same_line_format)
            __add_indent(s, depth);
        if (!str_eq(tag->name, "text"))
            str_rformat(*s, "</%s>\n", tag->name);
        else
            str_rcat(*s, "\n");
    }
}

char *tag_format(Tag *tag, bool format_root) {
    char *str = malloc(1);
    str[0] = '\0';

    if (!format_root) {
        for (size_t i = 0; i < tag->tags_len; i++) {
            __tag_format(tag->tags[i], &str, 0);
        }
    } else {
        __tag_format(tag, &str, 0);
    }

    str[strlen(str)] = '\0';

    return str;
}

// ---------------------------------------------------------

void tag_insert(Tag *tag, Tag *child) {
    if (tag->tags == NULL)
        tag->tags = array_create(Tag *);

    array_resize_add(tag->tags, tag->tags_len, child, Tag *);
}

bool tag_insert_text(Tag *tag, const char *text) {
    if (tag->tags != NULL)
        return false;

    if (tag->text != NULL) {
        free(tag->text);
        free(tag->tag_source);
    }

    tag->tag_source = strdup(text);
    tag->text = __tag_brackets_strip(text);

    return true;
}

Tag *tag_new(const char *name) {
    Tag *tag = malloc(sizeof(Tag));

    tag->name = strdup(name);
    tag->text = NULL;

    tag->tag_source = NULL;
    tag->tags = NULL;
    tag->tags_len = 0;

    return tag;
}

// ---------------------------------------------------------

void tag_free(Tag *tag) {
    if (tag == NULL)
        return;

    free(tag->name);

    if (tag->tag_source != NULL)
        free(tag->tag_source);

    if (tag->text != NULL)
        free(tag->text);

    if (tag->tags != NULL) {
        for (size_t i = 0; i < tag->tags_len; i++) {
            tag_free(tag->tags[i]);
        }
        free(tag->tags);
    }
    free(tag);
}

void tag_print(Tag *tag) {
    char *formatted = tag_format(tag, true);
    printf("%s\n", formatted);
    free(formatted);
}
