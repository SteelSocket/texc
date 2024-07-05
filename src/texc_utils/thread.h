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
struct __ThreadData;

typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    pthread_t handle;
#endif
    struct __ThreadData *__data;
    bool __is_running;
} Thread;

typedef struct {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
} Mutex;

// Function to be executed in a separate thread
typedef void (*ThreadCallback)(void *);

Thread *thread_create(ThreadCallback function, void *argument);

void thread_join(Thread *thread);

bool thread_is_running(Thread *thread);

bool thread_terminate(Thread *thread);

Mutex *mutex_create();

void mutex_lock(Mutex *mutex);

void mutex_unlock(Mutex *mutex);

void mutex_destroy(Mutex *mutex);

#ifdef UTILS_IMPLEMENTATION

#ifdef _WIN32
#define __THREAD_CALLBACK DWORD WINAPI
#define __THREAD_RETURN 0
#else
#define __THREAD_CALLBACK void *
#define __THREAD_RETURN NULL
#endif

typedef struct __ThreadData {
    Thread *thread;
    ThreadCallback callback;
    void *data;
} __ThreadData;

__THREAD_CALLBACK
__thread_callback_wrapper(void *thread_data) {
    __ThreadData *data = (__ThreadData *)thread_data;
    data->thread->__is_running = true;

    data->callback(data->data);

    data->thread->__is_running = false;
    data->thread->__data = NULL;
    free(data);
    return __THREAD_RETURN;
}

Thread *thread_create(ThreadCallback function, void *argument) {
    Thread *thread = malloc(sizeof(Thread));
    thread->__is_running = false;

    __ThreadData *data = malloc(sizeof(__ThreadData));
    data->thread = thread;
    data->callback = function;
    data->data = argument;

    thread->__data = data;

#ifdef _WIN32
    thread->handle =
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)__thread_callback_wrapper,
                     data, 0, NULL);
#else
    pthread_create(&thread->handle, NULL, __thread_callback_wrapper, data);
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

bool thread_is_running(Thread *thread) { return thread->__is_running; }

bool thread_terminate(Thread *thread) {
#ifdef _WIN32
    return TerminateThread(thread->handle, 0) != 0;
#else
    return pthread_cancel(thread->handle) == 0;
#endif
    if (thread->__data != NULL)
        free(thread->__data);
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
