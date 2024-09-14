#include <khopanstring.h>
#include <hrsp.h>
#include "hrsp_client.h"

LPCWSTR HRSPClientGetErrorCode(const HRSPCLIENTERRORCODE code) {
	switch(code) {
	case HRSP_CLIENT_ERROR_SUCCESS:               return L"An operation succeeded with no error";
	case HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER: return L"Cannot connect to the server";
	}

	return L"Unknown or invalid error";
}

LPWSTR HRSPClientGetErrorMessage(const LPCWSTR functionName, const PHRSPCLIENTERROR error) {
	if(!functionName || !error) {
		return NULL;
	}

	if(error->type == HRSP_CLIENT_ERROR_TYPE_HRSP || error->type == HRSP_CLIENT_ERROR_TYPE_WIN32) {
		HRSPERROR protocolError;
		protocolError.win32 = error->type == HRSP_CLIENT_ERROR_TYPE_WIN32;
		protocolError.function = error->function;
		protocolError.code = error->code;
		return HRSPGetErrorMessage(functionName, &protocolError);
	}

	LPWSTR message = error->type == HRSP_CLIENT_ERROR_TYPE_CLIENT ? (LPWSTR) HRSPClientGetErrorCode(error->code) : NULL;
	return KHFormatMessageW(message ? L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws() Message:\n%ws" : L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws()%ws", functionName, error->type == HRSP_CLIENT_ERROR_TYPE_CLIENT ? L"HRSP Client" : L"Unknown", error->code, error->function, message ? message : (LPWSTR) &message);
}
