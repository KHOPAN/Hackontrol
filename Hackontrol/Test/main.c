#include <khopanwin32.h>

int main(int argc, char** argv) {
	HMODULE libraryJava = LoadLibraryW(L"C:\\Program Files\\Java\\jdk-21\\bin\\server\\jvm.dll");

	if(!libraryJava) {
		KHWin32ConsoleErrorW(GetLastError(), L"LoadLibraryW");
		return 1;
	}

	if(!FreeLibrary(libraryJava)) {
		KHWin32ConsoleErrorW(GetLastError(), L"LoadLibraryW");
		return 1;
	}

	return 0;
}
