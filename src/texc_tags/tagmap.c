#include "tagmap.h"

#include <ctype.h>
#include <stdint.h>

#include "../texc_utils/array.h"
#include "../texc_utils/str.h"

#include "expand_tags.h"
#include "match_tags.h"

#ifdef _WIN32
#define __ENTER_CHARACTER "\r"
#else
#define __ENTER_CHARACTER "\n"
#endif

#define __GET_MAP_TYPE(is_match) is_match ? "match" : "expand"
#define __STD_EXPAND_TAGMAP(tname, key_code)                     \
    {                                                            \
        .tag_name = tname, .data = (void *)(intptr_t)key_code,   \
        .flags = TAGMAP_FLAGS_NONE, .tag_enter = etag_std_press, \
        .tag_exit = etag_std_release,                            \
    }
#define __STD_MATCH_TAGMAP(tname, char_str)                            \
    {                                                                  \
        .tag_name = tname, .data = (void *)char_str,                   \
        .flags = TAGMAP_FLAGS_STANDALONE, .tag_enter = mtag_key_check, \
        .tag_char = mtag_key_char,                                     \
    }

// ---------------------------------
// TagMap for all match tags
// ---------------------------------

TagMap __tagmap_match[] = {
    __STD_MATCH_TAGMAP("enter", __ENTER_CHARACTER),
    __STD_MATCH_TAGMAP("tab", "\t"),

    {
        .tag_name = "tcase",
        .flags = TAGMAP_FLAGS_TEXT_ONLY,
        .tag_enter = mtag_tcase_enter,
        .tag_exit = mtag_tcase_exit,
    },
};
size_t __tagmap_match_len = array_len(__tagmap_match);
// ---------------------------------

// ---------------------------------
// TagMap for all expand tags
// ---------------------------------
const TagMap __tagmap_expand[] = {
    __STD_EXPAND_TAGMAP("enter", KEYBOARD_RETURN),
    __STD_EXPAND_TAGMAP("tab", KEYBOARD_TAB),
    __STD_EXPAND_TAGMAP("backspace", KEYBOARD_BACKSPACE),

    __STD_EXPAND_TAGMAP("shift", KEYBOARD_SHIFT),
    __STD_EXPAND_TAGMAP("ctrl", KEYBOARD_CONTROL),

};
size_t __tagmap_expand_len = array_len(__tagmap_expand);
// ---------------------------------
#define __TM_FLAG_ERR(msg)                                             \
    do {                                                               \
        str_format(*error_msg, "(%s) <%s> " msg, map_type, tag->name); \
        return;                                                        \
    } while (0)

void __handle_tag_name(Tag *tag, bool is_match, char **error_msg) {
    TagMap current_map = tagmap_get(tag->name, is_match);
    const char *map_type = __GET_MAP_TYPE(is_match);

    if (current_map.tag_name == NULL) {
        if (strlen(tag->name) == 1)
            __TM_FLAG_ERR(
                "is an invalid tag name, single character tags must be only "
                "lower case alpha numeric characters");
        __TM_FLAG_ERR("is an invalid tag name");
    }

    // TODO implement better flag checking system
    if (current_map.flags != TAGMAP_FLAGS_NONE) {
        if (current_map.flags & TAGMAP_FLAGS_STANDALONE) {
            if (current_map.flags & TAGMAP_FLAGS_TEXT_ONLY &&
                tag->tags_len != 0)
                __TM_FLAG_ERR(
                    "should be standalone or should only contain text");

            else if (tag->tags_len != 0 || tag->text != NULL)
                __TM_FLAG_ERR("should be standalone");

        } else {
            if (current_map.flags & TAGMAP_FLAGS_TEXT_ONLY &&
                tag->tags_len != 0)
                __TM_FLAG_ERR("should contain text only");

            else if (tag->tags_len == 0 && tag->text == NULL)
                __TM_FLAG_ERR("should contain child tags or text");
        }
    }

    if (current_map.tag_validate != NULL)
        *error_msg = current_map.tag_validate(tag);
}

void __tagmap_validate(Tag *tag, bool is_match, char **error_msg) {
    if (!str_eq(tag->name, "match") && !str_eq(tag->name, "expand") &&
        !str_eq(tag->name, "text")) {
        __handle_tag_name(tag, is_match, error_msg);

        if (*error_msg != NULL)
            return;

    } else if (str_eq(tag->name, "text") && tag->tags_len != 0) {
        str_format(*error_msg, "(%s) <text> tag should not contain child tags",
                   __GET_MAP_TYPE(is_match));
        return;
    }

    for (int i = 0; i < tag->tags_len; i++) {
        __tagmap_validate(tag->tags[i], is_match, error_msg);
        if (*error_msg != NULL)
            return;
    }
}

const TagMap tagmap_get(const char *tag_name, bool is_match) {
    // Handle single character presses
    if (strlen(tag_name) == 1) {
        if (is_match || !isalnum(*tag_name) || !islower(*tag_name))
            return (const TagMap){0};

        return (const TagMap){
            .tag_name = tag_name,
            .data = (void *)tag_name,
            .flags = TAGMAP_FLAGS_NONE,
            .tag_enter = etag_press_char,
            .tag_exit = etag_release_char,
        };
    }

    const TagMap *map;
    size_t map_len;

    if (is_match) {
        map = __tagmap_match;
        map_len = __tagmap_match_len;
    } else {
        map = __tagmap_expand;
        map_len = __tagmap_expand_len;
    }

    for (int i = 0; i < map_len; i++) {
        if (str_eq(tag_name, map[i].tag_name)) {
            return map[i];
        }
    }

    return (const TagMap){0};
}

char *tagmap_validate(Tag *tag, bool is_match) {
    char *error = NULL;
    __tagmap_validate(tag, is_match, &error);

    return error;
}
