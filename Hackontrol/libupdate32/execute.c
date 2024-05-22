#include "update.h"
#include <khopanstring.h>

void ExecuteLibdll32() {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		return;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_LIBDLL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileLibdll32) {
		KHWin32DialogErrorW(GetLastError(), L"KHFormatMessageW");
		return;
	}

	if(!KHWin32StartDynamicLibraryW(pathFileLibdll32, FUNCTION_LIBDLL32, NULL)) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32StartDynamicLibraryW");
	}
}
