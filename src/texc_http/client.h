#pragma once

#include "../texc_utils/http_response.h"

typedef struct {
    int port;
    char token[64];
} TexcClient;

/**
 * @brief Prepares the client for the texc server
 *
 * @param error -1 if the texc server is not running, -2 if the session token in
 * port file is not found, 0 if there no error
 *
 * @return The api client
 */
TexcClient client_prepare(int *error);

/**
 * @brief Makes a request to url at port
 *
 * @param client the client from client_prepare()
 * @param url url of the server. heap allocated.
 * @param error 0 if there is no error, 1 if connection fail, 2 if timeout
 *
 * @return response returned from server
 */
Response *client_request(TexcClient client, const char *url);
