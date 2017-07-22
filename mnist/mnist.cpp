// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "mnist.h"

void mnist_destroy(mnist_data* data) {
	if (data != NULL) {
		if (data->batches != NULL){
			for (size_t i = 0; i < data->batch_count; i++) {
				mnist_safe_free(data->batches[i].pixels);
			}
			mnist_safe_free(data->batches);
		}
		mnist_safe_free(data);
	}
}

void mnist_print_batch(mnist_data* data, size_t batch_index, size_t begin, size_t count) {
	for (size_t i = begin; i < MNIST_IMAGE_SIZE; i++) {
		if (count-- == 0) {
			return;
		}
		printf("%lu [Tag: %d color: %f]\n", i,
			(int)data->batches[batch_index].tag,
			data->batches[batch_index].pixels[i]);
	}
}

mnist_data* mnist_load_data(LPCWSTR batches_path, LPCWSTR labels_path, size_t batch_count) {
	if (!batches_path) {
		printf("MNIST Error: Null pointer [batches_path]\n");
		return NULL;
	}

	if (!labels_path) {
		printf("MNIST Error: Null pointer [labels_path]\n");
		return NULL;
	}

	if (batch_count == 0){
		printf("MNIST Error: [batch_count] must be greater than 0\n");
		return NULL;
	}

	if (batch_count > MNIST_IMAGES_COUNT) {
		printf("MNIST Error: [batch_count] must be less than or equal to %d\n", MNIST_IMAGES_COUNT);
		return NULL;
	}

	errno_t result;
	FILE* file_batches = NULL;
	FILE* file_labels = NULL;

	mnist_header data_header; ZeroMemory(&data_header, sizeof(data_header));
	mnist_label_header labels_header; ZeroMemory(&labels_header, sizeof(labels_header));
	byte pixels_buff[MNIST_IMAGE_SIZE]; ZeroMemory(&pixels_buff, sizeof(pixels_buff));
	byte* labels_buff = NULL;

	result = _wfopen_s(&file_batches, batches_path, L"rb");
	if (result != 0 || file_batches == NULL) {
		wprintf(L"MNIST Error: Open file %s\n", batches_path);
		return NULL;
	}

	result = _wfopen_s(&file_labels, labels_path, L"rb");
	if (result != 0 || file_labels == NULL) {
		fclose(file_batches);
		wprintf(L"MNIST Error: Open file %s\n", labels_path);
		return NULL;
	}

	try{
		size_t data_header_bytes = fread_s(&data_header, sizeof(data_header), 1, sizeof(data_header), file_batches);
		if (data_header_bytes != sizeof(data_header)) {
			throw "Read file";
		}

		size_t labels_header_bytes = fread_s(&labels_header, sizeof(labels_header), 1, sizeof(labels_header), file_labels);
		if (labels_header_bytes != sizeof(labels_header)) {
			throw "Read file";
		}

		labels_header.magic_number = endian_swap(labels_header.magic_number);
		labels_header.items_count = endian_swap(labels_header.items_count);

		labels_buff = (byte*)malloc(sizeof(byte)* batch_count);
		if (!labels_buff){
			throw "Out of memory";
		}
		ZeroMemory(labels_buff, sizeof(byte) * batch_count);

		size_t labels_bytes = fread_s(labels_buff, batch_count, 1, batch_count, file_labels);
		if (labels_bytes != batch_count) {
			throw "Read file";
		}

		data_header.magic_number = endian_swap(data_header.magic_number);
		data_header.images_count = endian_swap(data_header.images_count);
		data_header.rows_count = endian_swap(data_header.rows_count);
		data_header.colums_count = endian_swap(data_header.colums_count);

		mnist_data* data = NULL;
		mnist_batch* batches = NULL;

		try{
			data = (mnist_data*)malloc(sizeof(mnist_data));
			batches = (mnist_batch*)malloc(sizeof(mnist_batch) * batch_count);
			if (!data || !batches) {
				throw "Out of memory";
			}

			data->batch_count = batch_count;
			data->batches = batches;

			for (size_t i = 0; i < batch_count; i++)
			{
				ZeroMemory(&batches[i], sizeof(mnist_batch));
			}

			for (size_t i = 0; i < batch_count; i++) {
				batches[i].pixels = (mnist_type*)malloc(sizeof(mnist_type) * MNIST_IMAGE_SIZE);
				if (!batches[i].pixels) {
					throw "Out of memory";
				}
				batches[i].tag = labels_buff[i];
			}

			mnist_safe_free(labels_buff);

			for (size_t i = 0; i < batch_count; i++) {
				size_t pixelBytesReaded = fread_s(pixels_buff, MNIST_IMAGE_SIZE, 1, MNIST_IMAGE_SIZE, file_batches);
				if (pixelBytesReaded != MNIST_IMAGE_SIZE) {
					throw "Read file";
				}

				for (size_t j = 0; j < MNIST_IMAGE_SIZE; j++) {
					data->batches[i].pixels[j] = (mnist_type)pixels_buff[j] / 255;
				}
			}
		}
		catch (const char* msg){
			mnist_destroy(data);
			throw msg;
		}

		fclose(file_batches);
		fclose(file_labels);

		return data;
	}
	catch (const char* msg){
		mnist_safe_free(labels_buff);
		fclose(file_batches);
		fclose(file_labels);
		printf("%s\n", ("MNIST Error: " + std::string(msg)).c_str());
		return NULL;
	}
}