#pragma once

typedef unsigned long size_t;

typedef struct {
	int *buf;
	size_t capacity;
	size_t size;
} array_int;

void array_init(array_int *arr, size_t size);
void array_resize(array_int *arr);
void array_add(array_int *arr, int number);
void array_free(array_int *arr);
void array_sort(array_int *arr, void *(fun)(int *, 	size_t, size_t));