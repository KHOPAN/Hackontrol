#include "window_session_tabs.h"
#include <CommCtrl.h>
#include <hrsp_remote.h>

#define CLASS_NAME L"HackontrolRemoteSessionTabAudio"

extern HFONT font;

typedef struct {
	HWND border;
	HWND list;
} TABAUDIODATA, *PTABAUDIODATA;

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG customData, const HWND parent) {
	PTABAUDIODATA data = KHOPAN_ALLOCATE(sizeof(TABAUDIODATA));

	if(KHOPAN_ALLOCATE_FAILED(data)) {
		KHOPANERRORCONSOLE_WIN32(KHOPAN_ALLOCATE_ERROR, KHOPAN_ALLOCATE_FUNCTION);
		return NULL;
	}

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
	SendMessageW(data->list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
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

	LVITEMW listItem = {0};
	listItem.mask = LVIF_TEXT;
	listItem.iSubItem = 0;
	listItem.pszText = L"Third";
	SendMessageW(data->list, LVM_INSERTITEM, 0, (LPARAM) &listItem);
	listItem.pszText = L"Second";
	SendMessageW(data->list, LVM_INSERTITEM, 0, (LPARAM) &listItem);
	listItem.pszText = L"First";
	SendMessageW(data->list, LVM_INSERTITEM, 0, (LPARAM) &listItem);
	HRSPSendTypePacket(client->socket, &client->hrsp, HRSP_REMOTE_SERVER_AUDIO_QUERY_DEVICE, NULL);
	return window;
}

static BOOL __stdcall packetHandler(const PCLIENT client, const PULONGLONG data, const PHRSPPACKET packet) {
	if(packet->type != HRSP_REMOTE_CLIENT_AUDIO_DEVICE_RESULT || packet->size < 4) {
		return FALSE;
	}

	UINT count = (packet->data[0] << 24) | (packet->data[1] << 16) | (packet->data[2] << 8) | packet->data[3];
	LOG("Count: %u\n", count);
	return TRUE;
}

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	USERDATA(PTABAUDIODATA, data, window, message, wparam, lparam);
	RECT bounds;

	switch(message) {
	case WM_DESTROY:
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
