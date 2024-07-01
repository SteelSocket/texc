#include "server.h"

#include <signal.h>

#include "../texc_data/data.h"

#include "../texc_utils/logger.h"
#include "../texc_utils/path.h"
#include "../texc_utils/socket.h"
#include "../texc_utils/thread.h"
#include "../texc_utils/filelock.h"

#define SERVER_MAX_THREADS 10

SOCKET __server_socket;
Thread *__server_thread;

Thread *__client_threads[SERVER_MAX_THREADS] = {NULL};
size_t __client_thread_len = 0;

FileLock __port_lock;

void __delete_port_file(void) {
    filelock_close(&__port_lock);

    char *port_path = data_get_port_file();
    remove(port_path);
    free(port_path);
}

void __delete_port_file_sig(int sig) {
    __delete_port_file();
    exit(1);
}

THREAD_CALLBACK __handle_client(void *data) {
    SOCKET *client = (SOCKET *)data;
    char *buffer;

    int size = socket_recv_all(*client, &buffer, 5);
    if (size == -1) {
        LOGGER_WARNING(socket_get_error());

        socket_close(*client);
        free(client);
        return THREAD_RETURN;
    }

    if (size == -2) {
        Response *response = response_new(504, "Timeout");
        socket_send(*client, response->raw, response->raw_len);
        socket_close(*client);
        free(client);
        response_free(response);

        LOGGER_WARNING("client socket recv timeout");
        return THREAD_RETURN;
    }

    Request *request = request_parse(buffer);
    free(buffer);

    if (request == NULL) {
        Response *response =
            response_new(504, "The request format is not correct");
        socket_send(*client, response->raw, response->raw_len);

        socket_close(*client);
        free(client);
        response_free(response);

        LOGGER_WARNING("client socket recv invalid request format");
        return THREAD_RETURN;
    }

    Response *response = __server_handle_api(request);
    request_free(request);

    socket_send(*client, response->raw, response->raw_len);
    socket_close(*client);
    free(client);
    response_free(response);

    return THREAD_RETURN;
}

THREAD_CALLBACK __server_start(void *_) {
    while (1) {
        SOCKET *client = malloc(sizeof(*client));
        *client = socket_accept(__server_socket);

        if (*client == INVALID_SOCKET) {
            LOGGER_WARNING(socket_get_error());
            free(client);
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

    return THREAD_RETURN;
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

    __port_lock = filelock_acquire(port_path);
    if (__port_lock == FILELOCK_ERROR) {
        free(port_path);
        return false;
    }

    char *port_str;
    str_format(port_str, "%d", port);
    filelock_write(__port_lock, port_str);
    free(port_str);

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

    FileLock lock = filelock_acquire(port_path);

    // texc is not running as port lock is not locked
    if (lock != FILELOCK_ERROR) {
        filelock_close(&lock);
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
