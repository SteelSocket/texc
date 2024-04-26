#pragma once

#include "../expandtext.h"
#include "data_sql.h"

char *data_io_expandtext_as_csv(ExpandText *exptext, DataSqlRow row);

void data_io_save();

void data_io_load();
