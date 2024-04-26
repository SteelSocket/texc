#include "match.h"
#include "keybuffer.h"

#include "../texc_tags/tagmap.h"
#include "../texc_utils/str.h"

bool __char_eq(char c1, char c2, MatchSettings *settings) {
    if (settings->is_casesensitive)
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
        .is_casesensitive = true,
    };

    __match_text(match, &settings);

    return settings;
}
