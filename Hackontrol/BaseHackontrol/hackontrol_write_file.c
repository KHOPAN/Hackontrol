#include "hackontrol.h"

BOOL HackontrolWriteFile(LPCWSTR const filePath, const DataStream* const stream) {
	if(!filePath || !stream) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	HANDLE file = CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	DWORD bytesWritten;
	BOOL returnValue = FALSE;

	if(!WriteFile(file, stream->data, (DWORD) stream->size, &bytesWritten, NULL)) {
		goto closeFile;
	}

	if(stream->size != bytesWritten) {
		SetLastError(ERROR_FUNCTION_FAILED);
		goto closeFile;
	}

	returnValue = TRUE;
closeFile:
	CloseHandle(file);
	return returnValue;
}
