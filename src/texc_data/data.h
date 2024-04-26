#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sqlite3.h"

#include "../expandtext.h"
#include "../texc_utils/thread.h"

typedef struct {
    Mutex *mutex;

    ExpandText **exptexts;
    size_t exptext_len;
    size_t exptext_cap;

    sqlite3 *db;
    char *db_error;

} Data;

extern Data data;

char *data_get_dir();

char *data_get_port_file();

bool data_init();

void data_free();
