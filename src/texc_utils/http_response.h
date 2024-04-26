#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESPONSE_CODE_200 "200 OK"
#define RESPONSE_CODE_400 "400 Bad Request"
#define RESPONSE_CODE_504 "504 Gateway Timeout"

typedef struct {
    int status_code;
    char *body;

    char *raw;
    int raw_len;
} Response;

Response *response_new(int status_code, const char *body);

Response *response_from(char *source);

void response_free(Response *response);

#ifdef UTILS_IMPLEMENTATION

const char *REQUEST_FORMAT =
    "HTTP/1.1 %s\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s";

Response *response_new(int status_code, const char *body) {
    char *string_code;
    switch (status_code) {
        case 200: {
            string_code = RESPONSE_CODE_200;
            break;
        }

        case 400: {
            string_code = RESPONSE_CODE_400;
            break;
        }

        case 504: {
            string_code = RESPONSE_CODE_504;
            break;
        }
        default: {
            string_code = RESPONSE_CODE_200;
            break;
        }
    }

    char *raw;
    str_format(raw, REQUEST_FORMAT, string_code, strlen(body), body);

    Response *response = malloc(sizeof(Response));
    response->body = strdup(body);
    response->status_code = status_code;

    response->raw = raw;
    response->raw_len = strlen(raw);

    return response;
}

Response *response_from(char *source) {
    char *body_start = strstr(source, "\r\n\r\n") + 4;

    int status_code;
    char *body = malloc((strlen(body_start) + 1) * sizeof(char));

    sscanf(source, "HTTP/1.1 %d", &status_code);
    strcpy(body, body_start);

    Response *response = malloc(sizeof(Response));
    response->body = body;
    response->status_code = status_code;

    response->raw = strdup(source);
    response->raw_len = strlen(source);

    return response;
}

void response_free(Response *response) {
    free(response->body);
    free(response->raw);
    free(response);
}

#endif
