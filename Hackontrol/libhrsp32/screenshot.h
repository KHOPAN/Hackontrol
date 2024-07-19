#pragma once

#include <WS2tcpip.h>
#include <jni.h>

BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket, int width, int height);
