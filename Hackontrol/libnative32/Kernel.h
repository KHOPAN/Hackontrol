#pragma once

#include <jni.h>

void KernelRegisterNatives(JNIEnv* const environment);
void Kernel_sleep(JNIEnv* const environment, jclass const class);
void Kernel_hibernate(JNIEnv* const environment, jclass const class);
void Kernel_shutdown(JNIEnv* const environment, jclass const class);
void Kernel_restart(JNIEnv* const environment, jclass const class);
void Kernel_setProcessCritical(JNIEnv* const environment, const jclass class, const jboolean critical);
void Kernel_setFreeze(JNIEnv* const environment, const jclass class, const jboolean freeze);
jobjectArray Kernel_getProcessList(JNIEnv* const environment, const jclass class);
