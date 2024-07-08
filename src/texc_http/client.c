#include "client.h"

#include <stdio.h>
#include <stdlib.h>

#include "../texc_data/data.h"

#include "../texc_utils/array.h"
#include "../texc_utils/filelock.h"
#include "../texc_utils/path.h"
#include "../texc_utils/socket.h"

const char *DEFAULT_CREQUEST =
    "GET %s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

TexcClient client_prepare(int *error) {
    TexcClient client = {0};
    *error = 0;

    char *port_path = data_get_port_file();
    if (!path_is_file(port_path)) {
        free(port_path);
        *error = -1;
        return (TexcClient){0};
    }

    FileLock lock = filelock_acquire(port_path);

    // texc is not running as port lock is not locked
    if (lock != FILELOCK_ERROR) {
        filelock_close(&lock);
        free(port_path);
        *error = -1;
        return (TexcClient){0};
    }

    char *contents = path_read_all(port_path);

    char *session_token;
    char *port_str = strtok_r(contents, "\n", &session_token);
    if (*session_token == '\0') {
        *error = -2;
        return (TexcClient){0};
    }

    client.port = atoi(port_str);
    if (client.port == 0) {
        *error = -1;
        return (TexcClient){0};
    }

    strncpy(client.token, session_token, (int)(array_len(client.token)) - 1);

    free(contents);
    free(port_path);

    return client;
}

Response *client_request(TexcClient client, const char *url) {
    SOCKET socket = socket_create();

    if (socket == INVALID_SOCKET ||
        socket_connect(socket, "127.0.0.1", client.port) == SOCKET_ERROR) {
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
               client.port);
        return NULL;
    }

    Response *ret_response = response_from(response_buffer);
    free(response_buffer);

    return ret_response;
}
