#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "texc_data/data_sql.h"

#include "texc_tags/tags.h"
#include "texc_utils/http_request.h"

typedef struct ExpandText {
    Tag *match;
    Tag *expand;
} ExpandText;

typedef enum {
    ETx_BY_MATCH,
    ETx_BY_ID,
} ETxIdentifier;

void expandtext_free(ExpandText *exptext);

char *expandtext_add(DataSqlRow *row);

char *expandtext_delete(const char *ident, ETxIdentifier by);

char *expandtext_config(const char *ident, ETxIdentifier by, Request *request);
