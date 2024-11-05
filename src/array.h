#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#include "color.h"
#include "triangle.h"

// preprocessor function for fast push backs
#define array_push(array, value)                                                                   \
    do {                                                                                           \
        (array) = array_hold((array), 1, sizeof(*(array)));                                        \
        (array)[array_size(array) - 1] = (value);                                                  \
    } while (0)

void *array_hold(void *array, int count, int element_size);
void array_reset(void *array);
int array_size(void *array);
void array_free(void *array);

#define dynarray(type)                                                                             \
    typedef struct {                                                                               \
        size_t capacity, occupied;                                                                 \
        type *data;                                                                                \
    } type##header;

#define dynarray_append(array, value)                                                              \
    do {                                                                                           \
        (array) = dynarray_reserve((array), 1, sizeof(*(array)));                                  \
        (array)[array_size(array) - 1] = (value);                                                  \
    } while (0)

void *dynarray_init(void *array, size_t count, size_t type_size);

#endif
