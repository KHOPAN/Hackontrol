#include <jni.h>
#include "CommandSource.h"
#include "ErrorUtils.h"
#include "StringUtils.h"

void CommandSource_message(JNIEnv* environment, jobject source, jstring message) {
	jclass commandSourceClass = environment->GetObjectClass(source);
	std::string className;
	if(StringUtils_GetClassName(environment, commandSourceClass, className)) return;
	jmethodID messageMethod = environment->GetMethodID(commandSourceClass, "message", "(Ljava/lang/String;)V");

	if(!messageMethod) {
		ErrorUtils_ThrowException(environment, "java/lang/NoSuchMethodException", "Method 'message(Ljava/lang/String;)V' not found in class '" + className + '\'');
		return;
	}

	environment->CallVoidMethod(source, messageMethod, message);
}
