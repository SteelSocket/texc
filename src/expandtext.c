#include "expandtext.h"

#include "texc_data/data.h"
#include "texc_data/data_io.h"
#include "texc_data/data_sql.h"
#include "texc_data/data_sql_row.h"

#include "texc_match/match.h"
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

void __expandtext_add(ExpandText *exptext, int index) {
    if (index >= data.exptext_cap) {
        data.exptext_cap = data.exptext_cap * 2;
        data.exptexts =
            realloc(data.exptexts, data.exptext_cap * sizeof(ExpandText *));
    }

    data.exptexts[index] = exptext;
    data.exptext_len++;
}

char *expandtext_add(DataSqlRow *row) {
    char *error = NULL;
    ExpandText *exptext = __expandtext_new(row->match, row->expand, &error);
    if (error != NULL)
        return error;

    __expandtext_add(exptext, row->index);
    data_sql_add(row);

    LOGGER_FORMAT_LOG(LOGGER_INFO,
                      "Added record to expandtexts table with id=%zd", row->id);
    return NULL;
}

char *expandtext_delete(const char *ident, ETxIdentifier by) {
    char *condition;

    if (by == ETx_BY_MATCH) {
        str_format(condition, "match = '%s'\n", ident);
    } else if (by == ETx_BY_ID) {
        str_format(condition, "id = %s\n", ident);
    } else if (by == ETx_BY_GROUP) {
        str_format(condition, "\"group\"= '%s'\n", ident);
    }

    int count;
    DataSqlRow **rows = data_sql_get_row(condition, &count);
    if (rows == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        DataSqlRow *row = rows[i];
        ExpandText *exptext = data.exptexts[row->index];

        expandtext_free(exptext);
        data.exptexts[row->index] = NULL;
        data.exptext_len--;
        data_sql_row_free(row);
    }
    free(rows);

    bool deleted = data_sql_delete(condition);
    free(condition);

    data_io_save();
    if (!deleted) {
        return strdup("texc INTERNAL SQL ERROR see logs.txt");
    }

    if (by == ETx_BY_MATCH)
        LOGGER_FORMAT_LOG(LOGGER_INFO, "deleted expandtext with match=%s",
                          ident);
    else if (by == ETx_BY_ID)
        LOGGER_FORMAT_LOG(LOGGER_INFO, "deleted expandtext with id=%s", ident);
    else if (by == ETx_BY_GROUP)
        LOGGER_FORMAT_LOG(LOGGER_INFO, "deleted expandtext with group=%s",
                          ident);

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
        str_format(query_condition, "match = '%s'\n", ident);
    } else if (by == ETx_BY_ID) {
        str_format(query_condition, "id = %s\n", ident);
    } else if (by == ETx_BY_GROUP) {
        str_format(query_condition, "\"group\"= '%s'\n", ident);
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
        return error;
    }

    if (sqlite3_changes(data.db) <= 0) {
        if (by == ETx_BY_MATCH) {
            str_format(error,
                       "No text-expansion with given match \"%s\" is found",
                       ident);
        } else if (by == ETx_BY_ID) {
            str_format(error, "No text-expansion with given id \"%s\" is found",
                       ident);
        } else {
            str_format(
                error,
                "No text-expansion with given identifier \"%s\" is found",
                ident);
        }
        return error;
    }

    data_io_save();
    free(update_query);
    free(query_condition);
    free(update_section);

    return error;
}
