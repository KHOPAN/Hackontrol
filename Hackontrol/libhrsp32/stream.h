#pragma once

#include <WS2tcpip.h>
#include <jni.h>

DWORD WINAPI StreamThread(_In_ LPVOID parameter);
BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket, int width, int height, BYTE* screenshotBuffer, BYTE* qoiBuffer, BYTE* previousBuffer);
