#include "argparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "str.h"

typedef enum {
    // Sucess
    PS_SUCESS,
    // Help
    PS_HELP,

    // Invalid Flag
    PS_FLAG_REQUIRED,
    PS_FLAG_INVALID,
    PS_FLAG_REPEATED,
    PS_FLAG_CONFLICT,

    // Invalid flag Value
    PS_VALUE_NOT_FOUND,

    // Invalid position
    PS_POSITIONAL_REQUIRED,
    PS_POSITIONAL_EXCESS,
    PS_POSITIONAL_NOT_FOUND,

    // Subcommands
    PS_SUBCOMMAND_REQUIRED,

} __ParseStatus;

typedef struct {
    __ParseStatus status;

    // No extra arguments are returned if status is SUCESS
    Args *args;

    // Any one of the following will be valid non NULL Value in case
    // of error or help command is used
    const char *raw_arg;
    Option opt;
    Positional pos;

} __StatusPair;

ArgParser *argparse_init(const char *name, const char *help, Option options[],
                         size_t opt_len, Positional positionals[],
                         size_t pos_len) {
    ArgParser *parser = malloc(sizeof(ArgParser));
    parser->name = name;
    parser->help = help;

    parser->options = options;
    parser->opt_length = opt_len;

    parser->positionals = positionals;
    parser->pos_length = pos_len;

    parser->parent = NULL;
    parser->sub_parsers_len = 0;
    parser->sub_parsers = array_create(ArgParser *);

    return parser;
}

void argparse_add_subparser(ArgParser *parser, ArgParser *subparser) {
    array_resize_add(parser->sub_parsers, parser->sub_parsers_len, subparser,
                     ArgParser *);
    parser->sub_parsers[parser->sub_parsers_len - 1]->parent = parser;
}

void argparse_free(ArgParser *parser) {
    for (size_t i = 0; i < parser->sub_parsers_len; i++) {
        argparse_free(parser->sub_parsers[i]);
    }
    free(parser->sub_parsers);
    free(parser);
}

void argparse_free_args(Args *args) {
    free(args->opt_values);
    free(args->pos_values);
    free(args);
}

// ----------------------------
//       Help Command
// ----------------------------

size_t __get_desc_offset(ArgParser *parser) {
    size_t offset = strlen("-h,--help");

    for (size_t i = 0; i < parser->opt_length; i++) {
        size_t flag_len = strlen(parser->options[i].flags);
        if (offset < flag_len)
            offset = flag_len;
    }

    for (size_t i = 0; i < parser->sub_parsers_len; i++) {
        size_t name_len = strlen(parser->sub_parsers[i]->name);
        if (offset < name_len)
            offset = name_len;
    }

    return offset;
}

void __print_help_cmd_name(ArgParser *parser) {
    if (parser->parent == NULL) {
        printf("%s ", parser->name);
        return;
    }

    __print_help_cmd_name(parser->parent);
    printf("%s ", parser->name);
}

void __print_help_header(ArgParser *parser) {
    printf("Usage: ");
    __print_help_cmd_name(parser);

    if (parser->opt_length) {
        printf("[options] ");
    }

    if (parser->sub_parsers_len) {
        printf("{");
        for (size_t i = 0; i < parser->sub_parsers_len; i++) {
            if (i == parser->sub_parsers_len - 1) {
                printf("%s", parser->sub_parsers[i]->name);
                break;
            }
            printf("%s, ", parser->sub_parsers[i]->name);
        }
        printf("}");
    }

    for (size_t i = 0; i < parser->pos_length; i++) {
        if (i == parser->pos_length - 1) {
            printf("<%s>", parser->positionals[i].name);
            break;
        }
        printf("<%s> ", parser->positionals[i].name);
    }
    printf("\n\n%s\n\n", parser->help);
}

void __print_help_options(ArgParser *parser, size_t offset) {
    printf("Options:\n");
    printf("  %-*s %s\n", offset, "-h,--help", "Shows command line usage");
    for (size_t i = 0; i < parser->opt_length; i++) {
        printf("  %-*s %s\n", offset, parser->options[i].flags,
               parser->options[i].help);
    }
    printf("\n");
}

void __print_help_subcommands(ArgParser *parser, size_t offset) {
    printf("Subcommands:\n");
    for (size_t i = 0; i < parser->sub_parsers_len; i++) {
        printf("  %-*s %s\n", offset, parser->sub_parsers[i]->name,
               parser->sub_parsers[i]->help);
    }
    printf("\n");
}

void argparse_print_help(ArgParser *parser) {
    __print_help_header(parser);

    size_t desc_offset = __get_desc_offset(parser) + 4;

    __print_help_options(parser, desc_offset);
    if (parser->sub_parsers_len) {
        __print_help_subcommands(parser, desc_offset);
    }
}

// ----------------------------
//       Error Handling
// ----------------------------

