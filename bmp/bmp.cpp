// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "bmp.h"

inline bool swap_bytes_(byte* dst, const byte* src, size_t count) {
	if (count == 0){
		return false;
	}

	/*¬ыдел€ем пам€ть под временное хранение данныы источника, так как приемник(dst) и источник(src)
	могут указывать на одну и ту же область в пам€ти*/
	byte* tmp = (byte*)malloc(sizeof(byte) * count);
	if (tmp == NULL){
		return false;
	}
	memcpy(tmp, src, count);

	/*for (size_t i = 0; i < count; i++)
	{
	dst[i] = tmp[count - i - 1];
	}*/

	/*Fast code*/
	tmp += count;
	for (size_t i = 0; i < count; i++) {
		*dst++ = *(--tmp);
	}

	free(tmp);
	return true;
}

byte* bmp_load_file(_In_ LPCWSTR path,
	_Out_opt_ BITMAPFILEHEADER* pfileHeader,
	_Out_opt_ BITMAPINFOHEADER* pinfoHeader) {
	BITMAPFILEHEADER fileHeader; ZeroMemory(&fileHeader, sizeof(fileHeader));
	BITMAPINFOHEADER infoHeader; ZeroMemory(&infoHeader, sizeof(infoHeader));

	if (pfileHeader) {
		*pfileHeader = fileHeader;
	}
	if (pinfoHeader) {
		*pinfoHeader = infoHeader;
	}

	std::wstring pathStr(path);
	auto extension = pathStr.substr(pathStr.find_first_of('.') + 1);
	//auto extension = getExtension(path);
	if (extension != L"bmp") {
		wprintf_s(L"BMP Error: File extension [%s] not supported \n", extension.c_str());
		return NULL;
	}

	FILE* file = NULL;
	errno_t result = _wfopen_s(&file, path, L"rb+");

	if (file == NULL) {
		wprintf_s(L"BMP Error: Open file [%s]\n", path);
		return NULL;
	}

	size_t bytes_readed = fread_s(&fileHeader, sizeof(fileHeader), 1, sizeof(fileHeader), file);
	if (bytes_readed != sizeof(fileHeader)) {
		wprintf_s(L"BMP Error: Read header of file [%s]\n", path);
		fclose(file);
		return NULL;
	}

	bytes_readed = fread_s(&infoHeader, sizeof(infoHeader), 1, sizeof(infoHeader), file);
	if (bytes_readed != sizeof(infoHeader)) {
		wprintf_s(L"BMP Error: Read info header of file [%s]\n", path);
		fclose(file);
		return NULL;
	}

	if (infoHeader.biSizeImage == 0) {
		infoHeader.biSizeImage = fileHeader.bfSize - fileHeader.bfOffBits;
	}

	if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) {
		wprintf_s(L"BMP Error: File [%s] not supported. Only 24 and 32 bits\n", path);
		fclose(file);
		return NULL;
	}

	size_t bytes = infoHeader.biBitCount / 8;
	size_t data_stride = infoHeader.biWidth * bytes;
	size_t file_stride = infoHeader.biSizeImage / infoHeader.biHeight;
	size_t data_size = data_stride * infoHeader.biHeight;

	byte* file_data = (byte*)malloc(infoHeader.biSizeImage);
	if (!file_data) {
		wprintf_s(L"BMP Error: Allocate memory for reading data\n");
		fclose(file);
		return NULL;
	}

	byte* data = (byte*)malloc(data_size);
	if (!data) {
		wprintf_s(L"BMP Error: Allocate memory for reading data\n");
		fclose(file);
		free(file_data);
		return NULL;
	}

	bytes_readed = fread_s(file_data, infoHeader.biSizeImage, 1, infoHeader.biSizeImage, file);
	if (bytes_readed != infoHeader.biSizeImage) {
		wprintf_s(L"BMP Error: Reading data of file [%s]\n", path);
		fclose(file);
		free(file_data);
		free(data);
		return NULL;
	}

	fclose(file);

	auto data_it = data;
	auto file_data_it = file_data + file_stride * infoHeader.biHeight;

	for (LONG i = 0; i < infoHeader.biHeight; i++) {
		file_data_it -= file_stride;
		memcpy(data_it, file_data_it, data_stride);

		for (LONG j = 0; j < infoHeader.biWidth; j++) {
			swap_bytes_(data_it, data_it, bytes);
			data_it += bytes;
			/*printf("[%d %d %d] ",
				   data[data_stride * i + 3 * j],
				   data[data_stride * i + 3 * j + 1],
				   data[data_stride * i + 3 * j + 2]);*/
		}
		//printf("\n");
	}

	free(file_data);

	if (pfileHeader) {
		*pfileHeader = fileHeader;
	}
	if (pinfoHeader) {
		*pinfoHeader = infoHeader;
	}

	return data;
}

void bmp_free(byte* data) {
	if (data) {
		free(data);
		data = NULL;
	}
}

void bmp_save_to_file(_In_ LPCWSTR path, _In_ byte* data, size_t size, LONG width, LONG height, BMP_FORMAT format) {
	BITMAPFILEHEADER fileHeader; ZeroMemory(&fileHeader, sizeof(fileHeader));
	BITMAPINFOHEADER infoHeader; ZeroMemory(&infoHeader, sizeof(infoHeader));

	FILE* file = NULL;
	errno_t result = _wfopen_s(&file, path, L"wb+");

	if (file == NULL) {
		wprintf_s(L"BMP Error: Create file %s\n", path);
		return;
	}

	size_t stride = width;
	size_t pixel_bytes = 0;
	switch (format) {
	case BMP_FORMAT::BMP_FORMAT_24:
		infoHeader.biBitCount = 24;
		stride *= pixel_bytes = 3;
		break;
	case BMP_FORMAT::BMP_FORMAT_32:
		infoHeader.biBitCount = 32;
		stride *= pixel_bytes = 4;
		break;
	}

	size_t width_bytes = stride;
	stride += stride % 4;

	infoHeader.biSize = sizeof(infoHeader);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biSizeImage = stride * height;

	fileHeader.bfType = 0x4D42; //magic number BM
	fileHeader.bfSize = sizeof(fileHeader) + sizeof(infoHeader) + infoHeader.biSizeImage;
	fileHeader.bfOffBits = 0x36; //54

	fwrite(&fileHeader, 1, sizeof(fileHeader), file);
	fwrite(&infoHeader, 1, sizeof(infoHeader), file);

	byte* stride_data = new byte[stride];

	//for (LONG i = height - 1; i >= 0; i--) {
	//	size_t offset = 0;
	//	size_t offset_bytes = width_bytes * i;
	//	for (size_t j = 0; j < width; j++) {
	//		auto ss = &stride_data[offset];
	//		auto dd = &data[offset_bytes + 3 * j];
	//		//swap_bytes(ss, dd, 3);
	//		memcpy(&stride_data[offset], &data[offset_bytes + 3 * j], stride);
	//		offset += 3;
	//	}

	//	//ZeroMemory(&stride_data[offset], sizeof(*stride_data) * (width % 4));

	//	fwrite(stride_data, 1, stride, file);
	//}

	for (LONG i = height - 1; i >= 0; i--) {
		size_t offset = 0;
		size_t offset_bytes = width_bytes * i;
		memcpy(&stride_data[0], &data[offset_bytes], stride);
		//ZeroMemory(&stride_data[offset_bytes], sizeof(*stride_data) * (width % 4));

		fwrite(stride_data, 1, stride, file);
	}

	delete[] stride_data;

	fclose(file);
}