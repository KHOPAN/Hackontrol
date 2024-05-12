#include <khopanwin32.h>
#include <khopanstring.h>
#include <stdio.h>

#define CLOSE_HANDLE(x) if(!CloseHandle(x)) KHWin32DialogErrorW(GetLastError(), L"CloseHandle")
#define MESSAGE_BOX(x) MessageBoxW(NULL, x, L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL)
#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define FUNCTION_NAME  L"Execute"
#define FILE_NAME      L"ctrl32.dll"
#define FILE_NAME_PATH L"System32\\" FILE_NAME

#define RUNDLL32PATH   L"System32\\rundll32.exe"
#define URL_VERSION_FILE "https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/version.json"

#define FILE_CTRL32       L"ctrl32.dll"
#define FILE_RUNDLL32     L"rundll32.exe"
#define FOLDER_SYSTEM32   L"System32"
#define FUNCTION_CTRL32   L"Execute"

int main(int argc, char** argv) {
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		return;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_RUNDLL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileRundll32) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	printf("Rundll32: %ws\nArgument: %ws\n", pathFileRundll32, FILE_RUNDLL32 L" " FILE_CTRL32 L"," FUNCTION_CTRL32);
	FREE(pathFileRundll32);
}
