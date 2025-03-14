/*#include <windowsx.h>
#include <hrsp_remote.h>
#include "window_session.h"
#include <CommCtrl.h>

#define IDM_STREAM_OPEN    0xE001
#define IDM_STREAM_REFRESH 0xE002

#define IDM_STREAM_WINDOW_ENABLE_STREAM      0xE001
#define IDM_STREAM_WINDOW_ALWAYS_ON_TOP      0xE002
#define IDM_STREAM_WINDOW_FULLSCREEN         0xE003
#define IDM_STREAM_WINDOW_SCREEN_LIMIT       0xE004
#define IDM_STREAM_WINDOW_LOCK_WINDOW        0xE005
#define IDM_STREAM_WINDOW_MATCH_ASPECT_RATIO 0xE006
#define IDM_STREAM_WINDOW_PICTURE_IN_PICTURE 0xE007

#define SM_STREAM_DEVICE (WM_USER + 0x01)

#define CLASS_NAME       L"HackontrolRemoteSessionTabStream"
#define CLASS_NAME_POPUP L"HackontrolRemoteSessionTabStreamPopup"

extern HINSTANCE instance;
extern HFONT font;

typedef struct {
	BOOLEAN name : 1;
	BOOLEAN ascending : 1;
} SORTPARAMETER, *PSORTPARAMETER;

typedef struct {
	PCLIENT client;
	HANDLE mutex;
	HWND window;
	HWND border;
	HWND list;
	SORTPARAMETER sort;
} TABSTREAMDATA, *PTABSTREAMDATA;

typedef struct {
	LPWSTR name;
	UINT32 identifierLength;
	PBYTE identifier;
	HRSPREMOTESTREAMDEVICETYPE type;
	PTABSTREAMDATA data;

	struct {
		HANDLE thread;
		HWND window;
		POINT cursor;
		RECT client;

		struct {
			ULONG width;
			ULONG height;
		} target;

		struct {
			BOOLEAN stream : 1;
			BOOLEAN lock : 1;
			BOOLEAN limit : 1;
			BOOLEAN north : 1;
			BOOLEAN east : 1;
			BOOLEAN south : 1;
			BOOLEAN west : 1;
		};
	} popup;
} DEVICEENTRY, *PDEVICEENTRY;

static DWORD WINAPI popupThread(_In_ PDEVICEENTRY entry) {
	if(!entry) {
		return 1;
	}

	entry->popup.window = CreateWindowExW(WS_EX_TOPMOST, CLASS_NAME_POPUP, entry->name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.32942899), (int) (((double) GetSystemMetrics(SM_CYSCREEN)) * 0.390625), NULL, NULL, instance, entry);
	entry->popup.target.width = 16;
	entry->popup.target.height = 9;
	DWORD codeExit = 1;

	if(!entry->popup.window) {
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
	CloseHandle(entry->popup.thread);

	for(size_t i = 0; i < sizeof(entry->popup); i++) {
		((PBYTE) &entry->popup)[i] = 0;
	}

	return codeExit;
}

static void streamToggle(const PDEVICEENTRY entry) {
	DATASTREAM stream = {0};
	BYTE data = entry->type;

	if(!KHOPANStreamAdd(&stream, &data, 1, NULL) || !KHOPANStreamAdd(&stream, entry->identifier, entry->identifierLength, NULL)) {
		KHOPANStreamFree(&stream, NULL);
		return;
	}

	data = entry->popup.stream;

	if(!KHOPANStreamAdd(&stream, &data, 1, NULL)) {
		KHOPANStreamFree(&stream, NULL);
		return;
	}

	HRSPPACKET packet;
	packet.type = HRSP_REMOTE_SERVER_STREAM_ACTION;
	packet.size = stream.size;
	packet.data = stream.data;
	HRSPPacketSend(&entry->data->client->hrsp, &packet, NULL);
	KHOPANStreamFree(&stream, NULL);
}

static LRESULT CALLBACK procedurePopup(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PDEVICEENTRY, entry, window, message, wparam, lparam);
	HMENU menu;
	struct {
		BOOLEAN pictureInPicture : 1;
		BOOLEAN topMost : 1;
	} boolean;

	BOOL status;
	POINT point;
	int length = 0;

	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_CONTEXTMENU:
		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		AppendMenuW(menu, MF_STRING | (entry->popup.stream ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_WINDOW_ENABLE_STREAM, L"Enable Stream");
		AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		boolean.topMost = GetWindowLongW(window, GWL_EXSTYLE) & WS_EX_TOPMOST ? TRUE : FALSE;
		AppendMenuW(menu, MF_STRING | (boolean.topMost ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_WINDOW_ALWAYS_ON_TOP, L"Always On Top");
		AppendMenuW(menu, MF_STRING, IDM_STREAM_WINDOW_FULLSCREEN, L"Fullscreen");
		AppendMenuW(menu, MF_STRING | (entry->popup.limit ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_WINDOW_SCREEN_LIMIT, L"Limit To Screen");
		AppendMenuW(menu, MF_STRING | (entry->popup.lock ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_WINDOW_LOCK_WINDOW, L"Lock Window");
		AppendMenuW(menu, MF_STRING, IDM_STREAM_WINDOW_MATCH_ASPECT_RATIO, L"Match Aspect Ratio");
		boolean.pictureInPicture = GetWindowLongPtrW(window, GWL_STYLE) & WS_POPUP ? TRUE : FALSE;
		AppendMenuW(menu, MF_STRING | (boolean.pictureInPicture ? MF_CHECKED : MF_UNCHECKED), IDM_STREAM_WINDOW_PICTURE_IN_PICTURE, L"Picture in Picture");
		SetForegroundWindow(window);
		status = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_TOPALIGN, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), window, NULL);
		DestroyMenu(menu);

		switch(status) {
		case IDM_STREAM_WINDOW_ENABLE_STREAM:
			entry->popup.stream = !entry->popup.stream;
			streamToggle(entry);
			return 0;
		case IDM_STREAM_WINDOW_ALWAYS_ON_TOP:
			SetWindowPos(window, boolean.topMost ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			return 0;
		case IDM_STREAM_WINDOW_FULLSCREEN:
			return 0;
		case IDM_STREAM_WINDOW_SCREEN_LIMIT:
			entry->popup.limit = !entry->popup.limit;
			return 0;
		case IDM_STREAM_WINDOW_LOCK_WINDOW:
			entry->popup.lock = !entry->popup.lock;
			return 0;
		case IDM_STREAM_WINDOW_MATCH_ASPECT_RATIO:
			return 0;
		case IDM_STREAM_WINDOW_PICTURE_IN_PICTURE:
			SetWindowLongPtrW(window, GWL_STYLE, (boolean.pictureInPicture ? WS_OVERLAPPEDWINDOW : WS_POPUP) | WS_VISIBLE);
			SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			return 0;
		}

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
		if(entry->popup.lock) {
			break;
		}

		GetCursorPos(&entry->popup.cursor);
		GetWindowRect(window, &entry->popup.client);
		SetCapture(window);
		return 1;
	case WM_LBUTTONUP:
		ReleaseCapture();
		return 1;
	case WM_MOUSEMOVE:
		if(entry->popup.lock || !(wparam & MK_LBUTTON)) {
			break;
		}

		GetCursorPos(&point);
		SetWindowPos(window, HWND_TOP, point.x - entry->popup.cursor.x + entry->popup.client.left, point.y - entry->popup.cursor.y + entry->popup.client.top, 0, 0, SWP_NOSIZE);
		return 0;
	case WM_SIZING: {
		if(entry->popup.lock) {
			break;
		}

		switch(wparam) {
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_TOP:
		case WMSZ_TOPLEFT:
		case WMSZ_TOPRIGHT:
			length = (int) (((double) (((PRECT) lparam)->bottom - ((PRECT) lparam)->top)) / ((double) entry->popup.target.height) * ((double) entry->popup.target.width));
			break;
		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			length = (int) (((double) (((PRECT) lparam)->right - ((PRECT) lparam)->left)) / ((double) entry->popup.target.width) * ((double) entry->popup.target.height));
			break;
		}

		switch(wparam) {
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			((PRECT) lparam)->right = ((PRECT) lparam)->left + length;
			return TRUE;
		case WMSZ_LEFT:
			((PRECT) lparam)->top = ((PRECT) lparam)->bottom - length;
			return TRUE;
		case WMSZ_RIGHT:
			((PRECT) lparam)->bottom = ((PRECT) lparam)->top + length;
			return TRUE;
		case WMSZ_TOP:
		case WMSZ_TOPLEFT:
		case WMSZ_TOPRIGHT:
			((PRECT) lparam)->left = ((PRECT) lparam)->right - length;
			return TRUE;
		}

		return FALSE;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

// ----------------------------------------------------------------------------------------------------

static void __stdcall uninitialize(const PULONGLONG data) {
	if(*data) {
		UnregisterClassW(CLASS_NAME_POPUP, instance);
	}
}

static int CALLBACK compare(const PDEVICEENTRY first, const PDEVICEENTRY second, const PSORTPARAMETER parameter) {
	if(!parameter) {
		return 0;
	}

	char compare = first ? second ? parameter->name ? wcscmp(first->name, second->name) : first->type > second->type ? 1 : first->type == second->type ? wcscmp(first->name, second->name) : -1 : 1 : second ? -1 : 0;
	compare = compare ? compare : parameter->name && first && second ? first->type > second->type ? 1 : first->type == second->type ? 0 : -1 : 0;
	return parameter->ascending ? compare : -compare;
}

static void listHeader(const int index, const PTABSTREAMDATA data) {
	if(index < 0 || !data) {
		return;
	}

	HWND header = (HWND) SendMessageW(data->list, LVM_GETHEADER, 0, 0);

	if(!header) {
		return;
	}

	int count = (int) SendMessageW(header, HDM_GETITEMCOUNT, 0, 0);

	if(count < 1) {
		return;
	}

	HDITEMW item = {0};

	for(int i = 0; i < count; i++) {
		item.mask = HDI_FORMAT;
		item.fmt = 0;
		SendMessageW(header, HDM_GETITEM, i, (LPARAM) &item);

		if(i != index) {
			item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			goto setItem;
		}

		if(item.fmt & HDF_SORTUP) {
			item.fmt = (item.fmt & ~HDF_SORTUP) | HDF_SORTDOWN;
		} else if(item.fmt & HDF_SORTDOWN) {
			item.fmt = (item.fmt & ~HDF_SORTDOWN) | HDF_SORTUP;
		} else {
			item.fmt |= HDF_SORTUP;
		}

		data->sort.ascending = item.fmt & HDF_SORTUP ? TRUE : FALSE;
	setItem:
		SendMessageW(header, HDM_SETITEM, i, (LPARAM) &item);
	}

	data->sort.name = index == 0;
	SendMessageW(data->list, LVM_SORTITEMS, (WPARAM) &data->sort, (LPARAM) compare);
}

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG customData, const HWND parent) {
	PTABSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(TABSTREAMDATA));

	if(!data) {
		KHOPANERRORCONSOLE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
		return NULL;
	}

	data->client = client;
	data->mutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE | DELETE);

	if(!data->mutex) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateMutexExW");
		goto freeData;
	}

	data->window = CreateWindowExW(WS_EX_CONTROLPARENT, CLASS_NAME, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!data->window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto closeMutex;
	}

	data->border = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Stream Sources", BS_GROUPBOX | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, data->window, NULL, NULL, NULL);

	if(!data->border) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	SendMessageW(data->border, WM_SETFONT, (WPARAM) font, TRUE);
	data->list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 3, 17, 0, 0, data->window, NULL, NULL, NULL);

	if(!data->list) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	SendMessageW(data->list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	column.cx = (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.13250366);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"Name";

	if(SendMessageW(data->list, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto destroyWindow;
	}

	column.pszText = L"Type";

	if(SendMessageW(data->list, LVM_INSERTCOLUMN, 1, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		goto destroyWindow;
	}

	listHeader(0, data);
	HRSPPACKET packet = {0};
	packet.type = HRSP_REMOTE_SERVER_STREAM_DEVICES;
	HRSPPacketSend(&data->client->hrsp, &packet, NULL);
	*customData = (ULONGLONG) data;
	return data->window;
destroyWindow:
	DestroyWindow(data->window);
closeMutex:
	CloseHandle(data->mutex);
freeData:
	KHOPAN_DEALLOCATE(data);
	return NULL;
}

static BOOLEAN packetHandler(const PCLIENT client, const PULONGLONG customData, const PHRSPPACKET packet) {
	PTABSTREAMDATA data;

	if(packet->size < 1 || !customData || !(data = (PTABSTREAMDATA) *customData)) {
		return FALSE;
	}

	switch(packet->type) {
	case HRSP_REMOTE_CLIENT_STREAM_DEVICES:
		if(((PBYTE) packet->data)[0]) {
			return TRUE;
		}

		if(PostMessageW(data->window, SM_STREAM_DEVICE, packet->size, (LPARAM) packet->data)) {
			packet->data = NULL;
		}

		return TRUE;
	}

	return FALSE;
}

static void openPopup(const PDEVICEENTRY entry) {
	if(entry->popup.thread) {
		PostMessageW(entry->popup.window, WM_CLOSE, 0, 0);
		WaitForSingleObject(entry->popup.thread, INFINITE);
	}

	entry->popup.thread = CreateThread(NULL, 0, popupThread, entry, 0, NULL);

	if(!entry->popup.thread) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateThread");
	}
}

static void freeDeviceEntry(const PDEVICEENTRY entry) {
	KHOPAN_DEALLOCATE(entry->name);
	KHOPAN_DEALLOCATE(entry->identifier);

	if(entry->popup.thread) {
		PostMessageW(entry->popup.window, WM_CLOSE, 0, 0);
		WaitForSingleObject(entry->popup.thread, INFINITE);
	}

	KHOPAN_DEALLOCATE(entry);
}

static void openPopupIndex(const HWND listView, const int index) {
	if(index < 0) {
		return;
	}

	LVITEMW item = {0};
	item.mask = LVIF_PARAM;
	item.iItem = index;

	if(SendMessageW(listView, LVM_GETITEM, 0, (LPARAM) &item)) {
		openPopup((PDEVICEENTRY) item.lParam);
	}
}

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	size_t index = 1;
	int count;
	int i;
	LVITEMW item = {0};
	PDEVICEENTRY entry = NULL;
	UINT32 size;
	HRSPREMOTESTREAMDEVICETYPE type;
	UINT32 nameLength;
	PBYTE pointerName;
	UINT32 x;
	LVHITTESTINFO information = {0};
	RECT bounds;
	HMENU menu;
	HRSPPACKET packet = {0};

	switch(message) {
	case SM_STREAM_DEVICE:
		if(!lparam) {
			return 0;
		}

		while(index < wparam) {
			if(index + 5 > wparam) goto cleanup;
			index += ((((PBYTE) lparam)[index + 1] << 24) | (((PBYTE) lparam)[index + 2] << 16) | (((PBYTE) lparam)[index + 3] << 8) | ((PBYTE) lparam)[index + 4]) + 5;
			if(index + 4 > wparam) goto cleanup;
			index += ((((PBYTE) lparam)[index] << 24) | (((PBYTE) lparam)[index + 1] << 16) | (((PBYTE) lparam)[index + 2] << 8) | ((PBYTE) lparam)[index + 3]) + 4;
			if(index > wparam) goto cleanup;
		}

		if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) {
			goto cleanup;
		}

		count = (int) SendMessageW(data->list, LVM_GETITEMCOUNT, 0, 0);

		for(i = count - 1; i >= 0; i--) {
			item.mask = LVIF_PARAM;
			item.iItem = i;

			if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || !(entry = (PDEVICEENTRY) item.lParam)) {
				SendMessageW(data->list, LVM_DELETEITEM, i, 0);
				continue;
			}

			index = 1;

			while(index < wparam) {
				index += ((((PBYTE) lparam)[index + 1] << 24) | (((PBYTE) lparam)[index + 2] << 16) | (((PBYTE) lparam)[index + 3] << 8) | ((PBYTE) lparam)[index + 4]) + 9;
				size = (((PBYTE) lparam)[index - 4] << 24) | (((PBYTE) lparam)[index - 3] << 16) | (((PBYTE) lparam)[index - 2] << 8) | ((PBYTE) lparam)[index - 1];
				index += size;
				if(entry->identifierLength != size) continue;
				if(!memcmp(((PBYTE) lparam) + index - size, entry->identifier, size)) goto pass;
			}

			freeDeviceEntry(entry);
			SendMessageW(data->list, LVM_DELETEITEM, i, 0);
		pass:
			continue;
		}

		index = 1;

		while(index < wparam) {
			type = ((PBYTE) lparam)[index];
			nameLength = (((PBYTE) lparam)[index + 1] << 24) | (((PBYTE) lparam)[index + 2] << 16) | (((PBYTE) lparam)[index + 3] << 8) | ((PBYTE) lparam)[index + 4];
			pointerName = ((PBYTE) lparam) + index + 5;
			index += nameLength + 9;
			size = (((PBYTE) lparam)[index - 4] << 24) | (((PBYTE) lparam)[index - 3] << 16) | (((PBYTE) lparam)[index - 2] << 8) | ((PBYTE) lparam)[index - 1];
			index += size;
			if(type > HRSP_REMOTE_STREAM_DEVICE_MONITOR) continue;
			count = (int) SendMessageW(data->list, LVM_GETITEMCOUNT, 0, 0);

			for(i = count - 1; i >= 0; i--) {
				item.mask = LVIF_PARAM;
				item.iItem = i;
				if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || !(entry = (PDEVICEENTRY) item.lParam) || entry->identifierLength != size) continue;
				if(!memcmp(((PBYTE) lparam) + index - size, entry->identifier, size)) goto skip;
			}

			entry = KHOPAN_ALLOCATE(sizeof(DEVICEENTRY));
			if(!entry) continue;
			entry->type = type;
			entry->data = data;
			entry->name = KHOPAN_ALLOCATE(nameLength + sizeof(WCHAR));

			if(!entry->name) {
				KHOPAN_DEALLOCATE(entry);
				continue;
			}

			entry->identifierLength = size;
			entry->identifier = KHOPAN_ALLOCATE(size);

			if(!entry->identifier) {
				KHOPAN_DEALLOCATE(entry->name);
				KHOPAN_DEALLOCATE(entry);
				continue;
			}

			for(x = 0; x < nameLength; x++) ((PBYTE) entry->name)[x] = pointerName[x];
			((PBYTE) entry->name)[nameLength] = ((PBYTE) entry->name)[nameLength + 1] = 0;
			for(x = 0; x < size; x++) entry->identifier[x] = ((PBYTE) lparam)[index - size + x];

			for(i = count - 1; i >= 0; i--) {
				item.mask = LVIF_PARAM;
				item.iItem = i;
				if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || compare(entry, (PDEVICEENTRY) item.lParam, &data->sort) <= 0) continue;
				item.iItem++;
				break;
			}

			item.mask = LVIF_PARAM | LVIF_TEXT;
			item.iSubItem = 0;
			item.pszText = entry->name;
			item.lParam = (LPARAM) entry;
			i = (int) SendMessageW(data->list, LVM_INSERTITEM, 0, (LPARAM) &item);

			if(i == -1) {
				KHOPAN_DEALLOCATE(entry->identifier);
				KHOPAN_DEALLOCATE(entry->name);
				KHOPAN_DEALLOCATE(entry);
				continue;
			}

			item.mask = LVIF_TEXT;
			item.iSubItem = 1;
			item.pszText = type == HRSP_REMOTE_STREAM_DEVICE_CAMERA ? L"Camera" : type == HRSP_REMOTE_STREAM_DEVICE_PRIMARY_MONITOR ? L"Monitor (Primary)" : L"Monitor";
			if(!SendMessageW(data->list, LVM_SETITEM, 0, (LPARAM) &item)) SendMessageW(data->list, LVM_DELETEITEM, i, 0);
		skip:
			continue;
		}

		ReleaseMutex(data->mutex);
	cleanup:
		KHOPAN_DEALLOCATE((LPVOID) lparam);
		return 0;
	case WM_CONTEXTMENU:
		GetCursorPos(&information.pt);
		ScreenToClient(data->list, &information.pt);
		GetClientRect(window, &bounds);

		if(information.pt.x < bounds.left || information.pt.x > bounds.right || information.pt.y < bounds.top || information.pt.y > bounds.bottom) {
			break;
		}

		if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) {
			goto skipHit;
		}

		if(SendMessageW(data->list, LVM_HITTEST, 0, (LPARAM) &information) != -1) {
			item.mask = LVIF_PARAM;
			item.iItem = information.iItem;
			SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item);
		}

		ReleaseMutex(data->mutex);
	skipHit:
		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		if(item.lParam) {
			AppendMenuW(menu, MF_STRING, IDM_STREAM_OPEN, L"Open");
			AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
			entry = (PDEVICEENTRY) item.lParam;
		}

		AppendMenuW(menu, MF_STRING, IDM_STREAM_REFRESH, L"Refresh");
		SetForegroundWindow(window);
		count = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_TOPALIGN, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(menu);

		switch(count) {
		case IDM_STREAM_OPEN:
			if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) return 0;
			openPopup(entry);
			ReleaseMutex(data->mutex);
			return 0;
		case IDM_STREAM_REFRESH:
			packet.type = HRSP_REMOTE_SERVER_STREAM_DEVICES;
			HRSPPacketSend(&data->client->hrsp, &packet, NULL);
			return 0;
		}

		break;
	case WM_CTLCOLORSTATIC:
		SetDCBrushColor((HDC) wparam, 0xF9F9F9);
		return (LRESULT) GetStockObject(DC_BRUSH);
	case WM_DESTROY:
		WaitForSingleObject(data->mutex, INFINITE);

		for(i = 0; i < SendMessageW(data->list, LVM_GETITEMCOUNT, 0, 0); i++) {
			item.mask = LVIF_PARAM;
			item.iItem = i;
			if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || !item.lParam) continue;
			freeDeviceEntry((PDEVICEENTRY) item.lParam);
		}

		CloseHandle(data->mutex);
		KHOPAN_DEALLOCATE(data);
		return 0;
	case WM_NOTIFY:
		if(!lparam) {
			break;
		}

		switch(((LPNMHDR) lparam)->code) {
		case LVN_COLUMNCLICK:
			if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) return 0;
			listHeader((UINT) ((LPNMLISTVIEW) lparam)->iSubItem, data);
			goto releaseMutex;
		case LVN_KEYDOWN:
			if(((LPNMLVKEYDOWN) lparam)->wVKey != VK_SPACE) return 0;
			if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) return 0;
			openPopupIndex(data->list, (int) SendMessageW(data->list, LVM_GETNEXTITEM, -1, LVNI_SELECTED));
			goto releaseMutex;
		case NM_DBLCLK:
			if(WaitForSingleObject(data->mutex, INFINITE) == WAIT_FAILED) return 0;
			openPopupIndex(data->list, ((LPNMITEMACTIVATE) lparam)->iItem);
			goto releaseMutex;
		}

		break;
	releaseMutex:
		ReleaseMutex(data->mutex);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(data->border, HWND_TOP, 0, 0, bounds.right - bounds.left - 2, bounds.bottom - bounds.top, SWP_NOMOVE);
		SetWindowPos(data->list, HWND_TOP, 0, 0, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 21, SWP_NOMOVE);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

void __stdcall WindowSessionTabStream(const PTABINITIALIZER tab) {
	tab->name = L"Stream";
	tab->uninitialize = uninitialize;
	tab->clientInitialize = clientInitialize;
	tab->packetHandler = packetHandler;
	tab->windowClass.lpfnWndProc = procedure;
	tab->windowClass.lpszClassName = CLASS_NAME;
	WNDCLASSEXW windowClass = {0};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = procedurePopup;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	windowClass.lpszClassName = CLASS_NAME_POPUP;
	tab->data = RegisterClassExW(&windowClass);
}*/
