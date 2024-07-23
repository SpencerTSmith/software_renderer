#ifndef ARRAY_H
#define ARRAY_H

// preprocessor function for fast push backs
#define array_push(array, value)                                               \
	do {                                                                       \
		(array) = array_hold((array), 1, sizeof(*(array)));                    \
		(array)[array_size(array) - 1] = (value);                              \
	} while (0)

void *array_hold(void *array, int count, int size);
int array_size(void *array);
void array_free(void *array);

#endif
