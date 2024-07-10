#include <khopanstring.h>
#include <khopanwin32.h>
#include <hackontrol.h>
#include "Kernel.h"
#include "exception.h"

#define FUNCTION_LIBDLL32 L"Execute"

void Kernel_initiateRestart(JNIEnv* const environment, const jclass class, const jboolean update) {
	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		HackontrolThrowWin32Error(environment, L"HackontrolGetDirectory");
		return;
	}

	LPWSTR pathFileLibdll32 = KHFormatMessageW(L"%ws\\" FILE_LIBDLL32, pathFolderHackontrol);
	LocalFree(pathFolderHackontrol);

	if(!pathFileLibdll32) {
		HackontrolThrowWin32Error(environment, L"KHFormatMessageW");
		return;
	}

	LPWSTR argumentFileLibdll32 = KHFormatMessageW(L"%lu %u", GetCurrentProcessId(), update);

	if(!argumentFileLibdll32) {
		HackontrolThrowWin32Error(environment, L"KHFormatMessageW");
		LocalFree(pathFileLibdll32);
		return;
	}

	BOOL result = KHWin32StartDynamicLibraryW(pathFileLibdll32, FUNCTION_LIBDLL32, argumentFileLibdll32);
	LocalFree(argumentFileLibdll32);
	LocalFree(pathFileLibdll32);

	if(!result) {
		HackontrolThrowWin32Error(environment, L"KHWin32StartDynamicLibraryW");
	}
}
