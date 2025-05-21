#include <Windows.h>

#define CLASS_NAME L"HackontrolInstallerWindow"

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR commandLine, _In_ int showWindow) {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize;
	windowClass.style;
	windowClass.lpfnWndProc;
	windowClass.cbClsExtra;
	windowClass.cbWndExtra;
	windowClass.hInstance;
	windowClass.hIcon;
	windowClass.hCursor;
	windowClass.hbrBackground;
	windowClass.lpszMenuName;
	windowClass.lpszClassName = CLASS_NAME;
	windowClass.hIconSm;
	ATOM atom = RegisterClassExW(&windowClass);

	if(!atom) {
		return 1;
	}

	UnregisterClassW(atom, instance);
	return 0;
}
