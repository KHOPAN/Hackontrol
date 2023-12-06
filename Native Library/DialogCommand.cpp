#include <Windows.h>
#include <jni.h>
#include "com_khopan_hackontrol_command_DialogCommand.h"

JNIEXPORT void JNICALL Java_com_khopan_hackontrol_command_DialogCommand_messageBox(JNIEnv* environment, jclass dialogCommand, jstring message, jstring title, jint type) {
	const jchar* messageText = environment->GetStringCritical(message, NULL);
	jchar* messageNonConst = const_cast<jchar*>(messageText);
	const jchar* titleText = title ? environment->GetStringCritical(title, NULL) : NULL;
	MessageBoxW(NULL, reinterpret_cast<LPCWSTR>(messageNonConst), title ? reinterpret_cast<LPCWSTR>(const_cast<jchar*>(titleText)) : NULL, type);
	environment->ReleaseStringCritical(message, messageText);
	if(title) environment->ReleaseStringCritical(title, titleText);
}
