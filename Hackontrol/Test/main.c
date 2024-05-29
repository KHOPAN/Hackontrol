#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	if(!KHWin32ExecuteCommandW(L"timeout 5", TRUE)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32ExecuteRundll32FunctionW");
	}

	printf("Done\n");
	return 0;
}
