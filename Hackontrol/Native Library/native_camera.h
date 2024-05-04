#pragma once

#include <jni.h>

jobjectArray NativeLibrary_cameraList(JNIEnv* environment, jclass nativeLibraryClass);
jobject NativeLibrary_capture(JNIEnv* environment, jclass nativeLibraryClass, jobject cameraDeviceInstance);
