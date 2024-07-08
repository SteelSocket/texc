#pragma once

#include <stdbool.h>

#include "../texc_utils/http_request.h"
#include "../texc_utils/http_response.h"

/**
 * @brief Initializes the server
 * server_init() may fail if,
 * 1. There exists a socket server on the same port and host.
 * 2. Texc is already running on the same port
 *
 * @param port port of the server
 * @param host host of the server
 *
 * @return Boolean representing success.
 */
bool server_init(int port, const char *host);

/**
 * @brief Starts the server in a separate thread
 */
void server_start();

/**
 * @brief Stops the server thread
 */
void server_stop();


/**
 * @brief Handles the request as per api implemented in server_api.c
 *
 * @param request The http request
 * @return The http response
 */
Response *__server_handle_api(Request *request);
