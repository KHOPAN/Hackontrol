#include <stdio.h>
#include <khopanwin32.h>
#include <hackontrol.h>

int main(int argc, char** argv) {
	LPWSTR pathFolderHackontrol = HackontrolGetDirectory(TRUE);

	if(!pathFolderHackontrol) {
		KHWin32ConsoleErrorW(GetLastError(), L"HackontrolGetDirectory");
		return 1;
	}

	printf("Path: %ws\n", pathFolderHackontrol);
	LocalFree(pathFolderHackontrol);
	return 0;
}