void __handle_error(__StatusPair status) {
    ArgParser *parser = status.args->parser;
    argparse_print_help(parser);

    switch (status.status) {
        case PS_FLAG_REQUIRED: {
            printf("Error: The flag \"%s\" is required for the command \"",
                   status.opt.flags);
            __print_help_cmd_name(parser);
            printf("\b\"\n");
            break;
        }

        case PS_FLAG_INVALID: {
            printf("Error: The given flag \"%s\" is invalid\n", status.raw_arg);
            break;
        }

        case PS_FLAG_REPEATED: {
            printf("Error: The given flag \"%s\" is repeated\n",
                   status.raw_arg);
            break;
        }

        case PS_FLAG_CONFLICT: {
            printf(
                "Error: The given flag \"%s\" cannot be used with flag "
                "\"%s\"\n",
                status.opt.flags, status.raw_arg);
            break;
        }

        case PS_VALUE_NOT_FOUND: {
            printf("Error: The value for the flag \"%s\" is not found\n",
                   status.opt.flags);
            break;
        }

        case PS_POSITIONAL_REQUIRED: {
            printf("Error: Positional argument \"%s\" is required",
                   status.pos.name);
            break;
        }

        case PS_POSITIONAL_EXCESS: {
            printf("Error: Excessive positional argument \"%s\"\n",
                   status.raw_arg);
            break;
        }

        case PS_POSITIONAL_NOT_FOUND: {
            printf("Error: Positional argument not given after \"--\"");
            break;
        }

        case PS_SUBCOMMAND_REQUIRED: {
            printf("Error: Sub command is required for the command \"");
            __print_help_cmd_name(parser);
            printf("\b\"\n");
        }

        default:
            break;
    }
    printf("\n");
}

// ----------------------------
//          Parsing
// ----------------------------

// ----------------------------
//         Utilities
// ----------------------------
bool __flag_matches(const char *flags, const char *args) {
    char *cs1 = strdup(flags);
    char *cs2 = strdup(args);

    char *token1, *token2;
    char *saveptr1, *saveptr2;

    token1 = strtok_r(cs1, ",", &saveptr1);

    while (token1 != NULL) {
        token2 = strtok_r(cs2, ",", &saveptr2);

        while (token2 != NULL) {
            if (strcmp(token1, token2) == 0) {
                free(cs1);
                free(cs2);
                return true;
            }
            token2 = strtok_r(NULL, ",", &saveptr2);
        }
        token1 = strtok_r(NULL, ",", &saveptr1);
    }

    free(cs1);
    free(cs2);
    return false;
}

bool __flags_eq(const char *flags_1, const char *flags_2) {
    ;
    return false;
}
// ----------------------------

__StatusPair __handle_flag(Args *args, size_t arg_i, size_t opt_j) {
    Option opt = args->parser->options[opt_j];
    OptionValue *value = &args->opt_values[opt_j];
    const char *arg = args->parser->argv[arg_i];

    if (value->flags != NULL) {
        return (__StatusPair){
            .status = PS_FLAG_REPEATED,
            .args = args,
            .opt = opt,
            .raw_arg = arg,
        };
    }

    if (!opt.has_value) {
        *value = (OptionValue){
            .flags = opt.flags,
            .value = NULL,
            .found = true,
        };

        return (__StatusPair){
            .status = PS_SUCESS,
        };
    }

    if (args->parser->argc <= arg_i + 1) {
        return (__StatusPair){
            .status = PS_VALUE_NOT_FOUND, .args = args, .opt = opt};
    }

    // Handle if arg_value is -h/--help or some other flag?
    const char *arg_value = args->parser->argv[arg_i + 1];
    *value = (OptionValue){
        .flags = opt.flags,
        .value = arg_value,
        .found = true,
    };

    return (__StatusPair){
        .status = PS_SUCESS,
    };
}

__StatusPair __handle_pos(Args *args, size_t pos_i, size_t arg_i) {
    const char *arg = args->parser->argv[arg_i];

    if (str_eq(arg, "--")) {
        if (arg_i + 1 >= args->parser->argc)
            return (__StatusPair){
                .status = PS_POSITIONAL_NOT_FOUND,
                .args = args,
            };
        arg = args->parser->argv[arg_i + 1];
    }

    if (pos_i >= args->parser->pos_length) {
        return (__StatusPair){
            .status = PS_POSITIONAL_EXCESS,
            .args = args,
            .raw_arg = arg,
        };
    }

    args->pos_values[pos_i] = (PositionalValue){
        .position = pos_i,
        .value = arg,
    };

    return (__StatusPair){
        .status = PS_SUCESS,
    };
}

__StatusPair __handle_conflicts(Args *args) {
    for (size_t k = 0; k < args->parser->opt_length; k++) {
        Option opt = args->parser->options[k];
        if (opt.conflicting_flags == NULL) {
            continue;
        }

        for (size_t i = 0; i < args->parser->opt_length; i++) {
            if (i == k)
                continue;

            if (!args->opt_values[i].found || !args->opt_values[k].found) {
                continue;
            }

            if (!__flag_matches(args->opt_values[i].flags,
                                opt.conflicting_flags)) {
                continue;
            }
            return (__StatusPair){
                .status = PS_FLAG_CONFLICT,
                .args = args,
                .opt = opt,
                .raw_arg = args->opt_values[i].flags,
            };
        }
    }

    return (__StatusPair){
        .status = PS_SUCESS,
        args = args,
    };
}

