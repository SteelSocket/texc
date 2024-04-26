#pragma once

#include "texc_utils/argparse.h"

#include <stdbool.h>

int subcmd_start_server(Args *args);
int subcmd_close_server(Args *args);

int subcmd_add_match(Args *args);
int subcmd_remove_match(Args *args);

int subcmd_list_words(Args *args);
