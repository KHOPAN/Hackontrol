#include <string>
#include <Windows.h>
#include <jni.h>
#include "StringUtils.h"
#include "ErrorUtils.h"
#include "CommandSource.h"

void StringUtils_FormatAndSendError(JNIEnv* environment, jobject source, unsigned long errorCode) {
	wchar_t* messageBuffer = NULL;
	unsigned long response = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, NULL, reinterpret_cast<LPWSTR>(&messageBuffer), NULL, NULL);

	if(response == NULL) {
		std::string message = "Error code: ";
		message += std::to_string(errorCode);
		jstring string = environment->NewStringUTF(message.c_str());
		CommandSource_message(environment, source, string);
		return;
	}

	jchar* text = reinterpret_cast<jchar*>(messageBuffer);
	jstring string = environment->NewString(text, response);
	CommandSource_message(environment, source, string);
}

bool StringUtils_GetClassName(JNIEnv* environment, jclass inputClass, std::string& result) {
	jclass classClass = environment->FindClass("java/lang/Class");

	if(!classClass) {
		ErrorUtils_ThrowException(environment, "java/lang/ClassNotFoundException", "Class 'java.lang.Class' not found");
		return true;
	}

	jmethodID getNameMethod = environment->GetMethodID(classClass, "getName", "()Ljava/lang/String;");

	if(!getNameMethod) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Method 'getName()Ljava/lang/String;' not found in class 'java.lang.Class'");
		return true;
	}

	jobject nameObject = environment->CallObjectMethod(inputClass, getNameMethod);
	jstring name = static_cast<jstring>(nameObject);
	const char* text = environment->GetStringUTFChars(name, NULL);
	std::string string(text);
	result = string;
	return false;
}
