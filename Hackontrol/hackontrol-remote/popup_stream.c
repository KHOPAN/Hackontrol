#include "popup_stream.h"

#define CLASS_NAME L"HackontrolRemotePopupStream"

extern HINSTANCE instance;

BOOLEAN PopupStreamInitialize() {
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProcW;
	windowClass.hInstance = instance;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassExW(&windowClass)) {
		KHOPANLASTERRORMESSAGE_WIN32(L"RegisterClassExW");
		return FALSE;
	}

	return TRUE;
}

BOOLEAN PopupStreamSessionInitialize(const PPOPUPSTREAMSESSION session) {
	if(!session) {
		return FALSE;
	}

	return TRUE;
}

void PopupStreamSessionCleanup(const PPOPUPSTREAMSESSION session) {

}

void PopupStreamCleanup() {
	UnregisterClassW(CLASS_NAME, instance);
}
