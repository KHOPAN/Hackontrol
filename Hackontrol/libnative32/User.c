#include "User.h"

static JNINativeMethod NativeMethods[] = {
	{"getMasterVolume", "()F",                                      (void*) &User_getMasterVolume},
	{"isMute",          "()Z",                                      (void*) &User_isMute},
	{"setMasterVolume", "(F)V",                                     (void*) &User_setMasterVolume},
	{"setMute",         "(Z)V",                                     (void*) &User_setMute},
	{"showMessageBox",  "(Ljava/lang/String;Ljava/lang/String;I)I", (void*) &User_showMessageBox},
	{"screenshot",      "()[B",                                     (void*) &User_screenshot}
};

void UserRegisterNatives(JNIEnv* const environment) {
	jclass class = (*environment)->FindClass(environment, "com/khopan/hackontrol/library/User");

	if(!class) {
		return;
	}

	(*environment)->RegisterNatives(environment, class, NativeMethods, sizeof(NativeMethods) / sizeof(NativeMethods[0]));
}
