#pragma once

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#else
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Thread data structure
typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    pthread_t handle;
#endif
} Thread;

typedef struct {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
} Mutex;

// Function to be executed in a separate thread
typedef void (*ThreadFunction)(void *);

Thread *thread_create(ThreadFunction function, void *argument);

void thread_join(Thread *thread);

bool thread_is_running(Thread *thread);

bool thread_terminate(Thread *thread);

Mutex *mutex_create();

void mutex_lock(Mutex *mutex);

void mutex_unlock(Mutex *mutex);

void mutex_destroy(Mutex *mutex);

#ifdef UTILS_IMPLEMENTATION

Thread *thread_create(ThreadFunction function, void *argument) {
    Thread *thread = malloc(sizeof(Thread));

#ifdef _WIN32
    thread->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function,
                                  argument, 0, NULL);
#else
    pthread_create(&thread->handle, NULL, function, argument);
#endif

    return thread;
}

void thread_join(Thread *thread) {
#ifdef _WIN32
    WaitForSingleObject(thread->handle, INFINITE);
    CloseHandle(thread->handle);
#else
    pthread_join(thread->handle, NULL);
#endif
    free(thread);
}

bool thread_is_running(Thread *thread) {
#ifdef _WIN32
    DWORD exitCode;
    GetExitCodeThread(thread->handle, &exitCode);
    return exitCode == STILL_ACTIVE;
#else
    return pthread_kill(thread->handle, 0) == 0;
#endif
}

bool thread_terminate(Thread *thread) {
#ifdef _WIN32
    return TerminateThread(thread->handle, 0) != 0;
#else
    return pthread_cancel(thread->handle) == 0;
#endif
    free(thread);
}

Mutex *mutex_create() {
    Mutex *mutex = malloc(sizeof(Mutex));
#ifdef _WIN32
    InitializeCriticalSection(&mutex->mutex);
#else
    pthread_mutex_init(&mutex->mutex, NULL);
#endif
    return mutex;
}

void mutex_lock(Mutex *mutex) {
#ifdef _WIN32
    EnterCriticalSection(&mutex->mutex);
#else
    pthread_mutex_lock(&mutex->mutex);
#endif
}

void mutex_unlock(Mutex *mutex) {
#ifdef _WIN32
    LeaveCriticalSection(&mutex->mutex);
#else
    pthread_mutex_unlock(&mutex->mutex);
#endif
}

void mutex_destroy(Mutex *mutex) {
#ifdef _WIN32
    DeleteCriticalSection(&mutex->mutex);
#else
    pthread_mutex_destroy(&mutex->mutex);
#endif
    free(mutex);
}

#endif