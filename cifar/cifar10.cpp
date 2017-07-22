// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cifar10.h"

void cifar_destroy(cifar_data* data) {
	if (data != NULL) {
		if (data->batches != NULL){
			for (size_t i = 0; i < data->batch_count; i++) {
				cifar_safe_free(data->batches[i].r);
				cifar_safe_free(data->batches[i].g);
				cifar_safe_free(data->batches[i].b);
			}
			cifar_safe_free(data->batches);
		}
		cifar_safe_free(data);
	}
}

cifar_data* cifar_load_batch(LPCWSTR path, size_t batch_count) {
	if (!path) {
		printf("CIFAR Error: Null pointer [path]\n");
		return NULL;
	}

	if (batch_count == 0){
		printf("CIFAR Error: [batch_count] must be greater than 0\n");
		return NULL;
	}

	if (batch_count > CIFAR_BATCH_COUNT) {
		printf("CIFAR Error: [batch_count] must be less than or equal to %d\n", CIFAR_BATCH_COUNT);
		return NULL;
	}

	FILE* file = NULL;
	errno_t result = _wfopen_s(&file, path, L"rb");

	if (result != 0 || file == NULL) {
		wprintf(L"CIFAR Error: Open file %s\n", path);
		return NULL;
	}

	byte buff[CIFAR_BATCH_BYTES]; ZeroMemory(&buff, CIFAR_BATCH_BYTES);

	cifar_data* data = (cifar_data*)malloc(sizeof(cifar_data));
	cifar_batch* batches = (cifar_batch*)malloc(sizeof(cifar_batch) * batch_count);

	try{
		if (!data || !batches){
			throw "Out of memory";
		}

		data->batch_count = batch_count;
		data->batches = batches;

		for (size_t i = 0; i < batch_count; i++) {
			ZeroMemory(&batches[i], sizeof(cifar_batch));
			batches[i].r = (cifar_type*)malloc(sizeof(cifar_type) * CIFAR_SAMPLE_SIZE);
			batches[i].g = (cifar_type*)malloc(sizeof(cifar_type) * CIFAR_SAMPLE_SIZE);
			batches[i].b = (cifar_type*)malloc(sizeof(cifar_type) * CIFAR_SAMPLE_SIZE);
			if (!batches[i].r || !batches[i].g || !batches[i].b) {
				cifar_safe_free(batches[i].r);
				cifar_safe_free(batches[i].g);
				cifar_safe_free(batches[i].b);
				throw "Out of memory";
			}
		}

		for (size_t i = 0; i < batch_count; i++) {
			auto bytes = fread_s(buff, CIFAR_BATCH_BYTES, 1, CIFAR_BATCH_BYTES, file);
			if (bytes != CIFAR_BATCH_BYTES) {
				throw "Read data";
			}

			batches[i].tag = buff[0];
			for (size_t j = 0; j < CIFAR_SAMPLE_SIZE; j++) {
				batches[i].r[j] = (cifar_type)buff[1 + CIFAR_SAMPLE_SIZE * 0 + j] / 255;
				batches[i].g[j] = (cifar_type)buff[1 + CIFAR_SAMPLE_SIZE * 1 + j] / 255;
				batches[i].b[j] = (cifar_type)buff[1 + CIFAR_SAMPLE_SIZE * 2 + j] / 255;
			}
		}

		fclose(file);
		return data;
	}
	catch (const char* msg){
		cifar_destroy(data);
		fclose(file);
		printf("%s\n", ("CIFAR Error: " + std::string(msg)).c_str());
		return NULL;
	}
}

void cifar_print_batch(cifar_data* data, size_t batch_index, size_t begin, size_t count) {
	if (!data) {
		printf("CIFAR Error: null pointer [data]\n");
		return;
	}

	if (batch_index >= CIFAR_BATCH_COUNT) {
		printf("CIFAR Error: [batch_index] must be less than %d\n", CIFAR_BATCH_COUNT);
		return;
	}

	if (begin >= CIFAR_SAMPLE_SIZE) {
		printf("CIFAR Error: [begin] must be less than %d\n", CIFAR_SAMPLE_SIZE);
		return;
	}

	if (count >= CIFAR_SAMPLE_SIZE) {
		printf("CIFAR Error: [count] must be less than %d\n", CIFAR_SAMPLE_SIZE);
		return;
	}

	for (size_t i = begin; i < CIFAR_SAMPLE_SIZE; i++) {
		if (count-- == 0) {
			return;
		}
		printf("#%lu [R: %.4f G: %.4f B: %.4f\n", i,
			data->batches[batch_index].r[i],
			data->batches[batch_index].g[i],
			data->batches[batch_index].b[i]);
	}
}