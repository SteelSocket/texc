#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#define FileLock FILE *
#define FILELOCK_ERROR NULL
#else
#define FileLock int
#define FILELOCK_ERROR -1
#endif

FileLock filelock_acquire(const char *file_path);

void filelock_write(FileLock lock, const char *str);

void filelock_close(FileLock *lock);

#ifdef UTILS_IMPLEMENTATION

#include <string.h>
#include "path.h"

FileLock filelock_acquire(const char *file_path) {
    FileLock lock;

#ifdef _WIN32
    // We could use windows.h LockFileEx but it prevents us
    // from writting to the file

    if (path_is_file(file_path) && remove(file_path) != 0) {
        return FILELOCK_ERROR;
    }

    lock = fopen(file_path, "w");
#else
    lock = open(file_path, O_RDWR | O_CREAT, 0666);
    if (lock == -1) {
        return FILELOCK_ERROR;
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(lock, F_SETLK, &fl) == -1) {
        return FILELOCK_ERROR;
    }
#endif
    return lock;
}


void filelock_write(FileLock lock, const char *str) {
#ifdef _WIN32
    fprintf(lock, "%s", str);
    fflush(lock);
#else
    write(lock, str, strlen(str));
    fsync(lock);
#endif
}

void filelock_close(FileLock *lock) {
#ifdef _WIN32
    if (*lock != FILELOCK_ERROR) {
        fclose(*lock);
        *lock = FILELOCK_ERROR;
    }
#else
    if (*lock != FILELOCK_ERROR) {
        struct flock fl;
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fcntl(*lock, F_SETLK, &fl);
        close(*lock);

        *lock = FILELOCK_ERROR;
    }
#endif
}
#endif
