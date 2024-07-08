// Take from http://www.geekhideout.com/urlcode.shtml
#pragma once

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "array.h"

char *url_encode(const char *str);

char *url_decode(const char *str);

typedef struct {
    const char *url;
    char **params;
    char **values;
    int count;

    int __full_url_size;
} UrlBuilder;

UrlBuilder url_builder_new(const char *url);

void url_builder_add_param(UrlBuilder *builder, const char *param,
                           const char *value);

char *url_builder_prepare(UrlBuilder builder);

void url_builder_free(UrlBuilder *builder);

#ifdef UTILS_IMPLEMENTATION

/* Converts a hex character to its integer value */
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(const char *str) {
    const char *pstr = str;
    char *buf = malloc(strlen(str) * 3 + 1);
    char *pbuf = buf;

    while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' ||
            *pstr == '~')
            *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4),
            *pbuf++ = to_hex(*pstr & 15);
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(const char *str) {
    const char *pstr = str;
    char *buf = malloc(strlen(str) + 1);
    char *pbuf = buf;

    while (*pstr) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                int hex_val = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                if (!(hex_val >= 32 && hex_val <= 126)) {
                    pstr += 3;
                    continue;
                }

                *pbuf++ = hex_val;
                pstr += 2;
            }
        } else if (*pstr == '+') {
            *pbuf++ = ' ';
        } else {
            *pbuf++ = *pstr;
        }
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}

UrlBuilder url_builder_new(const char *url) {
    UrlBuilder builder;
    builder.url = url;
    builder.params = array_create(char *);
    builder.values = array_create(char *);
    builder.count = 0;
    builder.__full_url_size = strlen(url);
    return builder;
}

void url_builder_add_param(UrlBuilder *builder, const char *param,
                           const char *value) {
    
    array_increase_size(builder->params, builder->count, sizeof(char *));
    array_increase_size(builder->values, builder->count, sizeof(char *));


    char *eparam = url_encode(param);
    char *evalue = url_encode(value);

    builder->params[builder->count] = eparam;
    builder->values[builder->count] = evalue;

    builder->__full_url_size += strlen(eparam) + strlen(evalue) + 2; // +2 for = and ? or &

    builder->count++;
}

char *url_builder_prepare(UrlBuilder builder) {
    char *url = calloc(builder.__full_url_size, sizeof(char));
    strcpy(url, builder.url);

    if (builder.count == 0)
        return url;

    sprintf(url, "%s?%s=%s", url, builder.params[0], builder.values[0]);

    for (int i = 1; i < builder.count; i++) {
        sprintf(url, "%s&%s=%s", url, builder.params[i], builder.values[i]);
    }

    return url;
}

void url_builder_free(UrlBuilder *builder) {
    for (int i = 0; i < builder->count; i++) {
        free(builder->params[i]);
        free(builder->values[i]);
    }

    free(builder->params);
    free(builder->values);
}

#endif
