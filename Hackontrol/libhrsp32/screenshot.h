#pragma once

#include <WS2tcpip.h>
#include <jni.h>
#include <lodepng.h>

BOOL TakeScreenshot(JNIEnv* const environment, const SOCKET clientSocket, LodePNGState* state);
