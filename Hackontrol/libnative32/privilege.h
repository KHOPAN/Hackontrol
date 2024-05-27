#pragma once

#include <Windows.h>
#include <jni.h>

int EnablePrivilege(JNIEnv* environment, LPCWSTR privilege);
