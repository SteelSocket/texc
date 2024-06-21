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

char *__get_identifier_param(Request *request, const char **identifier,
                             ETxIdentifier *itype) {
    const char *match = request_get_query(request, "match");
    const char *id = request_get_query(request, "id");
    const char *group = request_get_query(request, "group");

    if ((match != NULL) + (id != NULL) + (group != NULL) > 1) {
        return strdup("Multiple identifiers specified at the same time");
    }

    if (match != NULL) {
        *identifier = match;
        *itype = ETx_BY_MATCH;
    } else if (id != NULL) {
        if (!str_isnumber(id))
            return strdup("The given id is not a number");

        if (atoi(id) < 0)
            return strdup("id should be a positive integer");

        *identifier = id;
        *itype = ETx_BY_ID;
    } else if (group != NULL) {
        *identifier = group;
        *itype = ETx_BY_GROUP;
    } else {
        return strdup("identifier not specified");
    }

    return NULL;
}

Response *__handle_add(Request *request) {
    mutex_lock(data.mutex);

    char *error = NULL;
    DataSqlRow *row = data_sql_row_from_request(request, &error);
    if (error == NULL) {
        error = expandtext_add(row);
        if (error == NULL)
            data_io_save();
    }
    data_sql_row_free(row);
    mutex_unlock(data.mutex);

    if (error != NULL) {
        Response *response = __RESPONSE_ERR(error);
        LOGGER_WARNING(error);
        free(error);
        return response;
    }

    return __RESPONSE_SUCESS;
}

Response *__handle_remove(Request *request) {
    const char *identifier;
    ETxIdentifier itype;

    char *error = __get_identifier_param(request, &identifier, &itype);

    if (error == NULL) {
        mutex_lock(data.mutex);
        error = expandtext_delete(identifier, itype);
        mutex_unlock(data.mutex);
    }

    if (error != NULL) {
        LOGGER_WARNING(error);
        Response *err = __RESPONSE_ERR(error);
        free(error);
        return err;
    }

    return __RESPONSE_SUCESS;
}

Response *__handle_list(Request *request) {
    mutex_lock(data.mutex);
    char *csv_string = data_io_expandtexts_as_csv(NULL, NULL);
    mutex_unlock(data.mutex);

    if (csv_string == NULL) {
        return __RESPONSE_ERR("texc INTERNAL SQL ERROR see logs.txt");
    }

    Response *res = response_new(200, csv_string);
    free(csv_string);

    return res;
}

Response *__handle_close(Request *request) {
    keyhook_raw_quit();
    return __RESPONSE_SUCESS;
}

Response *__handle_config(Request *request) {
    const char *identifier;
    ETxIdentifier itype;

    char *error = __get_identifier_param(request, &identifier, &itype);

    if (error == NULL) {
        mutex_lock(data.mutex);
        error = expandtext_config(identifier, itype, request);
        mutex_unlock(data.mutex);
    }

    if (error != NULL) {
        LOGGER_WARNING(error);
        Response *err = __RESPONSE_ERR(error);
        free(error);
        return err;
    }
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

    else if (str_eq(request->path, "/config"))
        return __handle_config(request);

    else if (str_eq(request->path, "/version"))
        return response_new(200, TEXC_VERSION);

    else
        return response_new(404, "Api not found");

    return __RESPONSE_SUCESS;
}
