#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	LPSTR pathFileCmd = KHWin32GetCmdFileA();

	if(!pathFileCmd) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32GetCmdFileW");
		return 1;
	}

	printf("Command: %s\n", pathFileCmd);
	return 0;
}
