#include "expand.h"

#include "../texc_tags/tagmap.h"
#include "../texc_utils/str.h"

#include "keyboard.h"

void __expand_text(Tag *tag, ExpandSettings *settings) {
    bool is_container =
        str_eq(tag->name, "expand") || str_eq(tag->name, "text");

    // Pure text tag or root tag containing text only
    if (is_container && tag->text != NULL) {
        keyboard_nomod_type_string(tag->text);
        settings->typed_count += strlen(tag->text);
        return;
    }

    const TagMap map = tagmap_get(tag->name, false);
    bool to_exit = false;

    if (map.tag_enter != NULL)
        to_exit = map.tag_enter(map.data, settings);

    if (tag->text != NULL) {
        keyboard_nomod_type_string(tag->text);
    } else if (tag->tags != NULL) {
        for (int i = 0; i < tag->tags_len; i++) {
            __expand_text(tag->tags[i], settings);
        }
    }

    if (map.tag_exit != NULL && to_exit)
        map.tag_exit(map.data, settings);
}

ExpandSettings expand_text(size_t replace_len, Tag *tag) {
    ExpandSettings settings = {
        .is_undoable = true,
        .typed_count = 0,
    };

    keyboard_backspace(replace_len);
    __expand_text(tag, &settings);

    return settings;
}
