#pragma once

#include "texc_utils/argparse.h"

#include <stdbool.h>

/**
 * @brief Starts texc server
 *
 * @param args The argparse args
 * @return exit code
 */
int subcmd_start_server(Args *args);

/**
 * @brief Closes texc server
 *
 * @param args The argparse args
 * @return exit code
 */
int subcmd_close_server(Args *args);

/**
 * @brief Adds a text expansion to texc
 *
 * @param args The argparse args
 * @return exit code
 */
int subcmd_add_exptexts(Args *args);

/**
 * @brief Removes text expansion(s) from texc
 *
 * @param args The argparse args
 * @return exit code
 */
int subcmd_remove_exptexts(Args *args);

/**
 * @brief Prints all text expansions from texc
 *
 * @param args The argparse args
 * @return exit code
 */
int subcmd_list_exptexts(Args *args);

/**
 * @brief Changes attribute of existing text expansion(s)
 *
 * @param args The argparse args
 * @return exit code
 */
int subcmd_config_exptexts(Args *args);
