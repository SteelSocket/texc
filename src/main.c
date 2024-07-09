#include "args.h"
#include "subcmds.h"

#include "texc_utils/utils.h"

int main(int argc, char *argv[]) {
    ArgParser *parser = args_init();
    Args *args = argparse_parse(parser, argc, argv);

    if (args == NULL) {
        return 1;
    }

    socket_init();

    const char *cmd_name = args->parser->name;
    int exit_code = 1;

    if (str_eq(cmd_name, "texc")) {
        exit_code = subcmd_start_server(args);
    } else if (str_eq(cmd_name, "add")) {
        exit_code = subcmd_add_exptexts(args);
    } else if (str_eq(cmd_name, "remove")) {
        exit_code = subcmd_remove_exptexts(args);
    } else if (str_eq(cmd_name, "close")) {
        exit_code = subcmd_close_server(args);
    } else if (str_eq(cmd_name, "list")) {
        exit_code = subcmd_list_exptexts(args);
    } else if (str_eq(cmd_name, "config")) {
        exit_code = subcmd_config_exptexts(args);
    }
    socket_cleanup();

    argparse_free_args(args);
    argparse_free(parser);
    return exit_code;
}
