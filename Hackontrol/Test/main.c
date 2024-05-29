#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	if(!KHWin32ExecuteRundll32FunctionW(L"D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Debug\\libupdate32.dll", "Update", NULL, FALSE)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32ExecuteRundll32FunctionW");
	}

	printf("Done\n");
	return 0;
}
