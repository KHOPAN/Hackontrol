#include <stdio.h>
#include <khopanwin32.h>
#include <khopandatastream.h>

int main(int argc, char** argv) {
	DataStream stream = {0};

	for(unsigned int i = 0; i < 10; i++) {
		if(!KHDataStreamAdd(&stream, "H", 1)) {
			KHWin32ConsoleErrorW(GetLastError(), L"KHDataStreamAdd");
			return 1;
		}
	}

	if(!KHDataStreamAdd(&stream, "", 1)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHDataStreamAdd");
		return 1;
	}

	printf("%s\n", (char*) stream.data);

	if(!KHDataStreamFree(&stream)) {
		KHWin32ConsoleErrorW(GetLastError(), L"KHDataStreamFree");
		return 1;
	}

	return 0;
}
