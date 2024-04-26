#include "client.h"

#include <stdio.h>
#include <stdlib.h>

#include "../texc_data/data.h"

#include "../texc_utils/socket.h"

const char *DEFAULT_CREQUEST =
    "GET %s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

Response *client_request(int port, const char *url) {
    SOCKET socket = socket_create();

    if (socket == INVALID_SOCKET ||
        socket_connect(socket, "127.0.0.1", port) == SOCKET_ERROR) {
        printf("Error: texc server is not started please run texc first\n");
        return NULL;
    }

    size_t buffer_len = strlen(DEFAULT_CREQUEST) + strlen(url) + 1;
    char *buffer = malloc(buffer_len * sizeof(char));

    sprintf(buffer, DEFAULT_CREQUEST, url);
    socket_send(socket, buffer, buffer_len);
    free(buffer);

    char *response_buffer;
    int bytes_read = socket_recv_all(socket, &response_buffer, 5);

    // Timeout
    if (bytes_read == -2 || bytes_read == -1) {
        printf("Error: Timeout (May be due to another app using port \"%d\")\n",
               port);
        return NULL;
    }

    Response *ret_response = response_from(response_buffer);
    free(response_buffer);

    return ret_response;
}
