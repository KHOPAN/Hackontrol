#include <hrsp_remote.h>
#include "window_session.h"
#include <CommCtrl.h>

#define IDM_STREAM_REFRESH 0xE001

#define CLASS_NAME L"HackontrolRemoteSessionTabAudio"

extern HFONT font;

typedef struct {
	PCLIENT client;
	HWND border;
	HWND list;
} TABSTREAMDATA, *PTABSTREAMDATA;

typedef struct {
	LPWSTR name;
	UINT32 identifierLength;
	PBYTE identifier;
} DEVICEENTRY, *PDEVICEENTRY;

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG customData, const HWND parent) {
	PTABSTREAMDATA data = KHOPAN_ALLOCATE(sizeof(TABSTREAMDATA));

	if(!data) {
		KHOPANERRORCONSOLE_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"KHOPAN_ALLOCATE");
		return NULL;
	}

	data->client = client;
	HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, CLASS_NAME, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto freeData;
	}

	data->border = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"Button", L"Stream Sources", BS_GROUPBOX | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window, NULL, NULL, NULL);

	if(!data->border) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	SendMessageW(data->border, WM_SETFONT, (WPARAM) font, TRUE);
	data->list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 3, 17, 0, 0, window, NULL, NULL, NULL);

	if(!data->list) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto destroyWindow;
	}

	SendMessageW(data->list, WM_SETFONT, (WPARAM) font, TRUE);
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

	HRSPPACKET packet = {0};
	packet.type = HRSP_REMOTE_SERVER_REQUEST_STREAM_DEVICE;
	HRSPPacketSend(&data->client->hrsp, &packet, NULL);
	*customData = (ULONGLONG) data;
	return window;
destroyWindow:
	DestroyWindow(window);
freeData:
	KHOPAN_DEALLOCATE(data);
	return NULL;
}

static int CALLBACK compareList(PDEVICEENTRY first, PDEVICEENTRY second, LPARAM parameter) {
	if(!first) {
		return second ? -1 : 0;
	} else if(!second) {
		return 1;
	}

	return wcscmp(first->name, second->name);
}

static BOOLEAN packetHandler(const PCLIENT client, const PULONGLONG customData, const PHRSPPACKET packet) {
	PTABSTREAMDATA data;

	if(packet->type != HRSP_REMOTE_CLIENT_RESPONSE_STREAM_DEVICE || packet->size < 1 || !customData || !(data = (PTABSTREAMDATA) *customData)) {
		return FALSE;
	}

	if(((PBYTE) packet->data)[0]) {
		return TRUE;
	}

	size_t index = 1;

	while(index < packet->size) {
		if(index + 5 > packet->size) {
			return FALSE;
		}

		index += ((((PBYTE) packet->data)[index + 1] << 24) | (((PBYTE) packet->data)[index + 2] << 16) | (((PBYTE) packet->data)[index + 3] << 8) | ((PBYTE) packet->data)[index + 4]) + 5;

		if(index + 4 > packet->size) {
			return FALSE;
		}

		index += ((((PBYTE) packet->data)[index] << 24) | (((PBYTE) packet->data)[index + 1] << 16) | (((PBYTE) packet->data)[index + 2] << 8) | ((PBYTE) packet->data)[index + 3]) + 4;

		if(index > packet->size) {
			return FALSE;
		}
	}

	int count = (int) SendMessageW(data->list, LVM_GETITEMCOUNT, 0, 0);
	int i;
	LVITEMW item = {0};
	PDEVICEENTRY entry;
	UINT32 size;

	for(i = count - 1; i >= 0; i--) {
		item.mask = LVIF_PARAM;
		item.iItem = i;

		if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || !(entry = (PDEVICEENTRY) item.lParam)) {
			SendMessageW(data->list, LVM_DELETEITEM, i, 0);
			continue;
		}

		index = 1;

		while(index < packet->size) {
			index += ((((PBYTE) packet->data)[index + 1] << 24) | (((PBYTE) packet->data)[index + 2] << 16) | (((PBYTE) packet->data)[index + 3] << 8) | ((PBYTE) packet->data)[index + 4]) + 9;
			size = (((PBYTE) packet->data)[index - 4] << 24) | (((PBYTE) packet->data)[index - 3] << 16) | (((PBYTE) packet->data)[index - 2] << 8) | ((PBYTE) packet->data)[index - 1];
			index += size;
			if(entry->identifierLength != size) continue;
			if(!memcmp(((PBYTE) packet->data) + index - size, entry->identifier, size)) goto pass;
		}

		KHOPAN_DEALLOCATE(entry->name);
		KHOPAN_DEALLOCATE(entry->identifier);
		KHOPAN_DEALLOCATE(entry);
		SendMessageW(data->list, LVM_DELETEITEM, i, 0);
	pass:
		continue;
	}

	index = 1;

	while(index < packet->size) {
		HRSPREMOTESTREAMDEVICETYPE type = ((PBYTE) packet->data)[index];
		UINT32 nameLength = (((PBYTE) packet->data)[index + 1] << 24) | (((PBYTE) packet->data)[index + 2] << 16) | (((PBYTE) packet->data)[index + 3] << 8) | ((PBYTE) packet->data)[index + 4];
		PBYTE pointerName = ((PBYTE) packet->data) + index + 5;
		index += nameLength + 9;
		size = (((PBYTE) packet->data)[index - 4] << 24) | (((PBYTE) packet->data)[index - 3] << 16) | (((PBYTE) packet->data)[index - 2] << 8) | ((PBYTE) packet->data)[index - 1];
		index += size;

		if(type > HRSP_REMOTE_STREAM_DEVICE_MONITOR) {
			continue;
		}

		count = (int) SendMessageW(data->list, LVM_GETITEMCOUNT, 0, 0);

		for(i = count - 1; i >= 0; i--) {
			item.mask = LVIF_PARAM;
			item.iItem = i;
			if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || !(entry = (PDEVICEENTRY) item.lParam) || entry->identifierLength != size) continue;
			if(!memcmp(((PBYTE) packet->data) + index - size, entry->identifier, size)) goto skip;
		}

		entry = KHOPAN_ALLOCATE(sizeof(DEVICEENTRY));

		if(!entry) {
			continue;
		}

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

		UINT32 x;

		for(x = 0; x < nameLength; x++) {
			((PBYTE) entry->name)[x] = pointerName[x];
		}

		((PBYTE) entry->name)[nameLength] = ((PBYTE) entry->name)[nameLength + 1] = 0;

		for(x = 0; x < size; x++) {
			entry->identifier[x] = ((PBYTE) packet->data)[index - size + x];
		}

		for(i = count - 1; i >= 0; i--) {
			item.mask = LVIF_PARAM;
			item.iItem = i;
			if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || compareList(entry, (PDEVICEENTRY) item.lParam, 0) <= 0) continue;
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

		if(!SendMessageW(data->list, LVM_SETITEM, 0, (LPARAM) &item)) {
			SendMessageW(data->list, LVM_DELETEITEM, i, 0);
		}
	skip:
		continue;
	}

	return TRUE;
}

