#include "window_session_stream_window.h"

#define IDM_STREAM_WINDOW_PICTURE_IN_PICTURE 0xE001

extern HINSTANCE instance;

DWORD WINAPI popupThread(_In_ PDEVICEENTRY entry) {
	if(!entry || !entry->popup) {
		return 1;
	}

	entry->popup->window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME_POPUP, entry->name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.32942899), (int) (((double) GetSystemMetrics(SM_CYSCREEN)) * 0.390625), NULL, NULL, instance, NULL);
	DWORD codeExit = 1;

	if(!entry->popup->window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto functionExit;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
functionExit:
	CloseHandle(entry->popup->thread);

	for(size_t i = 0; i < sizeof(POPUPDATA); i++) {
		((PBYTE) entry->popup)[i] = 0;
	}

	return codeExit;
}

LRESULT CALLBACK procedurePopup(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	HMENU menu;
	BOOLEAN pictureInPicture;
	BOOL status;
	PAINTSTRUCT paintStruct;
	HDC context;
	HDC memoryContext;
	RECT bounds;
	HBITMAP bitmap;
	HBITMAP oldBitmap;
	HBRUSH brush;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_CONTEXTMENU:
		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		pictureInPicture = GetWindowLongPtrW(window, GWL_STYLE) & WS_POPUP ? TRUE : FALSE;
		AppendMenuW(menu, MF_STRING | (pictureInPicture ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_WINDOW_PICTURE_IN_PICTURE, L"Picture in Picture");
		SetForegroundWindow(window);
		status = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_TOPALIGN, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), window, NULL);
		DestroyMenu(menu);

		switch(status) {
		case IDM_STREAM_WINDOW_PICTURE_IN_PICTURE:
			SetWindowLongPtrW(window, GWL_STYLE, (pictureInPicture ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_VISIBLE);
			SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			return 0;
		}

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
		return 1;
	case WM_LBUTTONUP:
		return 1;
	case WM_MOUSEMOVE:
		return 0;
	case WM_PAINT:
		context = BeginPaint(window, &paintStruct);
		memoryContext = CreateCompatibleDC(context);
		GetClientRect(window, &bounds);
		bitmap = CreateCompatibleBitmap(context, bounds.right, bounds.bottom);
		oldBitmap = SelectObject(memoryContext, bitmap);
		brush = GetStockObject(DC_BRUSH);
		SetDCBrushColor(memoryContext, 0x000000);
		FillRect(memoryContext, &bounds, brush);
		BitBlt(context, 0, 0, bounds.right, bounds.bottom, memoryContext, 0, 0, SRCCOPY);
		SelectObject(memoryContext, oldBitmap);
		DeleteObject(bitmap);
		DeleteDC(memoryContext);
		EndPaint(window, &paintStruct);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}
