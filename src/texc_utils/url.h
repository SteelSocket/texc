// Take from http://www.geekhideout.com/urlcode.shtml
#pragma once

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *url_encode(const char *str);

char *url_decode(const char *str);

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

#endif