#pragma once

#include <Windows.h>
#include <jni.h>

void HackontrolThrowWin32Error(JNIEnv* const environment, const LPWSTR functionName);
