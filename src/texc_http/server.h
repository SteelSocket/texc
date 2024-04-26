#pragma once

#include <stdbool.h>

#include "../texc_utils/http_request.h"
#include "../texc_utils/http_response.h"

bool server_init(int port, const char *host);

void server_start();

void server_stop();

int server_get_active_port();

Response *__server_handle_api(Request *request);
