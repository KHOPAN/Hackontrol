#include "hrsp.h"

LPCWSTR HRSPErrorHRSPDecoder(const PKHOPANERROR error) {
	if(!error) {
		return NULL;
	}

	if(error->facility != ERROR_FACILITY_HRSP) {
		return KHOPANErrorCommonDecoder(error);
	}

	switch(error->code) {
	case ERROR_HRSP_INVALID_MAGIC:       return L"Invalid HRSP magic number";
	case ERROR_HRSP_UNSUPPORTED_VERSION: return L"Incompatible client and server version";
	case ERROR_HRSP_CONNECTION_CLOSED:   return L"The connection was already closed";
	default:                             return L"Unknown error code";
	}
}
