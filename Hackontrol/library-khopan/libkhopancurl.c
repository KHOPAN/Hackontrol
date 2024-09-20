#include "libkhopan.h"
#include "libkhopancurl.h"

LPWSTR KHOPANCURLInternalGetErrorMessage(const CURLcode code, const LPCWSTR function) {
	LPCSTR message = curl_easy_strerror(code);

	if(!message) {
		return function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu", function, code) : KHOPANFormatMessage(L"Error occurred. Error code: %lu", code);
	}

	return function ? KHOPANFormatMessage(L"%ws() error occurred. Error code: %lu Message:\n%S", function, code, message) : KHOPANFormatMessage(L"Error occurred. Error code: %lu Message:\n%S", code, message);
}
