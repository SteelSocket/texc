#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "texc_data/data_sql_row.h"

#include "texc_tags/tags.h"
#include "texc_utils/http_request.h"

typedef struct ExpandText {
    Tag *match;
    Tag *expand;
} ExpandText;

typedef enum {
    ETx_BY_MATCH,
    ETx_BY_ID,
    ETx_BY_GROUP,
} ETxIdentifier;

/**
 * @brief Frees the ExpandText* from memory
 */
void expandtext_free(ExpandText *exptext);

/**
 * @brief Adds a expandtext to expandtext array and sqlite table
 *
 * @return error message
 */
char *expandtext_add(DataSqlRow *row);

/**
 * @brief Deletes a expandtext from expandtext array and sqlite table
 *
 * @param ident The indentifier
 * @param by The identifier type
 * @return error message
 */
char *expandtext_delete(const char *ident, ETxIdentifier by);

/**
 * @brief Changes the attributes of a expandtext in sqlite table
 *
 * @param ident The indentifier
 * @param by The indentifier type
 * @param request The http request
 * @return error message
 */
char *expandtext_config(const char *ident, ETxIdentifier by, Request *request);
