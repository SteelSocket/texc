#pragma once

#include <stdbool.h>

#include "../texc_match/match.h"

typedef struct {
    char character;
    int keycode;

    bool is_keydown;
    bool is_ctrldown;

} KeyEvent;

extern bool keyhook_is_expanding;
extern bool keyhook_try_undo;
extern char *keyhook_last_source;
extern int keyhook_expand_len;
extern bool keyhook_try_expand;
extern MatchSettings keyhook_match_settings;
extern Tag *keyhook_expand_tag;

void keyhook_raw_run();

void keyhook_raw_quit();

void keyhook_raw_handle_keydown(KeyEvent event);

void keyhook_raw_handle_keyup(KeyEvent event);

void keyhook_handle_event(KeyEvent event);

void keyhook_expand_matched();

bool keyhook_check_for_match(KeyEvent event);

void keyhook_reset_undo();

void keyhook_reset();

void keyhook_run();
