#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	if(!KHWin32ExecuteCommandA("start chrome https://www.youtube.com")) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHWin32ExecuteCommandW");
		return 1;
	}

	return 0;
}
