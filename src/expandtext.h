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

void expandtext_free(ExpandText *exptext);

bool expandtext_match_exists(const char *match);

char *expandtext_add_from_request(const char *match, const char *expand,
                                  Request *request);

char *expandtext_add_from_src(const char *match, const char *expand,
                              DataSqlRow attr_data);

char *expandtext_delete_by_match(const char *match);

char *expandtext_delete_by_id(size_t id);
