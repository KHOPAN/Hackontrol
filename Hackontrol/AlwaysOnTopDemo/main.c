#include <stdio.h>
#include <khopanwin32.h>
#include <TlHelp32.h>

#define CLASS_NAME L"AlwaysOnTopWindowClass"

static BOOL HasUIAccess(LPBOOL hasUIAccess);
static BOOL CreateUIAccessToken(LPHANDLE handle);
static BOOL DuplicateWinlogonToken(DWORD sessionIdentifier, DWORD desiredAccess, LPHANDLE token);
static BOOL CreateAlwaysOnTopWindow(HINSTANCE instance);
static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
static void PaintWindow(HWND window);

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR commandLine, _In_ int show) {
	BOOL hasUIAccess;

	if(!HasUIAccess(&hasUIAccess)) {
		return 1;
	}

	if(hasUIAccess) {
		return CreateAlwaysOnTopWindow(instance);
	}

	HANDLE accessToken;

	if(!CreateUIAccessToken(&accessToken)) {
		return 1;
	}

	STARTUPINFOW startupInformation;
	GetStartupInfoW(&startupInformation);
	PROCESS_INFORMATION processInformation;

	if(!CreateProcessAsUserW(accessToken, NULL, GetCommandLineW(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInformation, &processInformation)) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessAsUserW");
		CloseHandle(accessToken);
		return 1;
	}

	BOOL returnValue = TRUE;

	if(WaitForSingleObject(processInformation.hProcess, INFINITE) == WAIT_FAILED) {
		KHWin32DialogErrorW(GetLastError(), L"CreateProcessAsUserW");
		goto closeProcess;
	}

	returnValue = FALSE;
closeProcess:
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	CloseHandle(accessToken);
	return returnValue;
}

static BOOL HasUIAccess(LPBOOL hasUIAccess) {
	HANDLE token;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		KHWin32DialogErrorW(GetLastError(), L"OpenProcessToken");
		return FALSE;
	}

	BOOL returnValue = FALSE;
	DWORD access;
	DWORD returnLength;

	if(!GetTokenInformation(token, TokenUIAccess, &access, sizeof(access), &returnLength)) {
		KHWin32DialogErrorW(GetLastError(), L"GetTokenInformation");
		goto closeToken;
	}

	*hasUIAccess = (BOOL) access;
	returnValue = TRUE;
closeToken:
	CloseHandle(token);
	return returnValue;
}

static BOOL CreateUIAccessToken(LPHANDLE token) {
	HANDLE currentToken;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &currentToken)) {
		KHWin32DialogErrorW(GetLastError(), L"OpenProcessToken");
		return FALSE;
	}

	BOOL returnValue = FALSE;
	DWORD sessionIdentifier;
	DWORD returnLength;

	if(!GetTokenInformation(currentToken, TokenSessionId, &sessionIdentifier, sizeof(sessionIdentifier), &returnLength)) {
		KHWin32DialogErrorW(GetLastError(), L"GetTokenInformation");
		goto closeCurrentToken;
	}

	HANDLE systemToken;

	if(!DuplicateWinlogonToken(sessionIdentifier, TOKEN_IMPERSONATE, &systemToken)) {
		goto closeCurrentToken;
	}

	if(!SetThreadToken(NULL, systemToken)) {
		KHWin32DialogErrorW(GetLastError(), L"SetThreadToken");
		goto closeSystemToken;
	}

	if(!DuplicateTokenEx(currentToken, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT, NULL, SecurityAnonymous, TokenPrimary, token)) {
		KHWin32DialogErrorW(GetLastError(), L"DuplicateTokenEx");
		goto revertToSelf;
	}

	BOOL access = TRUE;

	if(!SetTokenInformation(*token, TokenUIAccess, &access, sizeof(access))) {
		KHWin32DialogErrorW(GetLastError(), L"SetTokenInformation");
		CloseHandle(*token);
		goto revertToSelf;
	}

	returnValue = TRUE;
revertToSelf:
	RevertToSelf();
closeSystemToken:
	CloseHandle(systemToken);
closeCurrentToken:
	CloseHandle(currentToken);
	return returnValue;
}

