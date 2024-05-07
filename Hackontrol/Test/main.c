#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	LPSTR narrowTest = KHWin32GetWindowsDirectoryA();
	LPWSTR wideTest = KHWin32GetWindowsDirectoryW();

	if(!narrowTest) {
		printf("Narrow Error\n");
		return 1;
	}

	if(!wideTest) {
		printf("Wide Error\n");
		return 1;
	}

	printf("Narrow: %s\n", narrowTest);
	printf("Wide: %ws\n", wideTest);
	LocalFree(narrowTest);
	LocalFree(wideTest);
	return 0;
}
