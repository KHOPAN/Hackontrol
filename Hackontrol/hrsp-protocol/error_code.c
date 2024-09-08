#include "hrsp_protocol.h"

LPCWSTR HRSPGetErrorCode(const HRSPPROTOCOLERRORCODE code) {
	switch(code) {
	case HRSP_ERROR_SUCCESS:                    return L"An operation succeeded with no error";
	case HRSP_ERROR_INVALID_FUNCTION_PARAMETER: return L"Invalid function parameter";
	}

	return L"Unknown or invalid error";
}
