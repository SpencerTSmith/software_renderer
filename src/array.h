#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

// preprocessor function for fast push backs
#define array_push(array, value)                                                                   \
    do {                                                                                           \
        (array) = array_hold((array), 1, sizeof(*(array)));                                        \
        (array)[array_size(array) - 1] = (value);                                                  \
    } while (0)

void *array_hold(void *array, int count, int element_size);
int array_size(void *array);
void array_free(void *array);

// typedef enum {
//     FACE_T,
//     VERT_T,
// } darray_type_e;

typedef struct {
    int capacity;
    int occupied;
    void *data;
} darray_t;

void darray_reserve(darray_t array, int count, size_t element_size);
// not type safe
void darray_pushback(darray_t array, const void *value);
void darray_free(darray_t array);

#endif
