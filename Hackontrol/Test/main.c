#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	if(!KHWin32EnablePrivilegeW(SE_DEBUG_NAME)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32EnablePrivilegeW");
		return 1;
	}

	return 0;
}
