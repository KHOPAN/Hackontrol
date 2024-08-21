#include <stdio.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	HANDLE mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!mutex) {
		KHWin32ConsoleErrorW(GetLastError(), L"CreateMutexExW");
		return 1;
	}

	CloseHandle(mutex);
	printf("Wait: %u\n", WaitForSingleObject(mutex, INFINITE));

	if(!ReleaseMutex(mutex)) {
		KHWin32ConsoleErrorW(GetLastError(), L"ReleaseMutex");
		goto closeMutex;
	}
closeMutex:
	//CloseHandle(mutex);
	return 0;
}