static void clickHeader(const int index, const HWND listView) {
	if(index < 0) {
		return;
	}

	HWND header = (HWND) SendMessageW(listView, LVM_GETHEADER, 0, 0);

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

		//sort.ascending = item.fmt & HDF_SORTUP;
	setItem:
		SendMessageW(header, HDM_SETITEM, i, (LPARAM) &item);
	}

	//sort.username = index == 0;
	SendMessageW(listView, LVM_SORTITEMS, 0, (LPARAM) compareList);
}

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABSTREAMDATA, data, window, message, wparam, lparam);
	LVHITTESTINFO information = {0};
	RECT bounds;
	LVITEMW item = {0};
	HMENU menu;
	BOOL option;
	HRSPPACKET packet = {0};

	switch(message) {
	case WM_CONTEXTMENU:
		GetCursorPos(&information.pt);
		ScreenToClient(data->list, &information.pt);
		GetClientRect(window, &bounds);

		if(information.pt.x < bounds.left || information.pt.x > bounds.right || information.pt.y < bounds.top || information.pt.y > bounds.bottom) {
			break;
		}

		if(SendMessageW(data->list, LVM_HITTEST, 0, (LPARAM) &information) != -1) {
			item.mask = LVIF_PARAM;
			item.iItem = information.iItem;
			SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item);
		}

		menu = CreatePopupMenu();

		if(!menu) {
			break;
		}

		/*if(item.lParam) {
			AppendMenuW(menu, MF_STRING, IDM_AUDIO_CAPTURE, L"Capture");
			AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
		}*/

		AppendMenuW(menu, MF_STRING, IDM_STREAM_REFRESH, L"Refresh");
		SetForegroundWindow(window);
		option = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_TOPALIGN, LOWORD(lparam), HIWORD(lparam), window, NULL);
		DestroyMenu(menu);

		switch(option) {
		case IDM_STREAM_REFRESH:
			packet.type = HRSP_REMOTE_SERVER_REQUEST_STREAM_DEVICE;
			HRSPPacketSend(&data->client->hrsp, &packet, NULL);
			return 0;
		}

		break;
	case WM_CTLCOLORSTATIC:
		SetDCBrushColor((HDC) wparam, 0xF9F9F9);
		return (LRESULT) GetStockObject(DC_BRUSH);
	case WM_DESTROY:
		for(option = 0; option < SendMessageW(data->list, LVM_GETITEMCOUNT, 0, 0); option++) {
			item.mask = LVIF_PARAM;
			item.iItem = option;
			if(!SendMessageW(data->list, LVM_GETITEM, 0, (LPARAM) &item) || !item.lParam) continue;
			KHOPAN_DEALLOCATE(((PDEVICEENTRY) item.lParam)->name);
			KHOPAN_DEALLOCATE(((PDEVICEENTRY) item.lParam)->identifier);
			KHOPAN_DEALLOCATE((LPVOID) item.lParam);
		}

		KHOPAN_DEALLOCATE(data);
		return 0;
	case WM_NOTIFY:
		if(!lparam) {
			break;
		}

		switch(((LPNMHDR) lparam)->code) {
		case LVN_COLUMNCLICK:
			clickHeader((UINT) ((LPNMLISTVIEW) lparam)->iSubItem, data->list);
			return 0;
		}

		break;
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
	tab->clientInitialize = clientInitialize;
	tab->packetHandler = packetHandler;
	tab->windowClass.lpfnWndProc = procedure;
	tab->windowClass.lpszClassName = CLASS_NAME;
}
