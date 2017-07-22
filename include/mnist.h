#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>

#if COMPILING_DLL
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif

#define MNIST_IMAGE_WIDTH		(28)
#define MNIST_IMAGE_HEIGHT		(28)
#define MNIST_IMAGE_SIZE		(784)
#define MNIST_IMAGES_COUNT		(60000)
#define MNIST_TEST_IMAGES_COUNT	(10000)
#define mnist_safe_free(x) {if(x) { free(x); x = NULL; }}

typedef float mnist_type;

struct mnist_header
{
	DWORD magic_number;
	DWORD images_count;
	DWORD rows_count;
	DWORD colums_count;
};

struct mnist_label_header
{
	DWORD magic_number;
	DWORD items_count;
};

struct mnist_batch
{
	byte tag;
	mnist_type* pixels;
};

struct mnist_data
{
	size_t batch_count;
	mnist_batch* batches;
};

inline DWORD endian_swap(DWORD x) {
	return
		(x >> 24) |
		((x << 8) & 0x00FF0000) |
		((x >> 8) & 0x0000FF00) |
		(x << 24);
}

LIB_API void mnist_destroy(mnist_data* data);

LIB_API void mnist_print_batch(mnist_data* data, size_t batch_index, size_t begin, size_t count);

LIB_API mnist_data* mnist_load_data(LPCWSTR batches_path, LPCWSTR labels_path, size_t batch_count);
