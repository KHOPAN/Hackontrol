#include "Information.h"

static JNINativeMethod NativeMethods[] = {
	{"getUserName",       "()Ljava/lang/String;", (void*) &Information_getUserName},
	{"getMachineName",    "()Ljava/lang/String;", (void*) &Information_getMachineName},
	{"isEnabledUIAccess", "()Z",                  (void*) &Information_isEnabledUIAccess}
};

void InformationRegisterNatives(JNIEnv* const environment) {
	jclass class = (*environment)->FindClass(environment, "com/khopan/hackontrol/nativelibrary/Information");

	if(!class) {
		return;
	}

	(*environment)->RegisterNatives(environment, class, NativeMethods, sizeof(NativeMethods) / sizeof(NativeMethods[0]));
}
