#include <libkhopan.h>
#include "hrsp.h"

LPWSTR HRSPGetErrorMessage(const LPCWSTR function, const PHRSPERROR error) {
	if(!function || !error) {
		return NULL;
	}

	if(!error->win32) {
		return KHOPANFormatMessage(L"%ws() error occurred. Error type: [HRSP] Error code: %lu Caused by %ws() Message:\n%ws", function, error->code, error->function, HRSPGetErrorCode(error->code));
	}

	LPWSTR message = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error->code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message, 0, NULL);

	if(!message) {
		return KHOPANFormatMessage(L"%ws() error occurred. Error type: [Win32] Error code: %lu Caused by %ws()", function, error->code, error->function);
	}

	LPWSTR buffer = KHOPANFormatMessage(L"%ws() error occurred. Error type: [Win32] Error code: %lu Caused by %ws() Message:\n%ws", function, error->code, error->function, message);
	LocalFree(message);
	return buffer;
}

LPCWSTR HRSPGetErrorCode(const HRSPERRORCODE code) {
	switch(code) {
	case HRSP_ERROR_SUCCESS:                    return L"An operation succeeded with no error";
	case HRSP_ERROR_INVALID_FUNCTION_PARAMETER: return L"Invalid function parameter";
	case HRSP_ERROR_INVALID_MAGIC:              return L"Invalid HRSP magic number";
	case HRSP_ERROR_UNSUPPORTED_VERSION:        return L"Incompatible client and server version";
	case HRSP_ERROR_CONNECTION_CLOSED:          return L"The connection was already closed";
	}

	return L"Unknown error code";
}
