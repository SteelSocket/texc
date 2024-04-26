#include "keybuffer.h"

#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *keybuffer;

size_t keybuffer_size;
size_t keybuffer_cursor;

void __print_char(char c, bool nline) {
    if (c == '\n') {
        printf("'\\n'");
    } else if (c == '\b') {
        printf("'\\b'");
    } else if (c == '\r') {
        printf("'\\r'");
    } else if (c == '\t') {
        printf("'\\t'");
    } else {
        printf("'%c'", c);
    }
    if (nline)
        printf("\n");
}

void keybuffer_print() {
    printf("[");
    for (size_t i = 0; i < keybuffer_size; i++) {
        __print_char(keybuffer[i], false);
        printf(", ");
    }
    printf("]\n");
}

bool keybuffer_cursor_move(int offset) {
    int new_cursor = keybuffer_cursor + offset;
    if (new_cursor >= 0 && new_cursor <= keybuffer_size) {
        keybuffer_cursor = (size_t)new_cursor;
        return true;
    }
    return false;
}

void keybuffer_clear() {
    if (keybuffer == NULL)
        keybuffer = malloc(sizeof(char));
    else
        keybuffer = realloc(keybuffer, sizeof(char));

    keybuffer_size = 0;
    keybuffer_cursor = 0;
}

void keybuffer_push(char c) {
    keybuffer_size++;
    keybuffer = realloc(keybuffer, keybuffer_size * sizeof(char));

    int cindex = keybuffer_size - keybuffer_cursor - 1;

    memmove(keybuffer + cindex + 1, keybuffer + cindex, keybuffer_cursor);
    keybuffer[cindex] = c;
}

void keybuffer_pop() {
    if (keybuffer_size == 0)
        return;

    int rindex = keybuffer_size - keybuffer_cursor - 1;
    for (; rindex < keybuffer_size - 1; rindex++) {
        keybuffer[rindex] = keybuffer[rindex + 1];
    }

    keybuffer_size--;
    keybuffer = realloc(keybuffer, keybuffer_size * sizeof(char));

    if (keybuffer_cursor >= keybuffer_size) {
        keybuffer_cursor = (keybuffer_size != 0) ? keybuffer_size - 1 : 0;
    }
}

void keybuffer_append(char *str) {
    while (*str != '\0') {
        keybuffer_push(*(str++));
    }
}

void keybuffer_replace(size_t pop_len, char *rstr) {
    for (size_t i = 0; i < pop_len; i++) {
        keybuffer_pop();
    }
    keybuffer_append(rstr);
}
