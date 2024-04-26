#pragma once

#include <stdint.h>

#define array_len(array) sizeof(array) / sizeof(array[0])

#define array_find(findex, array, size, condition)      \
    do {                                                \
        findex = -1;                                    \
        for (size_t index = 0; index < size; index++) { \
            if (condition) {                            \
                findex = index;                         \
                break;                                  \
            }                                           \
        }                                               \
    } while (0)

// Dynamic Array Only functions

#define array_create(type) malloc(1 * sizeof(type))

#define array_increase_size(array, size, element_size) \
    array = realloc(array, (size + 1) * element_size)

#define array_add(array, size, value) (array)[size++] = value

#define array_resize_add(array, size, value, value_type)        \
    do {                                                        \
        array_increase_size(array, (size), sizeof(value_type)); \
        (array)[(size)++] = value;                              \
    } while (0)

#ifdef UTILS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

void array_remove(void **array, size_t *size, size_t index) {
    for (size_t i = index; i < *size - 1; i++) {
        array[i] = array[i + 1];
    }
    (*size)--;
}

#endif