static BOOL DuplicateWinlogonToken(DWORD sessionIdentifier, DWORD desiredAccess, LPHANDLE token) {
	PRIVILEGE_SET privileges;
	privileges.PrivilegeCount = 1;
	privileges.Control = PRIVILEGE_SET_ALL_NECESSARY;

	if(!LookupPrivilegeValueW(NULL, SE_TCB_NAME, &privileges.Privilege[0].Luid)) {
		KHWin32DialogErrorW(GetLastError(), L"LookupPrivilegeValueW");
		return FALSE;
	}

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(snapshot == INVALID_HANDLE_VALUE) {
		KHWin32DialogErrorW(GetLastError(), L"CreateToolhelp32Snapshot");
		return FALSE;
	}

	BOOL returnValue = FALSE;
	PROCESSENTRY32W entry = {0};
	entry.dwSize = sizeof(entry);

	for(BOOL result = Process32FirstW(snapshot, &entry); result; result = Process32NextW(snapshot, &entry)) {
		if(wcscmp(entry.szExeFile, L"winlogon.exe")) {
			continue;
		}

		HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);

		if(!process) {
			KHWin32DialogErrorW(GetLastError(), L"OpenProcess");
			goto closeSnapshot;
		}

		HANDLE winlogonToken;

		if(!OpenProcessToken(process, TOKEN_QUERY | TOKEN_DUPLICATE, &winlogonToken)) {
			KHWin32DialogErrorW(GetLastError(), L"OpenProcessToken");
			CloseHandle(process);
			goto closeSnapshot;
		}

		CloseHandle(process);
		BOOL checkResult;

		if(!PrivilegeCheck(winlogonToken, &privileges, &checkResult)) {
			KHWin32DialogErrorW(GetLastError(), L"PrivilegeCheck");
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		if(!checkResult) {
			printf("Check failed\n");
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		DWORD resultIdentifier;
		DWORD returnLength;

		if(!GetTokenInformation(winlogonToken, TokenSessionId, &resultIdentifier, sizeof(resultIdentifier), &returnLength)) {
			KHWin32DialogErrorW(GetLastError(), L"GetTokenInformation");
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		if(resultIdentifier != sessionIdentifier) {
			printf("Session identifier mismatch\n");
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		if(!DuplicateTokenEx(winlogonToken, desiredAccess, NULL, SecurityImpersonation, TokenImpersonation, token)) {
			KHWin32DialogErrorW(GetLastError(), L"DuplicateTokenEx");
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		CloseHandle(winlogonToken);
		returnValue = TRUE;
		goto closeSnapshot;
	}
closeSnapshot:
	CloseHandle(snapshot);
	return returnValue;
}

static BOOL CreateAlwaysOnTopWindow(HINSTANCE instance) {
	WNDCLASSW windowClass = {0};
	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		KHWin32DialogErrorW(GetLastError(), L"RegisterClassW");
		return 1;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int windowWidth = (int) (screenWidth * 0.439238653);
	int windowHeight = (int) (screenHeight * 0.520833333);
	int windowX = (screenWidth - windowWidth) / 2;
	int windowY = (screenHeight - windowHeight) / 2;
	HWND window = CreateWindowExW(0L, CLASS_NAME, L"Always On Top", WS_OVERLAPPEDWINDOW, windowX, windowY, windowWidth, windowHeight, NULL, NULL, instance, NULL);

	if(!window) {
		KHWin32DialogErrorW(GetLastError(), L"CreateWindowExW");
		return 1;
	}

	if(!SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) {
		KHWin32DialogErrorW(GetLastError(), L"SetWindowPos");
		return 1;
	}

	ShowWindow(window, SW_NORMAL);
	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	if(!UnregisterClassW(CLASS_NAME, instance)) {
		KHWin32DialogErrorW(GetLastError(), L"UnregisterClassW");
		return 1;
	}

	return 0;
}

static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		PaintWindow(window);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

static void PaintWindow(HWND window) {
	PAINTSTRUCT paintStruct;
	HDC context = BeginPaint(window, &paintStruct);
	HBRUSH brush = GetStockObject(DC_BRUSH);
	SetDCBrushColor(context, 0xF0F0F0);
	RECT rectangle;
	GetClientRect(window, &rectangle);
	FillRect(context, &rectangle, brush);
	EndPaint(window, &paintStruct);
}
