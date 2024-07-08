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

#define __check_server_running(client)                               \
    do {                                                             \
        int cerror;                                                  \
        client = client_prepare(&cerror);                            \
        if (cerror != 0) {                                           \
            printf(                                                  \
                "Error: texc server is not started please run texc " \
                "first\n");                                          \
            return 1;                                                \
        }                                                            \
    } while (0)

char *__execute_request(TexcClient client, UrlBuilder ub) {
    char *url = url_builder_prepare(ub);
    int error;
    Response *response = client_request(client, url);
    free(url);

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

bool __append_identifier(Args *args, UrlBuilder *ub, const char *identifier,
                         const char *print_info) {
    if (argparse_flag_found(args, "--id")) {
        url_builder_add_param(ub, "id", identifier);
        printf("%s by id: \"%s\"\n", print_info, identifier);
        return true;
    } else if (argparse_flag_found(args, "--group")) {
        url_builder_add_param(ub, "group", identifier);
        printf("%s by group: \"%s\"\n", print_info, identifier);
        return true;
    } else {
        url_builder_add_param(ub, "match", identifier);
        printf("%s by match: \"%s\"\n", print_info, identifier);
        return true;
    }
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
        printf("Error: Invalid port number. Port number must be above 0\n");
        return 1;
    }

    int cerror;
    client_prepare(&cerror);

    if (cerror == 0) {
        printf("Error: texc is already running\n");
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
    if (getenv("TEXC_BACKGROUND") == NULL &&
        !argparse_flag_found(args, "-fg")) {
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
    TexcClient client;
    __check_server_running(client);

    UrlBuilder ub = url_builder_new("/close");
    url_builder_add_param(&ub, "token", client.token);

    char *body = __execute_request(client, ub);
    url_builder_free(&ub);

    if (body == NULL) {
        return 1;
    }

    printf("%s\n", body);
    free(body);

    return 0;
}

int subcmd_add_exptexts(Args *args) {
    TexcClient client;
    __check_server_running(client);

    UrlBuilder ub = url_builder_new("/add");
    url_builder_add_param(&ub, "token", client.token);

    url_builder_add_param(&ub, "match", argparse_positional_get(args, "match"));
    url_builder_add_param(&ub, "expand",
                          argparse_positional_get(args, "expand"));
    url_builder_add_param(&ub, "enabled", argparse_flag_get(args, "--enable"));
    url_builder_add_param(&ub, "group", argparse_flag_get(args, "--group"));

    printf("Adding %s -> %s\n", argparse_positional_get(args, "match"),
           argparse_positional_get(args, "expand"));

    char *body = __execute_request(client, ub);
    url_builder_free(&ub);

    if (body != NULL) {
        printf("%s\n", body);
        free(body);
        return 0;
    }
    return 1;
}

int subcmd_remove_exptexts(Args *args) {
    TexcClient client;
    __check_server_running(client);

    UrlBuilder ub = url_builder_new("/remove");
    url_builder_add_param(&ub, "token", client.token);

    const char *identifier = argparse_positional_get(args, "identifier");

    bool is_valid_iden =
        __append_identifier(args, &ub, identifier, "Removing text expansions");

    if (!is_valid_iden) {
        // Currently this case will not occur
        printf("Invalid Identifier\n");
        url_builder_free(&ub);
        return 1;
    }

    char *body = __execute_request(client, ub);
    url_builder_free(&ub);

    if (body == NULL) {
        return 1;
    }
    printf("%s\n", body);
    free(body);

    return 0;
}

int subcmd_list_exptexts(Args *args) {
    TexcClient client;
    __check_server_running(client);

    UrlBuilder ub = url_builder_new("/list");
    url_builder_add_param(&ub, "token", client.token);

    if (argparse_flag_found(args, "--columns")) {
        url_builder_add_param(&ub, "columns",
                              argparse_flag_get(args, "--columns"));
    }

    if (argparse_flag_found(args, "--where")) {
        url_builder_add_param(&ub, "condition",
                              argparse_flag_get(args, "--where"));
    }

    char *body = __execute_request(client, ub);
    url_builder_free(&ub);

    if (body == NULL) {
        return 1;
    }

    if (str_count(body, '\n') == 1) {
        free(body);
        printf("Empty!\n");
        return 0;
    }

    printf("%s\n", body);
    free(body);
    return 0;
}

int subcmd_config_exptexts(Args *args) {
    TexcClient client;
    __check_server_running(client);

    UrlBuilder ub = url_builder_new("/list");
    url_builder_add_param(&ub, "token", client.token);

    const char *identifier = argparse_positional_get(args, "identifier");
    bool is_valid_iden = __append_identifier(args, &ub, identifier,
                                             "Configuring text expansions");

    if (!is_valid_iden) {
        // Currently this case will not occur
        printf("Invalid Identifier\n");
        url_builder_free(&ub);
        return 1;
    }

    if (argparse_flag_found(args, "--enable")) {
        url_builder_add_param(&ub, "enabled",
                              argparse_flag_get(args, "--enable"));
    }

    char *body = __execute_request(client, ub);
    url_builder_free(&ub);

    if (body == NULL) {
        return 1;
    }
    printf("%s\n", body);
    free(body);

    return 0;
}
