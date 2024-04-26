#include "args.h"

#include "texc_utils/array.h"

// ---------------------------------------------------------
//                    Server Command
// ---------------------------------------------------------

Option __server_opts[] = {{
    .flags = "-p,--port",
    .help = "Port of the api server",
    .has_value = true,
    .default_value = "8000",
    .is_required = false,
}};

ArgParser *__args_server() {
    return argparse_init("server", "Starts the texc server", __server_opts,
                         array_len(__server_opts), NULL, 0);
}

// ---------------------------------------------------------
//                      Add Command
// ---------------------------------------------------------

Option __add_opts[] = {};

Positional __add_pos[] = {
    {
        .name = "text",
        .required = true,
    },
    {
        .name = "expand",
        .required = true,
    },
};

ArgParser *__args_add() {
    return argparse_init("add", "Adds a text-expansion to texc", __add_opts,
                         array_len(__add_opts), __add_pos,
                         array_len(__add_pos));
}

// ---------------------------------------------------------
//                      Remove Command
// ---------------------------------------------------------

Positional __remove_pos[] = {{
    .name = "identifier",
    .required = true,
}};

Option __remove_opts[] = {{
    .flags = "-i,--id",
    .help = "Removes the text-expansion by id",
    .has_value = false,
    .is_required = false,
}};

ArgParser *__args_remove() {
    return argparse_init("remove",
                         "Removes a text-expansion from the expand list",
                         __remove_opts, array_len(__remove_opts), __remove_pos,
                         array_len(__remove_pos));
}

// ---------------------------------------------------------
//        Remove Command
// ---------------------------------------------------------

ArgParser *__args_close() {
    return argparse_init("close", "Closes the running texc instance", NULL, 0,
                         NULL, 0);
}

// ---------------------------------------------------------
//                      List Command
// ---------------------------------------------------------

ArgParser *__args_list() {
    return argparse_init("list", "Prints all the text-expansions saved", NULL,
                         0, NULL, 0);
}

// ---------------------------------------------------------
//              Default(no subparser) Command
// ---------------------------------------------------------
Option __default_opts[] = {
    {
        .flags = "-p,--port",
        .help = "Port of the api server",
        .has_value = true,
        .default_value = "8000",
        .is_required = false,
    },
    {
        .flags = "-v,--version",
        .help = "Returns texc version",
        .has_value = false,
        .is_required = false,
    },
};

ArgParser *__args_default() {
    return argparse_init("texc", "Command line app to expand typed text",
                         __default_opts, array_len(__default_opts), NULL, 0);
}

ArgParser *args_init() {
    ArgParser *texc_parser = __args_default();

    argparse_add_subparser(texc_parser, __args_server());
    argparse_add_subparser(texc_parser, __args_close());
    argparse_add_subparser(texc_parser, __args_add());
    argparse_add_subparser(texc_parser, __args_remove());
    argparse_add_subparser(texc_parser, __args_list());

    return texc_parser;
}
