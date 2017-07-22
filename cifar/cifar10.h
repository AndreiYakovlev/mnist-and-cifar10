#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>

#if COMPILING_DLL
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif

#define CIFAR_SAMPLE_SIZE (1024)
#define CIFAR_BATCH_SIZE (3072)	//SAMPLE_SIZE * 3
#define CIFAR_BATCH_BYTES (3073)	// BATCH_SIZE + 1
#define CIFAR_BATCH_COUNT (10000)

#define cifar_safe_free(x) {if(x) { free(x); x = NULL; }}

typedef float cifar_type;

typedef struct _cifar_batch
{
	byte tag;
	cifar_type* r;
	cifar_type* g;
	cifar_type* b;
} cifar_batch;

typedef struct _cifar_data
{
	size_t batch_count;
	cifar_batch* batches;
} cifar_data;

LIB_API void cifar_destroy(cifar_data* data);

LIB_API cifar_data* cifar_load_batch(LPCWSTR path, size_t batch_count);

LIB_API void cifar_print_batch(cifar_data* data, size_t batch_index, size_t begin, size_t count);
