#pragma once

#include <stdbool.h>
#include <errno.h>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

int socket_init();

void socket_cleanup();

SOCKET socket_create();

void socket_close(SOCKET socket);

int socket_shutdown(SOCKET socket);

int socket_bind(SOCKET socket, const char *address, int port);

bool socket_is_bound(SOCKET socket);

int socket_listen(SOCKET socket, int backlog);

SOCKET socket_accept(SOCKET socket);

int socket_connect(SOCKET socket, const char *address, int port);

int socket_send(SOCKET socket, const void *data, int length);

int socket_recv(SOCKET socket, char *buffer, int buffer_length,
                int timeout_sec);

int socket_recv_all(SOCKET socket, char **message, int timeout_sec);

const char *socket_get_error();

#ifdef UTILS_IMPLEMENTATION

#if defined(_WIN32) && defined(__MINGW32__) && !defined(__MINGW64__)
int inet_pton(int af, const char *src, void *dst) {
    ((struct in_addr *)dst)->s_addr = inet_addr(src);
    return 1;
}
#endif

int socket_init() {
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data);
#else
    return 0;  // No initialization needed on Unix-like systems
#endif
}

void socket_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

SOCKET socket_create() { 
#ifdef _WIN32
    return socket(AF_INET, SOCK_STREAM, 0);
#else
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return sock;
    
    int opt = 1;
    // Prevent "address already in use" due to TIME_WAIT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        socket_close(sock);
        return SOCKET_ERROR;
    }

    return sock;
#endif
}

void socket_close(SOCKET socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

int socket_shutdown(SOCKET socket) {
#ifdef _WIN32
    return shutdown(socket, SD_BOTH);
#else
    return shutdown(socket, SHUT_RDWR);
#endif
}

int socket_bind(SOCKET socket, const char *address, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
#ifdef _WIN32
    if (inet_pton(AF_INET, address, &(server_address.sin_addr)) != 1) {
#else
    if (inet_pton(AF_INET, address, &(server_address.sin_addr.s_addr)) != 1) {
#endif
        return SOCKET_ERROR;
    }
    server_address.sin_port = htons(port);

    return bind(socket, (struct sockaddr *)&server_address,
                sizeof(server_address));
}

bool socket_is_bound(SOCKET socket) {
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    return getsockname(socket, (struct sockaddr *)&addr, &addr_len) != -1;
}

int socket_listen(SOCKET socket, int backlog) {
    return listen(socket, backlog);
}

SOCKET socket_accept(SOCKET socket) {
    struct sockaddr_in client_address;
    int client_address_size = sizeof(client_address);

    return accept(socket, (struct sockaddr *)&client_address,
                  &client_address_size);
}

int socket_connect(SOCKET socket, const char *address, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
#ifdef _WIN32
    if (inet_pton(AF_INET, address, &(server_address.sin_addr)) != 1) {
#else
    if (inet_pton(AF_INET, address, &(server_address.sin_addr.s_addr)) != 1) {
#endif
        return SOCKET_ERROR;
    }
    server_address.sin_port = htons(port);

    return connect(socket, (struct sockaddr *)&server_address,
                   sizeof(server_address));
}

int socket_send(SOCKET socket, const void *data, int length) {
#ifdef _WIN32
    return send(socket, data, length, 0);
#else
    return send(socket, data, (size_t)length, 0);
#endif
}

int socket_recv(SOCKET socket, char *buffer, int buffer_length,
                int timeout_sec) {
    if (timeout_sec > 0) {
        struct timeval timeout;
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = 0;

        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(socket, &read_set);

        int read_timeout = select(0, &read_set, NULL, NULL, &timeout);

        if (read_timeout == -1)
            return -1;

        if (read_timeout == 0)
            return -2;
    }

#ifdef _WIN32
    int recv_size = recv(socket, buffer, buffer_length - 1, 0);
#else
    int recv_size = recv(socket, buffer, (size_t)(buffer_length - 1), 0);
#endif
    buffer[recv_size] = '\0';
    return recv_size;
}

int socket_recv_all(SOCKET socket, char **message, int timeout_sec) {
    char buffer[1024];
    char dummy[1024];

    // Initalize buffer
    *message = calloc(1, sizeof(char));
    int message_size = 0;

    while (1) {
        if (timeout_sec > 0) {
            struct timeval timeout;
            timeout.tv_sec = timeout_sec;
            timeout.tv_usec = 0;

            fd_set read_set;
            FD_ZERO(&read_set);
            FD_SET(socket, &read_set);

            int read_timeout = select(socket + 1, &read_set, NULL, NULL, &timeout);

            if (read_timeout == -1) {
                free(*message);
                return -1;
            }

            if (read_timeout == 0) {
                free(*message);
                return -2;
            }
        }
        int size = recv(socket, dummy, 1024, MSG_PEEK);

        if (size < 0)
            return size;

        int recv_size = socket_recv(socket, buffer, 1024, 0);

        message_size += recv_size;
        *message = realloc(*message, sizeof(char) * (message_size + 1));
        strcpy(*message + (message_size - recv_size), buffer);

        if (size <= recv_size) {
            break;
        }
    }

    return message_size;
}

const char *socket_get_error() {
#ifdef _WIN32
    int err_code = WSAGetLastError();
    if (err_code == 0) {
        return NULL;
    }

    static char error_msg[256];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  error_msg, sizeof(error_msg), NULL);

    return error_msg;
#else
    if (errno == 0) {
        return NULL;
    }

    return strerror(errno);
#endif
}

#endif
