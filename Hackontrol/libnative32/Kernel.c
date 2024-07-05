#include "Kernel.h"

static JNINativeMethod NativeMethods[] = {
	{"sleep",                       "()V",                                     (void*) &Kernel_sleep},
	{"hibernate",                   "()V",                                     (void*) &Kernel_hibernate},
	{"restart",                     "()V",                                     (void*) &Kernel_restart},
	{"shutdown",                    "()V",                                     (void*) &Kernel_shutdown},
	{"setProcessCritical",          "(Z)V",                                    (void*) &Kernel_setProcessCritical},
	{"setFreeze",                   "(Z)V",                                    (void*) &Kernel_setFreeze},
	{"getProcessList",              "()[Lcom/khopan/hackontrol/ProcessEntry;", (void*) &Kernel_getProcessList},
	{"getCurrentProcessIdentifier", "()I",                                     (void*) &Kernel_getCurrentProcessIdentifier},
	{"terminateProcess",            "(I)V",                                    (void*) &Kernel_terminateProcess}
};

void KernelRegisterNatives(JNIEnv* const environment) {
	jclass class = (*environment)->FindClass(environment, "com/khopan/hackontrol/library/Kernel");

	if(!class) {
		return;
	}

	(*environment)->RegisterNatives(environment, class, NativeMethods, sizeof(NativeMethods) / sizeof(NativeMethods[0]));
}
