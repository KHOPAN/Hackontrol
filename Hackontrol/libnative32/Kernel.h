#pragma once

#include <jni.h>

void KernelRegisterNatives(JNIEnv* const environment);
void Kernel_sleep(JNIEnv* const environment, jclass const class);
void Kernel_hibernate(JNIEnv* const environment, jclass const class);
void Kernel_restart(JNIEnv* const environment, jclass const class);
void Kernel_shutdown(JNIEnv* const environment, jclass const class);
void Kernel_setProcessCritical(JNIEnv* const environment, const jclass class, const jboolean critical);
void Kernel_setFreeze(JNIEnv* const environment, const jclass class, const jboolean freeze, const jbyteArray image);
jobjectArray Kernel_getProcessList(JNIEnv* const environment, const jclass class);
jint Kernel_getCurrentProcessIdentifier(JNIEnv* const environment, const jclass class);
void Kernel_terminateProcess(JNIEnv* const environment, const jclass class, const jint processIdentifier);
void Kernel_initiateRestart(JNIEnv* const environment, const jclass class, const jboolean update);
void Kernel_shellExecute(JNIEnv* const environment, const jclass class, const jstring command, const jobject callback);
void Kernel_connectHRSPServer(JNIEnv* const environment, const jclass class, const jstring host, const jint port, const jobject callback);
