#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    // Required
    const char *flags;
    const char *help;

    // Optional
    // Ignored if has_value == false
    const char *default_value;

    // Returns error if the flag is not specified
    bool is_required;
    // Only the flag needs to be given not the value
    bool has_value;

    // The flags which cannot be used with this option
    const char *conflicting_flags;

} Option;

typedef struct {
    // The flags of the option
    const char *flags;
    // Sets to default if default_value is specified and it is not given in argv
    const char *value;

    bool found;
} OptionValue;

typedef struct {
    const char *name;

    const char *default_value;

    bool required;

} Positional;

typedef struct {
    // Index relative to other Positional Values
    size_t position;
    // Value if the position, only active if is_nargs is not specified in
    // Positional
    const char *value;
} PositionalValue;

typedef struct ArgParser {
    const char *name;
    const char *help;

    int argc;
    char **argv;

    // --------------
    //    Options
    // --------------
    Option *options;
    size_t opt_length;

    // --------------
    //  Positional
    // --------------
    Positional *positionals;
    size_t pos_length;

    // --------------
    //   Subparsers
    // --------------
    struct ArgParser *parent;
    size_t sub_parsers_len;
    struct ArgParser **sub_parsers;

} ArgParser;

typedef struct {
    ArgParser *parser;
    OptionValue *opt_values;
    PositionalValue *pos_values;
} Args;

ArgParser *argparse_init(const char *name, const char *help, Option options[],
                         size_t opt_len, Positional positionals[],
                         size_t pos_len);

void argparse_add_subparser(ArgParser *parser, ArgParser *subparser);

Args *argparse_parse(ArgParser *parser, int argc, char **argv);

void argparse_free(ArgParser *parser);

void argparse_free_args(Args *args);

void argparse_print_help(ArgParser *parser);

const char *argparse_flag_get(Args *args, const char *flag);

bool argparse_flag_found(Args *args, const char *flag);

const char *argparse_positional_get(Args *args, const char *name);
