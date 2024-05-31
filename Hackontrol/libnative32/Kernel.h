#pragma once

#include <jni.h>

void KernelRegisterNatives(JNIEnv* const environment);
void Kernel_sleep(JNIEnv* const environment, const jclass class);
void Kernel_hibernate(JNIEnv* const environment, const jclass class);
void Kernel_shutdown(JNIEnv* const environment, const jclass class);
void Kernel_restart(JNIEnv* const environment, const jclass class);
void Kernel_setProcessCritical(JNIEnv* const environment, const jclass class, const jboolean critical);
