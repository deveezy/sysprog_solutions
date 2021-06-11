#include "array.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

void 
array_init(array_int *arr, size_t size)
{
	arr->buf = malloc(size * sizeof(int));
	assert(arr->buf != NULL);
	if (arr->buf == NULL) exit(1);
	arr->capacity = size;
	arr->size = 0;
}

void
array_resize(array_int *arr)
{
	int new_cap = arr->capacity * 2;
	int* new_buf = malloc(new_cap * sizeof(int));
	memcpy(new_buf, arr->buf, arr->capacity * sizeof(int));
	free(arr->buf);
	arr->buf = new_buf;
	assert(arr->buf != NULL);
	if (arr->buf == NULL) exit(1);
	arr->capacity = new_cap;
}

void 
array_add(array_int *arr, int number)
{
	if (arr->size + 1 > arr->capacity) {
		array_resize(arr);
	}
	arr->buf[arr->size] = number;
	++arr->size;
}

void 
array_free(array_int *arr)
{
	free(arr->buf);
	arr->buf = NULL;
	arr->size = 0;
	arr->capacity = 0;
}

void array_sort(array_int *arr, void *(fun)(int *, 	size_t, size_t)) 
{
	fun(arr->buf, 0, arr->size - 1);
}

// void array_sort(array_int *arr, void *(fun)(), ...) 
// {
// 	fun(__VA_ARGS__);
// }