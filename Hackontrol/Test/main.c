#include <Windows.h>

int main(int argc, char** argv) {
	MessageBoxW(NULL, L"Hello, world!", L"Information", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
	return 0;
}
