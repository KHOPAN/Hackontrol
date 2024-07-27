#include <stdio.h>
#include <khopanwin32.h>
#include <khopanarray.h>

int main(int argc, char** argv) {
	ArrayList list;
	int returnValue = 1;

	if(!KHArrayInitialize(&list, sizeof(unsigned char))) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHArrayInitialize");
		goto freeList;
	}

	for(unsigned char i = 1; i <= 10; i++) {
		if(!KHArrayAdd(&list, &i)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHArrayAdd");
			goto freeList;
		}
	}

	for(size_t i = 0; i < list.elementCount; i++) {
		unsigned char* number;
		
		if(!KHArrayGet(&list, i, &number)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHArrayGet");
			goto freeList;
		}

		printf("Number: %d\n", *number);
	}

	returnValue = 0;
freeList:
	KHArrayFree(&list);
	return returnValue;
}
