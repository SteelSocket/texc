#include "data.h"
#include "data_io.h"
#include "data_sql.h"

#include "../texc_utils/logger.h"
#include "../texc_utils/path.h"

#include <stdio.h>
#include <stdlib.h>

Data data = {0};

char *data_get_dir() {
#ifdef _WIN32
    char *data_dir = getenv("LOCALAPPDATA");
#else
    char *data_dir = getenv("HOME");
#endif
    if (data_dir == NULL) {
        return NULL;
    }

    char *path = path_join(data_dir, "texc");

    return path;
}

char *data_get_port_file() {
    char *data_dir = data_get_dir();
    char *port_path = path_join(data_dir, "port");

    free(data_dir);
    return port_path;
}

bool data_init() {
    // Create texc dir in data folder
    char *data_dir = data_get_dir();
    if (!path_exists(data_dir) || !path_is_dir(data_dir)) {
        path_make_dir(data_dir);
    }

#ifdef NDEBUG
    char *log_file = path_join(data_dir, "logs.txt");
    logger_set_logfile(log_file);
    logger_set_level(LOGGER_LEVEL_ERROR);
    free(log_file);
#endif

    free(data_dir);

    data.mutex = mutex_create();

    data.exptext_cap = 1;
    data.exptext_len = 0;
    data.exptexts = malloc(data.exptext_cap * sizeof(ExpandText *));

    if (!data_sql_init())
        return false;

    data_io_load();
    return true;
}

void data_free() {
    mutex_lock(data.mutex);
    data_io_save();

    int iter_count = 0;
    int i = 0;
    while (iter_count < data.exptext_len) {
        if (data.exptexts[i] == NULL) {
            i++;
            continue;
        }

        expandtext_free(data.exptexts[i]);
        iter_count++;
        i++;
    }

    free(data.exptexts);
    data_sql_free();

    mutex_unlock(data.mutex);
    mutex_destroy(data.mutex);
}
