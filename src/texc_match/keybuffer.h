#pragma once

#include <stdbool.h>
#include <stdint.h>

extern char *keybuffer;

extern size_t keybuffer_size;
extern size_t keybuffer_cursor;

void keybuffer_print();

bool keybuffer_cursor_move(int offset);

void keybuffer_clear();

void keybuffer_push(char c);

void keybuffer_pop();

void keybuffer_append(char *str);

void keybuffer_replace(size_t pop_len, char *rstr);
