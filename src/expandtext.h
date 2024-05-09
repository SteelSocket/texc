#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "texc_data/data_sql.h"

#include "texc_tags/tags.h"
#include "texc_utils/http_request.h"

typedef struct {
    Tag *match;
    Tag *expand;
} ExpandText;

typedef enum {
    ETx_BY_MATCH,
    ETx_BY_ID,
} ETxIdentifier;

void expandtext_free(ExpandText *exptext);

int expandtext_index(const char *match);

char *expandtext_add_from_request(const char *match, const char *expand,
                                  Request *request);

char *expandtext_add_from_src(const char *match, const char *expand,
                              DataSqlRow attrs);

char *expandtext_delete(const char *ident, ETxIdentifier by);

char *expandtext_config(const char *ident, ETxIdentifier by, Request *request);
