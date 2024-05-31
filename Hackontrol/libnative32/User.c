#include "User.h"

static JNINativeMethod NativeMethods[] = {
	{"showMessageBox", "(Ljava/lang/String;Ljava/lang/String;I)I", (void*) &User_showMessageBox},
};

void UserRegisterNatives(JNIEnv* const environment) {
	jclass class = (*environment)->FindClass(environment, "com/khopan/hackontrol/nativelibrary/User");

	if(!class) {
		return;
	}

	(*environment)->RegisterNatives(environment, class, NativeMethods, sizeof(NativeMethods) / sizeof(NativeMethods[0]));
}
