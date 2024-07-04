#include "args.h"

#include "texc_utils/array.h"

// ---------------------------------------------------------
//                      Add Command
// ---------------------------------------------------------

Option __add_opts[] = {
    {
        .flags = "-e,--enable",
        .help = "Set enable or disable",
        .has_value = true,
        .default_value = "true",
        .is_required = false,
    },
    {
        .flags = "-g,--group",
        .help = "Set the text expansion group",
        .has_value = true,
        .default_value = "all",
        .is_required = false,
    },
};

Positional __add_pos[] = {
    {
        .name = "match",
        .required = true,
    },
    {
        .name = "expand",
        .required = true,
    },
};

ArgParser *__args_add() {
    return argparse_init("add", "Adds a text expansion to texc", __add_opts,
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

Option __remove_opts[] = {
    {
        .flags = "-i,--id",
        .help = "Removes the text expansion by id",
        .has_value = false,
        .is_required = false,
    },
    {
        .flags = "-g,--group",
        .help = "Removes the text expansion by group",
        .has_value = false,
        .is_required = false,
    },
};

ArgParser *__args_remove() {
    return argparse_init("remove",
                         "Removes a text expansion from the expand list",
                         __remove_opts, array_len(__remove_opts), __remove_pos,
                         array_len(__remove_pos));
}

// ---------------------------------------------------------
//                      Close Command
// ---------------------------------------------------------

ArgParser *__args_close() {
    return argparse_init("close", "Closes the running texc instance", NULL, 0,
                         NULL, 0);
}

// ---------------------------------------------------------
//                      List Command
// ---------------------------------------------------------

Option __list_opts[] = {
    {
        .flags = "-c,--columns",
        .help = "The columns to fetch from sql table",
        .has_value = true,
        .is_required = false,
    },
    {
        .flags = "-w,--where",
        .help = "The sql condition for the fetch",
        .has_value = true,
        .is_required = false,
    },
};

ArgParser *__args_list() {
    return argparse_init("list", "Display text expansions in texc as csv", __list_opts,
                            array_len(__list_opts), NULL, 0);
}

// ---------------------------------------------------------
//                      Config Command
// ---------------------------------------------------------

Option __config_opts[] = {
    {
        .flags = "-i,--id",
        .help = "Sets config by id",
        .has_value = false,
        .is_required = false,
    },
    {
        .flags = "-g,--group",
        .help = "Sets the config by group",
        .has_value = false,
        .is_required = false,
    },
    {
        .flags = "-e,--enable",
        .help = "Enables/Disables the text expansion",
        .has_value = true,
        .is_required = false,
    },
};

Positional __config_pos[] = {{
    .name = "identifier",
    .required = true,
}};

ArgParser *__args_config() {
    return argparse_init("config", "Modifies the config of a text expansion",
                         __config_opts, array_len(__config_opts), __config_pos,
                         array_len(__config_pos));
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
#ifdef _WIN32
    {
        .flags = "-fg,--foreground",
        .help = "Run texc in the foreground",
        .has_value = false,
        .is_required = false,
    },
#endif
};

ArgParser *__args_default() {
    return argparse_init("texc", "Command line app to expand typed text",
                         __default_opts, array_len(__default_opts), NULL, 0);
}

ArgParser *args_init() {
    ArgParser *texc_parser = __args_default();

    argparse_add_subparser(texc_parser, __args_close());
    argparse_add_subparser(texc_parser, __args_add());
    argparse_add_subparser(texc_parser, __args_remove());
    argparse_add_subparser(texc_parser, __args_list());
    argparse_add_subparser(texc_parser, __args_config());

    return texc_parser;
}
