#include <khopanerror.h>
#include <khopanstring.h>
#include "extractor.h"
#include "initialize.h"
#include "resource.h"

BOOL ExtractJRE() {
	HINSTANCE instance = GetProgramInstance();

	if(!instance) {
		KHWin32DialogErrorW(ERROR_INVALID_HANDLE, L"GetProgramInstance");
		return TRUE;
	}

	HRSRC resourceHandle = FindResourceW(instance, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
	
	if(!resourceHandle) {
		KHWin32DialogErrorW(GetLastError(), L"FindResourceW");
		return TRUE;
	}

	DWORD resourceSize = SizeofResource(instance, resourceHandle);
	
	if(!resourceSize) {
		KHWin32DialogErrorW(GetLastError(), L"SizeofResource");
		return TRUE;
	}

	HGLOBAL resource = LoadResource(instance, resourceHandle);

	if(!resource) {
		KHWin32DialogErrorW(GetLastError(), L"LoadResource");
		return TRUE;
	}

	BYTE* data = LockResource(resource);

	if(!data) {
		KHWin32DialogErrorW(GetLastError(), L"LockResource");
		return TRUE;
	}

	LPWSTR message = KHFormatMessageW(L"Resource size: %u", resourceSize);
	
	if(!message) {
		KHWin32DialogErrorW(ERROR_FUNCTION_FAILED, L"KHFormatMessageW");
		return TRUE;
	}

	MessageBoxW(NULL, message, L"Information", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	LocalFree(message);
	return FALSE;
}
