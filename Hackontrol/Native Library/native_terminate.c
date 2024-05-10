#include <khopanjava.h>
#include "native.h"
#include "privilege.h"

jboolean Native_terminate(JNIEnv* environment, jclass nativeLibraryClass, jint identifier) {
	if(!EnablePrivilege(environment, SE_DEBUG_NAME)) {
		return FALSE;
	}

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, identifier);

	if(!process) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"OpenProcess");
		return FALSE;
	}

	if(!TerminateProcess(process, 0)) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"TerminateProcess");
		CloseHandle(process);
		return FALSE;
	}

	CloseHandle(process);
	return TRUE;
}
