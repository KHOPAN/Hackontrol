#include <khopanstring.h>
#include <hrsp_protocol.h>
#include "hrsp_client.h"

LPCWSTR HRSPClientGetErrorCode(const HRSPCLIENTERRORCODE code) {
	switch(code) {
	case HRSP_CLIENT_ERROR_SUCCESS:               return L"An operation succeeded with no error";
	case HRSP_CLIENT_ERROR_CANNOT_CONNECT_SERVER: return L"Cannot connect to the server";
	}

	return L"Unknown or invalid error";
}

LPWSTR HRSPClientGetErrorMessage(const LPCWSTR functionName, const PHRSPCLIENTERROR error) {
	LPWSTR message = NULL;
	LPWSTR buffer;

	if(!functionName || !error) {
		buffer = LocalAlloc(LMEM_FIXED, 58);

		if(!buffer) {
			return NULL;
		}

		message = L"Invalid argument for function HRSPClientGetErrorMessage()";

		for(size_t i = 0; i < 58; i++) {
			((PBYTE) buffer)[i] = ((PBYTE) message)[i];
		}

		return (LPWSTR) buffer;
	}

	switch(error->type) {
	case HRSP_CLIENT_ERROR_TYPE_CLIENT:
		message = (LPWSTR) HRSPClientGetErrorCode(error->code);
		break;
	case HRSP_CLIENT_ERROR_TYPE_HRSP:
		message = (LPWSTR) HRSPGetErrorCode(error->code);
		break;
	case HRSP_CLIENT_ERROR_TYPE_WIN32:
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error->code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message, 0, NULL);
		break;
	}

	if(message) {
		buffer = KHFormatMessageW(L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws() Message:\n%ws", functionName, error->type == HRSP_CLIENT_ERROR_TYPE_CLIENT ? L"HRSP Client" : error->type == HRSP_CLIENT_ERROR_TYPE_HRSP ? L"HRSP" : error->type == HRSP_CLIENT_ERROR_TYPE_WIN32 ? L"Win32" : L"Unknown", error->code, error->function, message);

		if(error->type == HRSP_CLIENT_ERROR_TYPE_WIN32) {
			LocalFree(message);
		}
	} else {
		buffer = KHFormatMessageW(L"%ws() error occurred. Error type: %ws Error code: %lu Caused by %ws()", functionName, error->type == HRSP_CLIENT_ERROR_TYPE_CLIENT ? L"HRSP Client" : error->type == HRSP_CLIENT_ERROR_TYPE_HRSP ? L"HRSP" : error->type == HRSP_CLIENT_ERROR_TYPE_WIN32 ? L"Win32" : L"Unknown", error->code, error->function);
	}

	return buffer;
}
