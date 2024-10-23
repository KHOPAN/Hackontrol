#include "window_session_tabs.h"
#include <CommCtrl.h>

static HWND __stdcall clientInitialize(const PCLIENT client, const PULONGLONG data, const HWND parent) {
	HWND window = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD, 0, 0, 0, 0, parent, NULL, NULL, data);

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		return NULL;
	}

	return window;
}

void __stdcall WindowSessionTabAudio(const PTABINITIALIZER tab) {
	tab->name = L"Audio";
	tab->clientInitialize = clientInitialize;
}
