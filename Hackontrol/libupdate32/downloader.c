#define _CRT_SECURE_NO_WARNINGS
#include <khopancurl.h>
#include <khopanstring.h>
#include "update.h"

static size_t write_file(void* data, size_t size, size_t count, FILE* stream) {
	return fwrite(data, size, count, stream);
}

void DownloadLatestLibdll32() {
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_global_init");
		return;
	}

	CURL* curl = curl_easy_init();

	if(!curl) {
		KHCURLDialogErrorW(CURLE_FAILED_INIT, L"curl_easy_init");
		goto globalCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_URL, URL_LIBDLL32_FILE);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto easyCleanup;
	}

	LPWSTR pathFolderWindows = KHWin32GetWindowsDirectoryW();

	if(!pathFolderWindows) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		goto easyCleanup;
	}

	LPWSTR pathFileRundll32 = KHFormatMessageW(L"%ws\\" FOLDER_SYSTEM32 L"\\" FILE_LIBDLL32, pathFolderWindows);
	FREE(pathFolderWindows);

	if(!pathFileRundll32) {
		KHWin32DialogErrorW(GetLastError(), L"KHWin32GetWindowsDirectoryW");
		goto easyCleanup;
	}

	FILE* file;
	errno_t error = _wfopen_s(&file, pathFileRundll32, L"wb");
	FREE(pathFileRundll32);

	if(error || !file) {
		MessageBoxA(NULL, strerror(error), "Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
		goto easyCleanup;
	}

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_setopt");
		goto closeFile;
	}

	code = curl_easy_perform(curl);

	if(code != CURLE_OK) {
		KHCURLDialogErrorW(code, L"curl_easy_perform");
	}

closeFile:
	fclose(file);
easyCleanup:
	curl_easy_cleanup(curl);
globalCleanup:
	curl_global_cleanup();
}
