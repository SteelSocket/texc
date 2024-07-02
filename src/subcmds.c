#include "subcmds.h"

#include "texc_data/data.h"
#include "texc_keyhook/keyhook.h"

#include "texc_http/client.h"
#include "texc_http/server.h"

#include "texc_utils/argparse.h"
#include "texc_utils/logger.h"
#include "texc_utils/str.h"
#include "texc_utils/url.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define __check_server_running(port)                                 \
    do {                                                             \
        port = server_get_active_port();                             \
        if (port <= 0) {                                             \
            printf(                                                  \
                "Error: texc server is not started please run texc " \
                "first\n");                                          \
            return 1;                                                \
        }                                                            \
    } while (0)

char *__execute_request(int port, const char *url) {
    int error;
    Response *response = client_request(port, url);
    if (response == NULL) {
        return NULL;
    }

    if (response->status_code == 400) {
        printf("Error: %s\n", response->body);
        response_free(response);
        return NULL;
    }
    char *body = strdup(response->body);
    response_free(response);
    return body;
}

void __run_in_background() {
#ifdef _WIN32
    SetEnvironmentVariable("TEXC_BACKGROUND", "true");
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    if (CreateProcess(NULL, GetCommandLineA(), NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
#endif
}

bool __append_identifier(Args *args, char **url, const char *identifier,
                         const char *print_info) {
    char *encoded = url_encode(identifier);

    if (argparse_flag_found(args, "--id")) {
        str_rformat(*url, "id=%s", encoded);
        printf("%s by id: \"%s\"\n", print_info, identifier);
        free(encoded);
        return true;
    } else if (argparse_flag_found(args, "--group")) {
        str_rformat(*url, "group=%s", encoded);
        printf("%s by group: \"%s\"\n", print_info, identifier);
        free(encoded);
        return true;
    } else {
        str_rformat(*url, "match=%s", encoded);
        printf("%s by match: \"%s\"\n", print_info, identifier);
        free(encoded);
        return true;
    }

    free(encoded);
    return false;
}

int subcmd_start_server(Args *args) {
    if (argparse_flag_found(args, "--version")) {
        printf("version %s\n", TEXC_VERSION);
        return 1;
    }

    int port = atoi(argparse_flag_get(args, "--port"));
    if (port <= 0) {
        argparse_print_help(args->parser);
        printf("Error: Invalid port number. Port number must be above 0");
        return 1;
    }

    if (server_get_active_port() != -1) {
        printf("Error: texc is already running");
        return 1;
    }

    if (!data_init())
        return 1;

    // Check if the server is able to start
    if (!server_init(port, "127.0.0.1")) {
        LOGGER_FORMAT_LOG(LOGGER_ERROR,
                          "Error: Failed to initialize server. Some other "
                          "application maybe using port '%d'. "
                          "Please change the port using --port option",
                          port);
        data_free(false);
        return 1;
    }

#if defined(NDEBUG) && defined(_WIN32)
    if (getenv("TEXC_BACKGROUND") == NULL && !argparse_flag_found(args, "-fg")) {
        LOGGER_INFO("(windows) Running texc in the background");
        data_free(false);
        __run_in_background();
        return 0;
    }
#endif

    server_start();

    keyhook_run();  // Exits When keyhook_raw_quit() is called

    server_stop();
    data_free(true);

    return 0;
}

int subcmd_close_server(Args *args) {
    int port;
    __check_server_running(port);

    char *body = __execute_request(port, "/close");
    if (body == NULL) {
        return 1;
    }
    printf("%s\n", body);
    free(body);

    return 0;
}

int subcmd_add_exptexts(Args *args) {
    int port;
    __check_server_running(port);

    char *match = url_encode(argparse_positional_get(args, "text"));
    char *expand = url_encode(argparse_positional_get(args, "expand"));
    char *enabled = url_encode(argparse_flag_get(args, "--enable"));
    char *group = url_encode(argparse_flag_get(args, "--group"));

    char *url;
    str_format(url, "/add?match=%s&expand=%s&enabled=%s&group=%s", match,
               expand, enabled, group);

    printf("Adding %s -> %s\n", argparse_positional_get(args, "text"),
           argparse_positional_get(args, "expand"));

    char *body = __execute_request(port, url);
    int ret_code = 1;

    if (body != NULL) {
        printf("%s\n", body);
        free(body);
        ret_code = 0;
    }

    free(url);
    free(match);
    free(expand);
    free(enabled);
    free(group);

    return ret_code;
}

int subcmd_remove_exptexts(Args *args) {
    int port;
    __check_server_running(port);

    char *url;
    str_mcpy(url, "/remove?");

    const char *identifier = argparse_positional_get(args, "identifier");
    bool is_valid_iden =
        __append_identifier(args, &url, identifier, "Removing text-expansions");

    if (!is_valid_iden) {
        // Currently this case will not occur
        printf("Invalid Identifier\n");
        free(url);
        return 1;
    }

    char *body = __execute_request(port, url);
    if (body == NULL) {
        free(url);
        return 1;
    }
    printf("%s\n", body);

    free(body);
    free(url);

    return 0;
}

int subcmd_list_exptexts(Args *args) {
    int port;
    __check_server_running(port);

    char *body = __execute_request(port, "/list");
    if (body == NULL) {
        return 1;
    }
    if (str_count(body, '\n') == 1) {
        free(body);
        printf("Empty! no text-expansions in texc\n");
        return 0;
    }

    printf("%s\n", body);
    free(body);
    return 0;
}

int subcmd_config_exptexts(Args *args) {
    int port;
    __check_server_running(port);

    char *url;
    str_mcpy(url, "/config?");

    const char *identifier = argparse_positional_get(args, "identifier");
    bool is_valid_iden = __append_identifier(args, &url, identifier,
                                             "Configuring text-expansions");

    if (!is_valid_iden) {
        // Currently this case will not occur
        printf("Invalid Identifier\n");
        free(url);
        return 1;
    }

    if (argparse_flag_found(args, "--enable")) {
        str_rformat(url, "&enabled=%s", argparse_flag_get(args, "--enable"));
    }

    char *body = __execute_request(port, url);
    if (body == NULL) {
        free(url);
        return 1;
    }
    printf("%s\n", body);
    free(body);
    free(url);

    return 0;
}
