#include <stdio.h>
#include "khopanwin32.h"
#include "khopanstring.h"

#define ERROR_FORMAT          L"%ws() error in '%ws' Line: %u Error code: %u Message:\n%ws"
#define ERROR_FORMAT_FALLBACK L"%ws() error in '%ws' Line: %u Error code: %u"

#define DEFINE_GET_DIRECTORY(x,y) LP##x##STR KHWin32GetWindowsDirectory##y(){UINT size=GetSystemWindowsDirectory##y(NULL,0);if(!size)return NULL;LP##x##STR buffer=LocalAlloc(LMEM_FIXED,size*sizeof(x##CHAR));if(!buffer)return NULL;if(!GetSystemWindowsDirectory##y(buffer,size)){LocalFree(buffer);return NULL;}return buffer;}

DEFINE_GET_DIRECTORY(,A)
DEFINE_GET_DIRECTORY(W,W)

LPWSTR KHInternal_ErrorMessage(const DWORD errorCode, const LPCWSTR functionName, const LPCWSTR fileName, const UINT lineNumber, const BOOL specialError) {
	LPWSTR buffer;
	size_t lengthFileName = 0;

	if(!fileName) {
		goto formatMessage;
	}

	lengthFileName = wcslen(fileName);

	if(!lengthFileName) {
		goto formatMessage;
	}

	for(size_t i = 0; i < lengthFileName; i++) {
		buffer = (LPWSTR) (lengthFileName - i - 1);

		if(fileName[(size_t) buffer] == L'/' || fileName[(size_t) buffer] == L'\\') {
			lengthFileName = ((size_t) buffer) + 1;
			break;
		}
	}
formatMessage:
	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | (specialError ? FORMAT_MESSAGE_FROM_HMODULE : 0), specialError ? LoadLibraryW(L"ntdll.dll") : NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, NULL)) {
		return KHFormatMessageW(ERROR_FORMAT_FALLBACK, functionName, fileName + lengthFileName, lineNumber, errorCode);
	}

	LPWSTR message = KHFormatMessageW(ERROR_FORMAT, functionName, fileName + lengthFileName, lineNumber, errorCode, buffer);

	if(!message) {
		return buffer;
	}

	LocalFree(buffer);
	return message;
}
