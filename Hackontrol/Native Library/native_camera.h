#pragma once

#include <jni.h>

jobjectArray NativeLibrary_cameraList(JNIEnv* environment, jclass nativeLibraryClass);
jbyteArray NativeLibrary_capture(JNIEnv* environment, jclass nativeLibraryClass, jobject cameraDeviceInstance, jboolean useMjpg);
