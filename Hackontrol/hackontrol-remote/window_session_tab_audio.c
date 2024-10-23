#include "window_session_tabs.h"
#include <CommCtrl.h>

extern HFONT font;

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG data, const HWND parent) {
	HWND window = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP | WS_CHILD | WS_VSCROLL, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		return NULL;
	}

	SendMessageW(window, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	LVCOLUMNW column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	column.cx = (int) (((double) GetSystemMetrics(SM_CXSCREEN)) * 0.13250366);
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"Name";

	if(SendMessageW(window, LVM_INSERTCOLUMN, 0, (LPARAM) &column) == -1) {
		KHOPANLASTERRORMESSAGE_WIN32(L"ListView_InsertColumn");
		return NULL;
	}

	SendMessageW(window, WM_SETFONT, (WPARAM) font, TRUE);
	return window;
}

void __stdcall WindowSessionTabAudio(const PTABINITIALIZER tab) {
	tab->name = L"Audio";
	tab->clientInitialize = clientInitialize;
}
