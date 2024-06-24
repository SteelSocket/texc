#include "match.h"
#include "keybuffer.h"

#include "../texc_tags/tagmap.h"
#include "../texc_utils/str.h"

#include <ctype.h>

bool __char_eq(char c1, char c2, MatchSettings *settings) {
    if (settings->__is_casesensitive)
        return c1 == c2;
    return tolower(c1) == tolower(c2);
}

void __match_text(Tag *tag, MatchSettings *settings) {
    bool is_container = str_eq(tag->name, "match") || str_eq(tag->name, "text");
    const TagMap map = tagmap_get(tag->name, true);

    if (!is_container) {
        map.tag_enter(map.data, (void *)settings);

        if (!settings->ok) {
            return;
        }
    }

    if (tag->text != NULL) {
        for (int i = strlen(tag->text) - 1; i >= 0; i--) {
            settings->cursor--;
            if (settings->cursor < 0) {
                settings->ok = false;
                return;
            }

            if (!__char_eq(tag->text[i], keybuffer[settings->cursor],
                           settings)) {
                settings->ok = false;
                return;
            }
        }
    } else {
        for (int i = tag->tags_len - 1; i >= 0; i--) {
            __match_text(tag->tags[i], settings);
            if (!settings->ok) {
                return;
            }
        }
    }

    if (!is_container && map.tag_exit != NULL) {
        map.tag_exit(map.data, (void *)settings);
        if (!settings->ok) {
            return;
        }
    }
}

MatchSettings match_text(Tag *match) {
    MatchSettings settings = (MatchSettings){
        .ok = true,
        .cursor = keybuffer_size,
        .is_undoable = true,
        .__is_casesensitive = true,
    };

    __match_text(match, &settings);

    return settings;
}

void __match_get_initializer(Tag *tag, MatchSettings *settings, char **buf) {
    bool is_container = str_eq(tag->name, "match") || str_eq(tag->name, "text");
    const TagMap map = tagmap_get(tag->name, true);

    if (!is_container) {
        if (map.tag_char != NULL) {
            str_mcpy(*buf, map.tag_char(map.data, (void *)settings));
            return;
        }

        map.tag_enter(map.data, (void *)settings);
    }

    if (tag->text != NULL) {
        int i = strlen(tag->text) - 1;
        char c = tag->text[i];

        if (settings->__is_casesensitive || !isalpha(c)) {
            str_format(*buf, "%c", c);
            return;
        } else {
            str_format(*buf, "%c%c", toupper(c), tolower(c));
            return;
        }
    } else {
        int i = tag->tags_len - 1;
        __match_get_initializer(tag->tags[i], settings, buf);
        return;
    }

    if (!is_container && map.tag_exit != NULL) {
        map.tag_exit(map.data, (void *)settings);
        if (!settings->ok) {
            return;
        }
    }
}

char *match_get_initializer(const char *match) {
    MatchSettings settings = (MatchSettings){
        .ok = true,
        .cursor = keybuffer_size,
        .is_undoable = true,
        .__is_casesensitive = true,
    };
    Tag *match_tag = tag_parse(match, "match");

    char *buffer;
    __match_get_initializer(match_tag, &settings, &buffer);

    tag_free(match_tag);
    return buffer;
}
