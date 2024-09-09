#include <khopanstring.h>
#include "hrsp_protocol.h"

LPCWSTR HRSPGetErrorCode(const HRSPPROTOCOLERRORCODE code) {
	switch(code) {
	case HRSP_ERROR_SUCCESS:                    return L"An operation succeeded with no error";
	case HRSP_ERROR_INVALID_FUNCTION_PARAMETER: return L"Invalid function parameter";
	case HRSP_ERROR_INVALID_MAGIC:              return L"Invalid HRSP magic number";
	case HRSP_ERROR_UNSUPPORTED_VERSION:        return L"Incompatible client and server version";
	}

	return L"Unknown or invalid error";
}

LPWSTR HRSPGetErrorMessage(const LPCWSTR functionName, const PHRSPPROTOCOLERROR error) {
	LPWSTR message = NULL;
	LPWSTR buffer;

	if(!functionName || !error) {
		buffer = LocalAlloc(LMEM_FIXED, 52);

		if(!buffer) {
			return NULL;
		}

		message = L"Invalid argument for function HRSPGetErrorMessage()";

		for(size_t i = 0; i < 52; i++) {
			((PBYTE) buffer)[i] = ((PBYTE) message)[i];
		}

		return (LPWSTR) buffer;
	}

	if(error->win32) {
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error->code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message, 0, NULL);
	} else {
		message = (LPWSTR) HRSPGetErrorCode(error->code);
	}

	if(message) {
		buffer = KHFormatMessageW(L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws() Message:\n%ws", functionName, error->win32 ? L"Win32" : L"HRSP", error->code, error->function, message);

		if(error->win32) {
			LocalFree(message);
		}
	} else {
		buffer = KHFormatMessageW(L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws()", functionName, error->win32 ? L"Win32" : L"HRSP", error->code, error->function);
	}

	return buffer;
}
