#include <stdio.h>
#include <khopanwin32.h>
#include <hackontrol.h>

int main(int argc, char** argv) {
	DataStream stream = {0};
	CURLcode code;

	if(!HackontrolDownloadData(&stream, "https://api.quotable.io/random", FALSE, &code)) {
		KHCURLConsoleErrorW(code, L"HackontrolDownloadData");
		return 1;
	}

	if(!HackontrolWriteFile(L"C:\\Users\\puthi\\Downloads\\quote.json", &stream)) {
		KHWin32ConsoleErrorW(GetLastError(), L"HackontrolWriteFile");
	}

	KHDataStreamFree(&stream);
	return 0;
}
