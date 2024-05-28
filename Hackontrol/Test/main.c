#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	if(!KHWin32ExecuteRundll32FunctionW(L"D:\\GitHub Repository\\Hackontrol\\release\\GPURender.dll", "Execute", TRUE)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32ExecuteRundll32FunctionW");
	}

	printf("Hello, world!\n");
	Sleep(3000);
	printf("Multithreading test\n");
	return 0;
}
