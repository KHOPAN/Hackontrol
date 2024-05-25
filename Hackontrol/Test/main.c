#include <stdio.h>
#include <khopanwin32.h>
#include <hackontrol.h>

int main(int argc, char** argv) {
	if(!HackontrolEnsureDirectoryExistence(L"C:\\ProgramData\\Microsoft\\DeviceSync")) {
		KHWin32ConsoleErrorW(GetLastError(), L"HackontrolEnsureDirectoryExistence");
		return 1;
	}

	return 0;
}
