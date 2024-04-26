#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void __parse_queries(char *url, Request *request) {
    if (strchr(url, '?') == NULL) {
        request->query_count = 0;
        request->queries = NULL;
        return;
    }

    request->query_count = 1;

    char *context;
    // Also modifies request->path to be only pure path
    char *token = strtok_r(url, "?", &context);

    {
        bool is_valid_query = false;
        char *copy_context = context;
        while (*copy_context++ != '\0') {
            if (*copy_context == '&') {
                request->query_count += 1;
            } else if (*copy_context == '=' && (*copy_context + 1) != '\0') {
                is_valid_query = true;
            }
        }

        if (!is_valid_query) {
            request->query_count = 0;
            request->queries = NULL;
            return;
        }
    }

    request->queries = malloc(request->query_count * sizeof(UrlQuery));

    int i = 0;
    while ((token = strtok_r(NULL, "&", &context)) != NULL) {
        char *p_context;
        char *param = strtok_r(token, "=", &p_context);
        char *value = strtok_r(NULL, "=", &p_context);

        request->queries[i] = (UrlQuery){url_decode(param), url_decode(value)};
        i++;
    }
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
        free(request);
        free(request->method);
        return NULL;
    }
    __parse_queries(token, request);
    request->path = url_decode(token);

    char *double_newline = strstr(context, "\r\n\r\n");
    if (double_newline == NULL) {
        request->body = malloc(1);  // for request_free
        request_free(request);
        return NULL;
    }
    char *body = double_newline + strlen("\r\n\r\n");
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
