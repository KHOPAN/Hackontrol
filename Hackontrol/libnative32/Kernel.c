#include "Kernel.h"

static JNINativeMethod NativeMethods[] = {
	{"sleep",              "()V",  (void*) &Kernel_sleep},
	{"hibernate",          "()V",  (void*) &Kernel_hibernate},
	{"shutdown",           "()V",  (void*) &Kernel_shutdown},
	{"restart",            "()V",  (void*) &Kernel_restart},
	{"setProcessCritical", "(Z)V", (void*) &Kernel_setProcessCritical}
};

void KernelRegisterNatives(JNIEnv* const environment) {
	jclass class = (*environment)->FindClass(environment, "com/khopan/hackontrol/nativelibrary/Kernel");

	if(!class) {
		return;
	}

	(*environment)->RegisterNatives(environment, class, NativeMethods, sizeof(NativeMethods) / sizeof(NativeMethods[0]));
}
