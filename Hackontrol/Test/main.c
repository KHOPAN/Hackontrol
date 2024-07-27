#include <stdio.h>
#include <khopanwin32.h>
#include <khopanarray.h>

int main(int argc, char** argv) {
	ArrayList list;
	int returnValue = 1;

	if(!KHArrayInitialize(&list, sizeof(unsigned long long))) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArrayInitialize");
		goto freeList;
	}

	for(unsigned long long i = 0; i < 10; i++) {
		if(!KHArrayAdd(&list, &i)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHArrayAdd");
			goto freeList;
		}
	}

	if(!KHArrayRemove(&list, 5)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArrayRemove");
		goto freeList;
	}

	for(size_t i = 0; i < list.elementCount; i++) {
		unsigned long long* number;
		
		if(!KHArrayGet(&list, i, &number)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHArrayGet");
			goto freeList;
		}

		printf("Number: %llu\n", *number);
	}

	returnValue = 0;
freeList:
	KHArrayFree(&list);
	return returnValue;
}
