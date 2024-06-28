#include <khopanwin32.h>

int main(int argc, char** argv) {
	HKEY key;
	LSTATUS error = RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\KHOPAN", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, NULL);

	if(error) {
		KHWin32ConsoleErrorW(error, L"RegCreateKeyExW");
		return 1;
	}

	int returnValue = 1;

	if(!KHWin32RegistrySetStringValueW(key, NULL, L"Hello, world!")) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32RegistrySetStringValueW");
		goto closeKey;
	}

	if(!KHWin32RegistrySetStringValueW(key, L"TestKey", L"Test key value")) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32RegistrySetStringValueW");
		goto closeKey;
	}

	returnValue = 0;
closeKey:
	RegCloseKey(key);
	return returnValue;
}
