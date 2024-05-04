#pragma once

#include <jni.h>

jint NativeLibrary_dialog(JNIEnv* environment, jclass nativeLibraryClass, jstring title, jstring content, jint flags);
