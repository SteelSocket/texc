#include "server.h"

#include "../expandtext.h"
#include "../texc_data/data.h"
#include "../texc_data/data_io.h"
#include "../texc_data/data_sql.h"

#include "../texc_utils/logger.h"

#include "../texc_keyhook/keyhook.h"

#include <stdlib.h>

#define __RESPONSE_SUCESS response_new(200, "Sucess")
#define __RESPONSE_ERR(x) response_new(400, x)

#define __ERR_QUERY_REQUIRED(query, type) \
    "The query \"" query "\": " type " is required"

#define __QUERY_REQUIRED_GET(request, var, query, type)                  \
    do {                                                                 \
        var = request_get_query(request, query);                         \
        if (var == NULL) {                                               \
            LOGGER_WARNING(__ERR_QUERY_REQUIRED(query, type));           \
            return response_new(400, __ERR_QUERY_REQUIRED(query, type)); \
        }                                                                \
    } while (0)

Response *__handle_add(Request *request) {
    const char *match;
    __QUERY_REQUIRED_GET(request, match, "match", "string");

    if (expandtext_match_exists(match))
        return __RESPONSE_ERR("match with the given word already exists");

    const char *expand;
    __QUERY_REQUIRED_GET(request, expand, "expand", "string");

    char *error = expandtext_add_from_request(match, expand, request);
    if (error != NULL) {
        Response *response = __RESPONSE_ERR(error);
        LOGGER_WARNING(error);
        free(error);

        return response;
    }

    return __RESPONSE_SUCESS;
}

Response *__handle_remove(Request *request) {
    const char *match = request_get_query(request, "match");
    const char *id = request_get_query(request, "id");

    char *error;
    if (match != NULL) {
        error = expandtext_delete_by_match(match);
    } else if (id != NULL) {
        if (str_isnumber(id) && atoi(id) >= 0) {
            error = expandtext_delete_by_id(atoi(id));
        } else {
            error = strdup("id param should be a positive integer");
        }
    }

    if (error) {
        LOGGER_WARNING(error);
        Response *err = __RESPONSE_ERR(error);
        free(error);
        return err;
    }

    return __RESPONSE_SUCESS;
}

Response *__handle_list(Request *request) {
    mutex_lock(data.mutex);

    int count;
    DataSqlRow *rows = data_sql_get(NULL, &count);
    if (rows == NULL) {
        mutex_unlock(data.mutex);
        return __RESPONSE_ERR("texc INTERNAL SQL ERROR see logs.txt");
    }

    char *csv_string;
    str_mcpy(csv_string, "match,expand,id\n");

    for (int i = 0; i < count; i++) {
        DataSqlRow row = rows[i];
        ExpandText *exptext = data.exptexts[row.index];
        char *csv_line = data_io_expandtext_as_csv(exptext, row);

        str_rcat(csv_string, csv_line);

        free(csv_line);
    }

    free(rows);
    mutex_unlock(data.mutex);

    Response *res = response_new(200, csv_string);
    free(csv_string);

    return res;
}

Response *__handle_close(Request *request) {
    keyhook_raw_quit();
    return __RESPONSE_SUCESS;
}

Response *__server_handle_api(Request *request) {
    if (str_eq(request->path, "/add"))
        return __handle_add(request);

    else if (str_eq(request->path, "/remove"))
        return __handle_remove(request);

    else if (str_eq(request->path, "/list"))
        return __handle_list(request);

    else if (str_eq(request->path, "/close"))
        return __handle_close(request);

    else if (str_eq(request->path, "/version"))
        return response_new(200, TEXC_VERSION);
    else
        return response_new(404, "Api not found");

    return __RESPONSE_SUCESS;
}
