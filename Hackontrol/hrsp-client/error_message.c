#include <khopanstring.h>
#include <hrsp.h>
#include "hrsp_client.h"

LPWSTR HRSPClientGetErrorMessage(const LPCWSTR functionName, const PHRSPCLIENTERROR error) {
	if(!functionName || !error) {
		return NULL;
	}

	if(error->type == HRSP_CLIENT_ERROR_TYPE_HRSP || error->type == HRSP_CLIENT_ERROR_TYPE_WIN32) {
		return HRSPGetErrorMessage(functionName, error);
	}

	if(error->type != HRSP_CLIENT_ERROR_TYPE_CLIENT) {
		return KHFormatMessageW(L"%ws() error occurred. Error code: %lu Caused by %ws()", functionName, error->code, error->function);
	}

	return KHFormatMessageW(L"%ws() error occurred. Error type: [HRSP Client] Error code: %lu Caused by %ws() Message:\n%ws", functionName, error->code, error->function, HRSPClientGetErrorCode(error->code));
}

LPCWSTR HRSPClientGetErrorCode(const HRSPCLIENTERRORCODE code) {
	switch(code) {
	case HRSP_CLIENT_ERROR_SUCCESS:               return L"An operation succeeded with no error";
	case HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER: return L"Cannot connect to the server";
	}

	return L"Unknown or invalid error";
}
