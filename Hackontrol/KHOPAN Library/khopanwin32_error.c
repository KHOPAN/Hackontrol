#include "khopanwin32.h"
#include "khopanstring.h"

static LPWSTR getError(const DWORD code, const LPCWSTR function, const BOOL win32) {
	LPWSTR buffer;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | (win32 ? 0 : FORMAT_MESSAGE_FROM_HMODULE), win32 ? NULL : LoadLibraryW(L"ntdll.dll"), code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, NULL)) {
		return function ? KHFormatMessageW(L"%ws() error occurred. Error code: %lu", function, code) : KHFormatMessageW(L"Error occurred. Error code: %lu", code);
	}

	LPWSTR message = function ? KHFormatMessageW(L"%ws() error occurred. Error code: %lu Message:\n%ws", function, code, buffer) : KHFormatMessageW(L"Error occurred. Error code: %lu Message:\n%ws", code, buffer);
	LocalFree(buffer);
	return message;
}

LPWSTR KHOPANGetErrorMessageWin32(const DWORD code, const LPCWSTR function) {
	return getError(code, function, TRUE);
}

LPWSTR KHOPANGetErrorMessageHRESULT(const HRESULT code, const LPCWSTR function) {
	return getError(code == S_OK ? ERROR_SUCCESS : (HRESULT) (code & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0) ? HRESULT_CODE(code) : ERROR_CAN_NOT_COMPLETE, function, TRUE);
}

LPWSTR KHOPANGetErrorMessageNTSTATUS(const NTSTATUS code, const LPCWSTR function) {
	return getError(code, function, FALSE);
}
