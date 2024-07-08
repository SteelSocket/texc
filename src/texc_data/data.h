#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "sqlite3.h"

#include "../expandtext.h"
#include "../settings.h"
#include "../texc_utils/thread.h"

typedef struct {
    // Settings of texc
    // No need to lock mutex for settings as we don't write any value to it
    Settings settings;

    Mutex *mutex;
    char token[64];

    ExpandText **exptexts;
    size_t exptext_len;
    size_t exptext_cap;

    sqlite3 *db;
    char *db_error;

} Data;

/**
 * @brief The global data variable
 */
extern Data data;

/**
 * @brief Gets the path to the local data folder with /texc appended to it
 *
 * @return The path to texc folder
 */
char *data_get_dir();

/* @brief Gets the path to server port file
 *
 * @return The path to the port file
 */
char *data_get_port_file();

/**
 * @brief Initializes the data along with settings and sqlite table
 *
 * @return Boolean representing success
 */
bool data_init();

/**
 * @brief Frees all allocated data and saves all the ExpandTexts
 *
 * @param save whether to save the data or not
 */
void data_free(bool save);
