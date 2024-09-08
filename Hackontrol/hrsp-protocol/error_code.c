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
