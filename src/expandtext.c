#include "expandtext.h"

#include "texc_data/data.h"
#include "texc_data/data_io.h"
#include "texc_data/data_sql.h"
#include "texc_data/data_sql_row.h"

#include "texc_tags/tagmap.h"
#include "texc_tags/tags.h"

#include "texc_utils/array.h"
#include "texc_utils/http_request.h"
#include "texc_utils/logger.h"
#include "texc_utils/str.h"
#include "texc_utils/thread.h"

#include <math.h>
#include <string.h>

#define EXPANDTEXT_ITER(i, body)            \
    do {                                    \
        int iter = 0;                       \
        int i = 0;                          \
        while (iter < data.exptext_len) {   \
            if (data.exptexts[i] == NULL) { \
                i++;                        \
                continue;                   \
            }                               \
            body;                           \
            iter++;                         \
            i++;                            \
        }                                   \
    } while (0)

ExpandText *__expandtext_new(const char *match_str, const char *expand_str,
                             char **error) {
    Tag *match = tag_parse(match_str, "match");
    if (match == NULL) {
        *error = strdup("(match) invalid tag syntax");
        return NULL;
    }

    Tag *expand = tag_parse(expand_str, "expand");
    if (expand == NULL) {
        *error = strdup("(expand) invalid tag syntax");
        tag_free(match);
        return NULL;
    }

    *error = tagmap_validate(match, true);
    if (*error != NULL) {
        tag_free(match);
        tag_free(expand);
        return NULL;
    }

    *error = tagmap_validate(expand, false);
    if (*error != NULL) {
        tag_free(match);
        tag_free(expand);
        return NULL;
    }

    ExpandText *exptext = malloc(sizeof(ExpandText));
    exptext->match = match;
    exptext->expand = expand;

    return exptext;
}

void expandtext_free(ExpandText *exptext) {
    tag_free(exptext->match);
    tag_free(exptext->expand);
    free(exptext);
}

int expandtext_index(const char *match) {
    int found_index = -1;

    EXPANDTEXT_ITER(i, {
        if (str_eq(data.exptexts[i]->match->tag_source, match)) {
            expandtext_free(data.exptexts[i]);
            data.exptexts[i] = NULL;
            data.exptext_len--;
            found_index = i;
            break;
        }
    });

    return found_index;
}

void __expandtext_add(ExpandText *exptext, DataSqlRow row) {
    int index = data_sql_missing_int("idx");

    if (index >= data.exptext_cap) {
        data.exptext_cap = data.exptext_cap * 2;
        data.exptexts =
            realloc(data.exptexts, data.exptext_cap * sizeof(ExpandText *));
    }

    data.exptexts[index] = exptext;
    data.exptext_len++;

    row.index = index;
    data_sql_add(row);
}

char *expandtext_add_from_request(const char *match, const char *expand,
                                  Request *request) {
    char *error;
    ExpandText *exptext = __expandtext_new(match, expand, &error);

    if (error != NULL) {
        return error;
    }

    DataSqlRow row = data_sql_row_from_request(request, &error);
    if (error != NULL) {
        expandtext_free(exptext);
        return error;
    }

    __expandtext_add(exptext, row);

    data_io_save();

    return NULL;
}

char *expandtext_add_from_src(const char *match, const char *expand,
                              DataSqlRow attrs) {
    char *error;
    ExpandText *exptext = __expandtext_new(match, expand, &error);
    if (error != NULL) {
        return error;
    }
    __expandtext_add(exptext, attrs);

    return NULL;
}

char *__delete_by_match(const char *match) {
    int index = expandtext_index(match);

    if (index == -1) {
        char *err;
        str_format(err, "given match '%s' does not exists", match);
        return err;
    }

    char *condition;
    str_format(condition, "idx = %d", index);
    bool deleted = data_sql_delete(condition);
    free(condition);

    data_io_save();
    if (!deleted) {
        return strdup("texc INTERNAL SQL ERROR see logs.txt");
    }
    LOGGER_FORMAT_LOG(LOGGER_INFO, "deleted expandtext with match=%s", match);

    return NULL;
}

char *__delete_by_id(size_t id) {
    int count;
    char *condition;
    str_format(condition, "id = %zd", id);

    DataSqlRow *rows = data_sql_get(condition, &count);
    free(condition);

    if (count == 0) {
        char *err;
        str_format(err, "expandtext with id '%zd' is not found", id);
        free(rows);
        return err;
    }

    int index = rows[0].index;
    expandtext_free(data.exptexts[index]);
    data.exptexts[index] = NULL;
    data.exptext_len--;

    str_format(condition, "id = %zd", id);
    bool deleted = data_sql_delete(condition);

    free(condition);
    free(rows);

    data_io_save();
    if (!deleted) {
        return strdup("texc INTERNAL SQL ERROR see logs.txt");
    }

    LOGGER_FORMAT_LOG(LOGGER_INFO, "deleted expandtext with id=%zd", id);
    return NULL;
}

char *expandtext_delete(const char *ident, ETxIdentifier by) {
    if (by == ETx_BY_MATCH) {
        return __delete_by_match(ident);
    }

    if (by == ETx_BY_ID) {
        return __delete_by_id(atoi(ident));
    }

    return NULL;
}

char *__prepare_update_config(Request *request, char **error) {
    char *update = malloc(sizeof(char) * 1);
    update[0] = '\0';

    const char *enable = request_get_query(request, "enabled");
    if (enable != NULL) {
        if (!(str_eq(enable, "true") || str_eq(enable, "false"))) {
            *error = strdup("enable param must be a boolean value");
            return NULL;
        }

        str_rformat(update, "enabled = %d,", str_eq(enable, "true"));
    }

    // Remove trailing comma
    update[strlen(update) - 1] = '\0';

    return update;
}

char *expandtext_config(const char *ident, ETxIdentifier by, Request *request) {
    char *query_condition;
    if (by == ETx_BY_MATCH) {
        str_format(query_condition, "idx = %d\n", expandtext_index(ident));
    } else if (by == ETx_BY_ID) {
        str_format(query_condition, "id = %s\n", ident);
    }

    char *error = NULL;
    char *update_section = __prepare_update_config(request, &error);
    if (error != NULL) {
        free(query_condition);
        return error;
    }

    char *update_query;
    str_format(update_query, "UPDATE expandtexts SET %s WHERE %s",
               update_section, query_condition);

    if (sqlite3_exec(data.db, update_query, 0, 0, &data.db_error)) {
        error = strdup(data.db_error);
        sqlite3_free(data.db_error);
    }

    data_io_save();
    free(update_query);
    free(query_condition);
    free(update_section);

    return error;
}
