#include <khopanstring.h>
#include "hrsp.h"

LPCWSTR HRSPGetErrorCode(const HRSPERRORCODE code) {
	switch(code) {
	case HRSP_ERROR_SUCCESS:                    return L"An operation succeeded with no error";
	case HRSP_ERROR_INVALID_FUNCTION_PARAMETER: return L"Invalid function parameter";
	case HRSP_ERROR_INVALID_MAGIC:              return L"Invalid HRSP magic number";
	case HRSP_ERROR_UNSUPPORTED_VERSION:        return L"Incompatible client and server version";
	}

	return L"Unknown or invalid error";
}

LPWSTR HRSPGetErrorMessage(const LPCWSTR functionName, const PHRSPERROR error) {
	if(!functionName || !error) {
		return NULL;
	}

	LPWSTR message = NULL;

	if(error->win32) {
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error->code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message, 0, NULL);
	} else {
		message = (LPWSTR) HRSPGetErrorCode(error->code);
	}

	LPWSTR buffer = KHFormatMessageW(message ? L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws() Message:\n%ws%ws" : L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws()%ws%ws", functionName, error->win32 ? L"Win32" : L"HRSP", error->code, error->function, message ? message : (LPWSTR) &message, message ? error->win32 ? L"" : L"\n" : (LPWSTR) &message);

	if(message && error->win32) {
		LocalFree(message);
	}

	return buffer;
}
