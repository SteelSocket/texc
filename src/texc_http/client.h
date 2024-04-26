#pragma once

#include "../texc_utils/http_response.h"

/**
 * @brief Makes a request to url at port
 *
 * @param port port of the server
 * @param url url of the server
 * @param error 0 if there is no error, 1 if connection fail, 2 if timeout
 *
 * @return response returned from server
 */
Response *client_request(int port, const char *url);
