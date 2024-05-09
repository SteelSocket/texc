#define UTILS_IMPLEMENTATION
#include "texc_utils/array.h"
#include "texc_utils/csv.h"
#include "texc_utils/http_request.h"
#include "texc_utils/http_response.h"
#include "texc_utils/logger.h"
#include "texc_utils/path.h"
#include "texc_utils/socket.h"
#include "texc_utils/str.h"
#include "texc_utils/thread.h"
#include "texc_utils/url.h"

#include "args.h"
#include "subcmds.h"

int main(int argc, char *argv[]) {
    ArgParser *parser = args_init();
    Args *args = argparse_parse(parser, argc, argv);

    if (args == NULL) {
        return 1;
    }

    socket_init();

    const char *cmd_name = args->parser->name;
    int exit_code;

    if (str_eq(cmd_name, "texc") || str_eq(cmd_name, "server")) {
        exit_code = subcmd_start_server(args);
    } else if (str_eq(cmd_name, "add")) {
        exit_code = subcmd_add_match(args);
    } else if (str_eq(cmd_name, "remove")) {
        exit_code = subcmd_remove_match(args);
    } else if (str_eq(cmd_name, "close")) {
        exit_code = subcmd_close_server(args);
    } else if (str_eq(cmd_name, "list")) {
        exit_code = subcmd_list_exptexts(args);
    } else if (str_eq(cmd_name, "config")) {
        exit_code = subcmd_config_match(args);
    }
    socket_cleanup();

    argparse_free_args(args);
    argparse_free(parser);
    return exit_code;
}
