#include <Windows.h>
#include <jni.h>
#include "com_khopan_hackontrol_command_PowerCommand.h"

JNIEXPORT void JNICALL Java_com_khopan_hackontrol_command_PowerCommand_exitWindows(JNIEnv* environment, jclass powerCommand, jint flags) {
	ExitWindowsEx(flags, 0x50008);
}
