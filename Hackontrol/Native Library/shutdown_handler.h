#pragma once

#include <jni.h>

void RegisterShutdownHandler(JNIEnv* environment, JavaVM* virtualMachine);
