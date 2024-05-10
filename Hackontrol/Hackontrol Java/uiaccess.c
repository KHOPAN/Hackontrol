#include <khopanwin32.h>
#include "uiaccess.h"
#include <TlHelp32.h>

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
			MessageBoxW(NULL, L"Privilege check has failed", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
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
			MessageBoxW(NULL, L"Session identifier mismatch", L"Error", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		if(!DuplicateTokenEx(winlogonToken, desiredAccess, NULL, SecurityImpersonation, TokenImpersonation, token)) {
			KHWin32DialogErrorW(GetLastError(), L"DuplicateTokenEx");
			CloseHandle(winlogonToken);
			goto closeSnapshot;
		}

		CloseHandle(winlogonToken);
		goto success;
	}

success:
	returnValue = TRUE;
closeSnapshot:
	CloseHandle(snapshot);
	return returnValue;
}

BOOL CreateUIAccessToken(LPHANDLE token) {
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

	if(!systemToken) {
		returnValue = TRUE;
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
