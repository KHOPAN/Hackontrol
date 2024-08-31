#include <stdio.h>
#include <khopanwin32.h>
//#include <bcrypt.h>

int main(int argc, char** argv) {
	KHWIN32_ERROR(KHHRESULT_DECODE(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_APPEXEC_INVALID_HOST_GENERATION)), L"main");
	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWIN32_LAST_ERROR(L"KHWin32GetWindowsDirectoryW");
		return 1;
	}

	printf("Path: %ws\n", pathFolderWindows);
	LocalFree(pathFolderWindows);
	/*BCRYPT_ALG_HANDLE handle;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&handle, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	status = BCryptCloseAlgorithmProvider(handle, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR(status, L"BCryptCloseAlgorithmProvider");
		return 1;
	}*/

	return 0;
}
