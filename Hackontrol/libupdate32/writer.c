#include "update.h"
#include <khopanstring.h>

BOOL WriteLibdll32(DataStream* stream) {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();
	BOOL returnValue = FALSE;

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		goto freeStream;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_LIBDLL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileLibdll32) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		goto freeStream;
	}

	HANDLE file = CreateFileW(pathFileLibdll32, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	FREE(pathFileLibdll32);

	if(file == INVALID_HANDLE_VALUE) {
		KHWin32DialogErrorW(GetLastError(), L"CreateFileW");
		goto freeStream;
	}

	DWORD bytesWritten;

	if(!WriteFile(file, stream->data, (DWORD) stream->size, &bytesWritten, NULL)) {
		KHWin32DialogErrorW(GetLastError(), L"WriteFile");
		goto closeFile;
	}

	if(stream->size != bytesWritten) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"WriteFile");
		goto closeFile;
	}

	returnValue = TRUE;
closeFile:
	CloseHandle(file);
freeStream:
	KHDataStreamFree(stream);
	return returnValue;
}
