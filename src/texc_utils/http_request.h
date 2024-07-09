#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "str.h"
#include "url.h"

typedef struct {
    char *param;
    char *value;
} UrlQuery;

typedef struct {
    char *method;
    char *path;
    char *body;

    UrlQuery *queries;
    int query_count;
} Request;

Request *request_parse(char buffer[]);

void request_free(Request *request);

char *request_get_query(Request *request, const char *param);

#ifdef UTILS_IMPLEMENTATION

bool __parse_queries(char *url, Request *request) {
    if (strchr(url, '?') == NULL) {
        request->query_count = 0;
        request->queries = NULL;
        return true;
    }

    request->query_count = 1;

    char *context;
    // Also modifies request->path to be only pure path
    char *token = strtok_r(url, "?", &context);

    request->query_count = 0;
    request->queries = NULL;

    bool param_start = true;
    bool value_start = false;

    char *copy_ctx = context;
    while (*copy_ctx != '\0') {
        if (*copy_ctx == ' ') {
            break;
        }

        if (*copy_ctx == '=') {
            if (param_start) {
                param_start = false;
                value_start = true;
            } else {
                return false;
            }
        } else if (*copy_ctx == '&') {
            if (value_start) {
                value_start = false;
                param_start = true;
            } else {
                return false;
            }
        } else if (!isalnum(*copy_ctx) &&
            (*copy_ctx != '%' && *copy_ctx != '-' && *copy_ctx != '.' &&
             *copy_ctx != '~' && *copy_ctx != '_' && *copy_ctx != '+')) {
            return false;
        }

        copy_ctx++;
    }

    request->queries = array_create(UrlQuery);

    while ((token = strtok_r(NULL, "&", &context)) != NULL) {
        char *p_context;
        char *param = strtok_r(token, "=", &p_context);
        char *value = strtok_r(NULL, "=", &p_context);

        if (param == NULL || value == NULL) {
            if (request->query_count == 0)
                return false;
            request->query_count--;
            continue;
        }

        char *dparam = url_decode(param);
        char *dvalue = url_decode(value);
        UrlQuery uquery = (UrlQuery){dparam, dvalue};

        array_resize_add(request->queries, request->query_count, uquery, UrlQuery);
    }

    return true;
}

Request *request_parse(char buffer[]) {
    Request *request = malloc(sizeof(Request));

    char *context;
    char *token;

    token = strtok_r(buffer, " ", &context);
    if (token == NULL) {
        free(request);
        return NULL;
    }
    request->method = strdup(token);

    token = strtok_r(NULL, " ", &context);
    if (token == NULL) {
        free(request->method);
        free(request);
        return NULL;
    }
    bool success = __parse_queries(token, request);
    if (!success) {
        free(request->method);
        free(request);
        return NULL;
    }

    request->path = url_decode(token);

    char *double_newline = strstr(context, "\r\n\r\n");
    if (double_newline == NULL) {
        request->body = malloc(1);  // for request_free
        request_free(request);
        return NULL;
    }
    char *body = double_newline + 4;
    request->body = strdup(body);

    return request;
}

void request_free(Request *request) {
    if (request->queries != NULL) {
        for (size_t i = 0; i < request->query_count; i++) {
            free(request->queries[i].param);
            free(request->queries[i].value);
        }
        free(request->queries);
    }
    free(request->method);
    free(request->path);
    free(request->body);
    free(request);
}

char *request_get_query(Request *request, const char *param) {
    if (request->queries == NULL) {
        return NULL;  // No queries present
    }

    for (int i = 0; i < request->query_count; i++) {
        if (str_eq(request->queries[i].param, param)) {
            return request->queries[i].value;  // Found the query parameter
        }
    }

    return NULL;  // Query parameter not found
}

#endif
