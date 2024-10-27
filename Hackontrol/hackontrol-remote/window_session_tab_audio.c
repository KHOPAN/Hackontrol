#include "window_session_tabs.h"
#include <CommCtrl.h>
#include <hrsp_remote.h>

#define CLASS_NAME L"HackontrolRemoteSessionTabAudio"

#define IDM_AUDIO_CAPTURE 0xE001
#define IDM_AUDIO_REFRESH 0xE002

#pragma warning(disable: 6001)
#pragma warning(disable: 6385)
#pragma warning(disable: 6386)
#pragma warning(disable: 26454)

extern HFONT font;

typedef struct {
	BOOL sortName;
	BOOL ascending;
} SORTPARAMETER, *PSORTPARAMETER;

typedef enum {
	AUDIO_DEVICE_ACTIVE = 1,
	AUDIO_DEVICE_DISABLED,
	AUDIO_DEVICE_NOTPRESENT,
	AUDIO_DEVICE_UNPLUGGED
} AUDIODEVICESTATE;

typedef struct {
	LPWSTR identifier;
	LPWSTR name;
	AUDIODEVICESTATE state;
} AUDIODEVICE, *PAUDIODEVICE;

typedef struct {
	PCLIENT client;
	HWND border;
	HWND list;
	UINT sortColumn;
	UINT deviceCount;
	PAUDIODEVICE devices;
} TABAUDIODATA, *PTABAUDIODATA;

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG customData, const HWND parent) {
	PTABAUDIODATA data = KHOPAN_ALLOCATE(sizeof(TABAUDIODATA));

	if(KHOPAN_ALLOCATE_FAILED(data)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return NULL;
	}

	data->client = client;
	HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, CLASS_NAME, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	data->border = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Audio Devices", BS_GROUPBOX | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window, NULL, NULL, NULL);

	if(!data->border) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		DestroyWindow(window);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	SendMessageW(data->border, WM_SETFONT, (WPARAM) font, TRUE);
	data->list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 3, 17, 0, 0, window, NULL, NULL, NULL);

	if(!data->list) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		DestroyWindow(window);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	SendMessageW(data->list, WM_SETFONT, (WPARAM) font, TRUE);
	SendMessageW(data->list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.13250366);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"Name";

	if(SendMessageW(data->list, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		DestroyWindow(window);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	column.pszText = L"State";

	if(SendMessageW(data->list, LVM_INSERTCOLUMN, 1, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		DestroyWindow(window);
		KHOPAN_DEALLOCATE(data);
		return NULL;
	}

	HRSPSendTypePacket(client->socket, &client->hrsp, HRSP_REMOTE_SERVER_AUDIO_QUERY_DEVICE, NULL);
	*customData = (ULONGLONG) data;
	return window;
}

static int CALLBACK compareList(PAUDIODEVICE first, PAUDIODEVICE second, PSORTPARAMETER parameter) {
	if(!first || !second || !parameter) {
		return 0;
	}

	int compareName = wcscmp(first->name, second->name);
	int compareState = first->state == second->state ? 0 : first->state > second->state ? 1 : -1;
	return (parameter->sortName ? compareName ? compareName : compareState : compareState ? compareState : compareName) * (parameter->ascending ? 1 : -1);
}

static void sortListView(const PTABAUDIODATA data, const int index) {
	HWND header = (HWND) SendMessageW(data->list, LVM_GETHEADER, 0, 0);

	if(!header) {
		return;
	}

	HDITEMW item = {0};
	SORTPARAMETER parameter;

	if(index < 0) {
		item.mask = HDI_FORMAT;
		SendMessageW(header, HDM_GETITEM, data->sortColumn, (LPARAM) &item);

		if(!(item.fmt & HDF_SORTUP) && !(item.fmt & HDF_SORTDOWN)) {
			item.fmt |= HDF_SORTUP;
			parameter.ascending = TRUE;
			SendMessageW(header, HDM_SETITEM, data->sortColumn, (LPARAM) &item);
		} else {
			parameter.ascending = item.fmt & HDF_SORTUP;
		}

		goto sortItem;
	}

	int count = (int) SendMessageW(header, HDM_GETITEMCOUNT, 0, 0);

	if(count < 1) {
		return;
	}

	data->sortColumn = (UINT) index;

	for(UINT i = 0; i < (UINT) count; i++) {
		item.mask = HDI_FORMAT;
		SendMessageW(header, HDM_GETITEM, i, (LPARAM) &item);

		if(data->sortColumn != i) {
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

		parameter.ascending = item.fmt & HDF_SORTUP;
	setItem:
		SendMessageW(header, HDM_SETITEM, i, (LPARAM) &item);
	}
sortItem:
	parameter.sortName = data->sortColumn == 0;
	SendMessageW(data->list, LVM_SORTITEMS, (WPARAM) &parameter, (LPARAM) compareList);
}

static BOOL __stdcall packetHandler(const PCLIENT client, const PULONGLONG customData, const PHRSPPACKET packet) {
	if(packet->type != HRSP_REMOTE_CLIENT_AUDIO_DEVICE_RESULT) {
		return FALSE;
	}

	if(packet->size < 4) {
		return TRUE;
	}

	UINT count = (packet->data[0] << 24) | (packet->data[1] << 16) | (packet->data[2] << 8) | packet->data[3];

	if(!count) {
		return TRUE;
	}

	size_t size = sizeof(AUDIODEVICE) * count;
	PAUDIODEVICE devices = KHOPAN_ALLOCATE(size);

	if(KHOPAN_ALLOCATE_FAILED(devices)) {
		return TRUE;
	}

	size_t pointer;

	for(pointer = 0; pointer < size; pointer++) {
		((PBYTE) devices)[pointer] = 0;
	}

	pointer = 4;

	for(UINT i = 0; i < count; i++) {
		if(packet->size < pointer + 5) {
			goto functionExit;
		}

		devices[i].state = packet->data[pointer];
		UINT length = (packet->data[pointer + 1] << 24) | (packet->data[pointer + 2] << 16) | (packet->data[pointer + 3] << 8) | packet->data[pointer + 4];
		pointer += 5;

		if(!length || packet->size < pointer + length) {
			goto functionExit;
		}

		devices[i].name = KHOPAN_ALLOCATE(length + sizeof(WCHAR));

		if(KHOPAN_ALLOCATE_FAILED(devices[i].name)) {
			goto functionExit;
		}

		for(size = 0; size < length; size++) {
			((PBYTE) devices[i].name)[size] = packet->data[pointer + size];
		}

		pointer += length;
		((PBYTE) devices[i].name)[length] = 0;
		((PBYTE) devices[i].name)[length + 1] = 0;

		if(packet->size < pointer + 4) {
			goto functionExit;
		}

		length = (packet->data[pointer] << 24) | (packet->data[pointer + 1] << 16) | (packet->data[pointer + 2] << 8) | packet->data[pointer + 3];
		pointer += 4;

		if(!length || packet->size < pointer + length) {
			goto functionExit;
		}

		devices[i].identifier = KHOPAN_ALLOCATE(length + sizeof(WCHAR));

		if(KHOPAN_ALLOCATE_FAILED(devices[i].identifier)) {
			goto functionExit;
		}

		for(size = 0; size < length; size++) {
			((PBYTE) devices[i].identifier)[size] = packet->data[pointer + size];
		}

		pointer += length;
		((PBYTE) devices[i].identifier)[length] = 0;
		((PBYTE) devices[i].identifier)[length + 1] = 0;
	}

	PTABAUDIODATA data = (PTABAUDIODATA) *customData;

	if(data->devices) {
		for(pointer = 0; pointer < data->deviceCount; pointer++) {
			KHOPAN_DEALLOCATE(data->devices[pointer].identifier);
			KHOPAN_DEALLOCATE(data->devices[pointer].name);
		}

		KHOPAN_DEALLOCATE(data->devices);
	}

	data->deviceCount = count;
	data->devices = devices;
	SendMessageW(data->list, LVM_DELETEALLITEMS, 0, 0);
	LVITEMW item = {0};

	for(pointer = 0; pointer < count; pointer++) {
		item.mask = LVIF_PARAM | LVIF_TEXT;
		item.iItem = (int) pointer;
		item.iSubItem = 0;
		item.lParam = (LPARAM) &data->devices[count - pointer - 1];
		item.pszText = ((PAUDIODEVICE) item.lParam)->name;
		SendMessageW(data->list, LVM_INSERTITEM, 0, (LPARAM) &item);
		item.mask = LVIF_TEXT;
		item.iSubItem = 1;

		switch(((PAUDIODEVICE) item.lParam)->state) {
		case AUDIO_DEVICE_ACTIVE:
			item.pszText = L"Active";
			break;
		case AUDIO_DEVICE_DISABLED:
			item.pszText = L"Disabled";
			break;
		case AUDIO_DEVICE_NOTPRESENT:
			item.pszText = L"Not Present";
			break;
		case AUDIO_DEVICE_UNPLUGGED:
			item.pszText = L"Unplugged";
			break;
		}

		SendMessageW(data->list, LVM_SETITEM, 0, (LPARAM) &item);
	}

	sortListView(data, -1);
	return TRUE;
functionExit:
	for(pointer = 0; pointer < count; pointer++) {
		if(devices[pointer].identifier) {
			KHOPAN_DEALLOCATE(devices[pointer].identifier);
		}

		if(devices[pointer].name) {
			KHOPAN_DEALLOCATE(devices[pointer].name);
		}
	}

	KHOPAN_DEALLOCATE(devices);
	return TRUE;
}

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABAUDIODATA, data, window, message, wparam, lparam);
	RECT bounds;
	LVHITTESTINFO information = {0};
	LVITEMW item = {0};
	HMENU menu;
	BOOL option;

	switch(message) {
	case WM_DESTROY:
		if(data->devices) {
			for(UINT i = 0; i < data->deviceCount; i++) {
				KHOPAN_DEALLOCATE(data->devices[i].identifier);
				KHOPAN_DEALLOCATE(data->devices[i].name);
			}

			KHOPAN_DEALLOCATE(data->devices);
		}

		KHOPAN_DEALLOCATE(data);
		return 0;
	case WM_SIZE:
		GetClientRect(window, &bounds);
		SetWindowPos(data->border, HWND_TOP, 0, 0, bounds.right - bounds.left - 2, bounds.bottom - bounds.top, SWP_NOMOVE);
		SetWindowPos(data->list, HWND_TOP, 0, 0, bounds.right - bounds.left - 8, bounds.bottom - bounds.top - 21, SWP_NOMOVE);
		return 0;
	case WM_CTLCOLORSTATIC:
		SetDCBrushColor((HDC) wparam, 0xF9F9F9);
		return (LRESULT) GetStockObject(DC_BRUSH);
	case WM_NOTIFY:
		if(!lparam || ((LPNMHDR) lparam)->code != LVN_COLUMNCLICK) {
			break;
		}

		sortListView(data, ((LPNMLISTVIEW) lparam)->iSubItem);
		return 0;
	case WM_CONTEXTMENU:
		GetCursorPos(&information.pt);
		ScreenToClient(data->list, &information.pt);
		GetClientRect(window, &bounds);

		if(information.pt.x < bounds.left || information.pt.x > bounds.right || information.pt.y < bounds.top || information.pt.y > bounds.bottom) {
			return 0;
		}

		if(SendMessageW(data->list, LVM_HITTEST, 0, (LPARAM) &information) != -1) {
			item.mask = LVIF_PARAM;
			SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item);
		}

		menu = CreatePopupMenu();

		if(!menu) {
			return 0;
		}

		if(item.lParam) {
			AppendMenuW(menu, MF_STRING, IDM_AUDIO_CAPTURE, L"Capture");
			AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		}

		AppendMenuW(menu, MF_STRING, IDM_AUDIO_REFRESH, L"Refresh");
		SetForegroundWindow(window);
		option = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_TOPALIGN, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(menu);

		switch(option) {
		case IDM_AUDIO_CAPTURE:
			return 0;
		case IDM_AUDIO_REFRESH:
			HRSPSendTypePacket(data->client->socket, &data->client->hrsp, HRSP_REMOTE_SERVER_AUDIO_QUERY_DEVICE, NULL);
			return 0;
		}

		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

void __stdcall WindowSessionTabAudio(const PTABINITIALIZER tab) {
	tab->name = L"Audio";
	tab->clientInitialize = clientInitialize;
	tab->packetHandler = packetHandler;
	tab->windowClass.lpfnWndProc = procedure;
	tab->windowClass.lpszClassName = CLASS_NAME;
}