__StatusPair __endparse_validate(Args *args) {
    for (size_t i = 0; i < args->parser->pos_length; i++) {
        Positional pos = args->parser->positionals[i];
        PositionalValue *value = &args->pos_values[i];

        if (value->value == NULL) {
            if (pos.required) {
                return (__StatusPair){
                    .status = PS_POSITIONAL_REQUIRED,
                    .args = args,
                    .pos = pos,
                };
            }
            value->value = pos.default_value;
        }
    }

    for (size_t i = 0; i < args->parser->opt_length; i++) {
        Option opt = args->parser->options[i];
        OptionValue *value = &args->opt_values[i];

        if (value->flags == NULL) {
            if (opt.is_required) {
                return (__StatusPair){
                    .status = PS_FLAG_REQUIRED, .args = args, .opt = opt};
            }

            value->flags = opt.flags;
            if (opt.default_value) {
                value->value = opt.default_value;
            }
        }
    }

    __StatusPair status = __handle_conflicts(args);
    return status;
}

__StatusPair __argparse_parse(ArgParser *parser, int argc, char **argv);

__StatusPair __parse_args(ArgParser *parser) {
    size_t cpos_index = 0;

    for (size_t i = 0; i < parser->sub_parsers_len; i++) {
        if (parser->argc < 2)
            break;

        if (*parser->argv[1] == '-')
            break;

        if (str_eq(parser->sub_parsers[i]->name, parser->argv[1])) {
            return __argparse_parse(parser->sub_parsers[i], parser->argc - 1,
                                    parser->argv + 1);
        }
    }

    Args *args = malloc(sizeof(Args));

    args->parser = parser;
    args->opt_values = calloc(parser->opt_length, sizeof(OptionValue));
    args->pos_values = calloc(parser->pos_length, sizeof(PositionalValue));

    // Requires subcommand to be given
    if (parser->opt_length == 0 && parser->pos_length == 0 &&
        parser->sub_parsers_len > 0) {
        return (__StatusPair){
            .status = PS_SUBCOMMAND_REQUIRED,
            .args = args,
        };
    }

    for (size_t i = 1; i < parser->argc; i++) {
        const char *arg = parser->argv[i];

        if (*arg != '-' || str_eq(arg, "--")) {
            __StatusPair status_p = __handle_pos(args, cpos_index, i);

            if (status_p.status == PS_SUCESS) {
                if (str_eq(arg, "--"))
                    i++;

                cpos_index++;
                continue;
            }
            return status_p;
        }

        if (__flag_matches("-h,--help", arg))
            return (__StatusPair){
                .status = PS_HELP,
                .args = args,
            };

        bool flag_found = false;

        for (size_t j = 0; j < parser->opt_length; j++) {
            Option opt = parser->options[j];

            if (!__flag_matches(opt.flags, arg)) {
                continue;
            }

            __StatusPair status_p = __handle_flag(args, i, j);
            if (status_p.status != PS_SUCESS) {
                return status_p;
            }

            // If the flag contains value
            if (args->opt_values[j].value != NULL)
                i++;

            flag_found = true;
            break;
        }

        if (!flag_found) {
            return (__StatusPair){
                .status = PS_FLAG_INVALID,
                .args = args,
                .raw_arg = arg,
            };
        }
    }

    return __endparse_validate(args);
}

__StatusPair __argparse_parse(ArgParser *parser, int argc, char **argv) {
    parser->argc = argc;
    parser->argv = argv;

    return __parse_args(parser);
}

Args *argparse_parse(ArgParser *parser, int argc, char **argv) {
    __StatusPair status_p = __argparse_parse(parser, argc, argv);
    Args *args = status_p.args;

    if (status_p.status == PS_HELP) {
        argparse_print_help(args->parser);
        return NULL;
    }

    if (status_p.status != PS_SUCESS) {
        __handle_error(status_p);
        argparse_free_args(args);
        return NULL;
    }
    return args;
}

// ----------------------------
//       Value Functions
// ----------------------------

const char *argparse_flag_get(Args *args, const char *flag) {
    for (size_t i = 0; i < args->parser->opt_length; i++) {
        if (__flag_matches(args->opt_values[i].flags, flag)) {
            return args->opt_values[i].value;
        }
    }
    return NULL;
}

bool argparse_flag_found(Args *args, const char *flag) {
    for (size_t i = 0; i < args->parser->opt_length; i++) {
        if (__flag_matches(args->opt_values[i].flags, flag)) {
            return args->opt_values[i].found;
        }
    }
    return false;
}

const char *argparse_positional_get(Args *args, const char *name) {
    for (size_t i = 0; i < args->parser->pos_length; i++) {
        if (str_eq(args->parser->positionals[i].name, name)) {
            return args->pos_values[i].value;
        }
    }
    return NULL;
}
