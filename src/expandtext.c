#include "expandtext.h"

#include "texc_data/data.h"
#include "texc_data/data_io.h"
#include "texc_data/data_sql.h"

#include "texc_tags/tagmap.h"
#include "texc_tags/tags.h"

#include "texc_utils/array.h"
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

bool expandtext_match_exists(const char *match) {
    EXPANDTEXT_ITER(i, {
        if (str_eq(data.exptexts[i]->match->tag_source, match)) {
            return true;
        }
    });

    return false;
}

void __expandtext_add(ExpandText *exptext, DataSqlRow attrs) {
    int index = data_sql_missing_int("idx");

    if (index >= data.exptext_cap) {
        data.exptext_cap = data.exptext_cap * 2;
        data.exptexts =
            realloc(data.exptexts, data.exptext_cap * sizeof(ExpandText *));
    }

    data.exptexts[index] = exptext;
    data.exptext_len++;

    attrs.index = index;
    data_sql_add(attrs);
}

char *expandtext_add_from_request(const char *match, const char *expand,
                                  Request *request) {
    char *error;
    ExpandText *exptext = __expandtext_new(match, expand, &error);
    if (error != NULL) {
        return error;
    }

    mutex_lock(data.mutex);

    int id = data_sql_missing_int("id");
    __expandtext_add(exptext, (DataSqlRow){
                                  .id = id,
                              });

    data_io_save();
    mutex_unlock(data.mutex);

    return NULL;
}

char *expandtext_add_from_src(const char *match, const char *expand,
                              DataSqlRow attr_data) {
    char *error;
    ExpandText *exptext = __expandtext_new(match, expand, &error);
    if (error != NULL) {
        return error;
    }
    mutex_lock(data.mutex);

    __expandtext_add(exptext, attr_data);

    mutex_unlock(data.mutex);

    return NULL;
}

char *__delete_by_match(const char *match) {
    mutex_lock(data.mutex);
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

    if (found_index == -1) {
        char *err;
        str_format(err, "given match word '%s' does not exists", match);
        mutex_unlock(data.mutex);
        return err;
    }

    char *condition;
    str_format(condition, "idx = %d", found_index);
    bool deleted = data_sql_delete(condition);
    free(condition);

    data_io_save();
    mutex_unlock(data.mutex);
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

    mutex_lock(data.mutex);

    DataSqlRow *rows = data_sql_get(condition, &count);
    free(condition);

    if (count == 0) {
        char *err;
        str_format(err, "expandtext with id '%zd' is not found", id);
        free(rows);
        mutex_unlock(data.mutex);
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
    mutex_unlock(data.mutex);

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
