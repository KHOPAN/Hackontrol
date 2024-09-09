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
		PBYTE buffer = LocalAlloc(LMEM_FIXED, 58);

		if(!buffer) {
			return NULL;
		}

		PBYTE text = (PBYTE) L"Invalid argument for function HRSPClientGetErrorMessage()";

		for(size_t i = 0; i < 58; i++) {
			buffer[i] = text[i];
		}

		return (LPWSTR) buffer;
	}

	return NULL;
}
