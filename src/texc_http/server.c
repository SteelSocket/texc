#include "server.h"

#include <signal.h>

#include "../texc_data/data.h"

#include "../texc_utils/logger.h"
#include "../texc_utils/path.h"
#include "../texc_utils/socket.h"
#include "../texc_utils/thread.h"

#define SERVER_MAX_THREADS 10

SOCKET __server_socket;
Thread *__server_thread;

Thread *__client_threads[SERVER_MAX_THREADS] = {NULL};
size_t __client_thread_len = 0;

FILE *__port_file;

void __delete_port_file(void) {
    fclose(__port_file);
    char *port_path = data_get_port_file();
    remove(port_path);
    free(port_path);
}

void __delete_port_file_sig(int sig) {
    __delete_port_file();
    exit(1);
}

void __handle_client(void *data) {
    SOCKET client = (SOCKET)data;
    char *buffer;

    int size = socket_recv_all(client, &buffer, 5);
    if (size == -1) {
        LOGGER_WARNING(socket_get_error());
        socket_close(client);
        return;
    }

    if (size == -2) {
        Response *response = response_new(504, "Timeout");
        socket_send(client, response->raw, response->raw_len);
        socket_close(client);
        response_free(response);

        LOGGER_WARNING("client socket recv timeout");
        return;
    }

    Request *request = request_parse(buffer);
    free(buffer);

    if (request == NULL) {
        Response *response =
            response_new(504, "The request format is not correct");
        socket_send(client, response->raw, response->raw_len);

        socket_close(client);
        response_free(response);

        LOGGER_WARNING("client socket recv invalid request format");
        return;
    }

    Response *response = __server_handle_api(request);
    request_free(request);

    socket_send(client, response->raw, response->raw_len);
    socket_close(client);
    response_free(response);
}

void __server_start(void *_) {
    while (1) {
        SOCKET client = socket_accept(__server_socket);
        if (client == INVALID_SOCKET) {
            LOGGER_WARNING(socket_get_error());
            continue;
        }

        while (__client_thread_len == SERVER_MAX_THREADS) {
            int nsize = 0;
            for (size_t i = 0; i < __client_thread_len; i++) {
                if (!thread_is_running(__client_threads[i])) {
                    thread_join(__client_threads[i]);
                    continue;
                }
                __client_threads[nsize++] = __client_threads[i];
            }
            __client_thread_len = nsize;
        }

        __client_threads[__client_thread_len++] =
            thread_create(__handle_client, (void *)client);
    }
}

bool server_init(int port, const char *host) {
    __server_socket = socket_create();
    if (__server_socket == INVALID_SOCKET) {
        socket_cleanup();
        return false;
    }

    if (socket_bind(__server_socket, host, port) == SOCKET_ERROR) {
        socket_close(__server_socket);
        socket_cleanup();
        return false;
    }

    if (socket_listen(__server_socket, 5) == SOCKET_ERROR) {
        socket_close(__server_socket);
        socket_cleanup();
        return false;
    }

    char *port_path = data_get_port_file();

    // Lock the file and prevent deletion while texc is running
    __port_file = fopen(port_path, "w");

    fprintf(__port_file, "%d", port);
    fflush(__port_file);
    free(port_path);

    // Set up handles to delete port file on exit
    atexit(__delete_port_file);
    signal(SIGINT, __delete_port_file_sig);
    signal(SIGTERM, __delete_port_file_sig);

    return true;
}

void server_start() {
    __server_thread = thread_create(__server_start, NULL);
    LOGGER_INFO("server started");
}

void server_stop() {
    thread_terminate(__server_thread);
    socket_close(__server_socket);

    for (size_t i = 0; i < __client_thread_len; i++) {
        thread_join(__client_threads[i]);
    }
    LOGGER_INFO("server stoped");
}

int server_get_active_port() {
    char *port_path = data_get_port_file();
    if (!path_is_file(port_path)) {
        free(port_path);
        return -1;
    }

    // texc is not running as port file is not locked
    if (remove(port_path) == 0) {
        free(port_path);
        return -1;
    }

    char *contents = path_read_all(port_path);
    int port = atoi(contents);

    free(contents);
    free(port_path);

    if (port < 0)
        return 0;
    return port;
}
