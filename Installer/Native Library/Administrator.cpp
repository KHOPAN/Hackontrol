#include <Windows.h>
#include <jni.h>
#include "com_khopan_hackontrol_installer_nativelib_Administrator.h"

JNIEXPORT jboolean JNICALL Java_com_khopan_hackontrol_installer_nativelib_Administrator_hasAdministratorPrivileges(JNIEnv* environment, jclass administratorClass) {
    jboolean hasAdministratorPrivileges = FALSE;
    HANDLE process = GetCurrentProcess();
    HANDLE token = NULL;

    if(OpenProcessToken(process, TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);

        if(GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
            hasAdministratorPrivileges = elevation.TokenIsElevated;
        }
    }

    if(token) {
        CloseHandle(token);
    }

    return hasAdministratorPrivileges;
}
