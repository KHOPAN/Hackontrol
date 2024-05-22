#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	KHWin32StartDynamicLibraryA("D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Debug\\libdll32.dll", "DownloadFile", NULL);
	return 0;
}
