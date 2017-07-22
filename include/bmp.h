#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>

#if COMPILING_DLL
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif

inline byte color_float_to_byte(float color) {
	return (byte)(min(max(color, 0), 1) * 255);
}

inline float color_byte_to_float(byte color) {
	return (float)color / 255.0f;
}

inline float color_rgba_to_float(byte r, byte g, byte b, byte a = 0) {
	DWORD gray = ((DWORD)r + (DWORD)g + (DWORD)b) / 3;
	return (gray & 0xFF) / 255.0f;
}

enum class BMP_FORMAT
{
	BMP_FORMAT_24,
	BMP_FORMAT_32,
};

LIB_API byte* bmp_load_file(_In_ LPCWSTR path,
	_Out_opt_ BITMAPFILEHEADER* fileHeader,
	_Out_opt_ BITMAPINFOHEADER* infoHeader);

LIB_API void bmp_free(byte* data);

LIB_API void bmp_save_to_file(_In_ LPCWSTR path, _In_ byte* data, size_t size, LONG width, LONG height, BMP_FORMAT format);
