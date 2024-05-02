#include <stdio.h>
#include "khopanstring.h"

static LPSTR formatInternalA(const LPSTR format, va_list arguments) {
	int length = _vscprintf(format, arguments);

	if(length == -1) {
		return NULL;
	}

	size_t size = length + 1;
	LPSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(CHAR));

	if(!buffer) {
		return NULL;
	}

	if(vsprintf_s(buffer, size, format, arguments) == -1) {
		LocalFree(buffer);
		return NULL;
	}

	return buffer;
}

static LPWSTR formatInternalW(const LPWSTR format, va_list arguments) {
	int length = _vscwprintf(format, arguments);

	if(length == -1) {
		return NULL;
	}

	size_t size = length + 1;
	LPWSTR buffer = LocalAlloc(LMEM_FIXED, size * sizeof(WCHAR));

	if(!buffer) {
		return NULL;
	}

	if(vswprintf_s(buffer, size, format, arguments) == -1) {
		LocalFree(buffer);
		return NULL;
	}

	return buffer;
}

LPSTR KHFormatMessageA(const LPSTR format, ...) {
	va_list list;
	va_start(list, format);
	LPSTR result = formatInternalA(format, list);
	va_end(list);
	return result;
}

LPWSTR KHFormatMessageW(const LPWSTR format, ...) {
	va_list list;
	va_start(list, format);
	LPWSTR result = formatInternalW(format, list);
	va_end(list);
	return result;
}
