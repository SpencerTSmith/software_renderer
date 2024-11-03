#include "array.h"

#include <stdlib.h>

// Gonna store the capacity of the array and it's currently "occupied" size in
// the 4*2 bytes before the array, called a "header"
#define ARRAY_RAW_DATA(array) ((int *)(array) - 2)       // Grab 2 int sized spots
#define ARRAY_CAPACITY(array) (ARRAY_RAW_DATA(array)[0]) // store max capacity
#define ARRAY_OCCUPIED(array) (ARRAY_RAW_DATA(array)[1]) // store used count

// returns a dynamically sized array... must remember to free, see "array_free"
void *array_hold(void *array, int count, int element_size) {
    int header_size = sizeof(int) * 2;

    if (array == NULL) {
        int adjusted_size = header_size + (count * element_size);
        int *result = (int *)malloc(adjusted_size);

        result[0] = count; // capacity
        result[1] = count; // actual count
        return result + 2;
    } else if (ARRAY_OCCUPIED(array) + count <= ARRAY_CAPACITY(array)) {
        ARRAY_OCCUPIED(array) += count;
        return array;
    } else {
        int needed_size = ARRAY_OCCUPIED(array) + count;
        int double_capacity = ARRAY_CAPACITY(array) * 2;
        int new_capacity = needed_size > double_capacity
                               ? needed_size
                               : double_capacity; // double the capacity, if not enough,
                                                  // allocate exactly the needed size

        int adjusted_size = header_size + (new_capacity * element_size);

        int *result = (int *)realloc(ARRAY_RAW_DATA(array), adjusted_size);
        result[0] = new_capacity; // capacity
        result[1] = needed_size;  // new actual count

        return result + 2;
    }
}

// Retrieve # elements
int array_size(void *array) {
    int size = (array != NULL) ? ARRAY_OCCUPIED(array) : 0;
    return size;
}

// Free the array and its "header"
void array_free(void *array) {
    if (array != NULL)
        free(ARRAY_RAW_DATA(array));
}

void darray_reserve(darray_t array, int size, size_t element_size) {
    if (array.occupied == 0 || array.data == NULL) {
        array.capacity = size;
        array.data = malloc(size * element_size);

    } else if (array.occupied + size <= array.occupied) {
        array.occupied += size;
    } else {
        int needed_size = array.occupied + size;
        int double_size = array.capacity * 2;

        int new_capacity = needed_size > double_size ? needed_size : double_size;

        array.data = realloc(array.data, new_capacity);
        array.capacity = new_capacity;
    }
}

void darray_pushback(darray_t array, const void *value) {}
