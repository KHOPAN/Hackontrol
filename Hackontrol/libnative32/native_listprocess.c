#include <khopanjava.h>
#include <TlHelp32.h>
#include "native.h"

jobjectArray NativeLibrary_listProcess(JNIEnv* environment, jclass nativeLibraryClass) {
	jclass arrayListClass = (*environment)->FindClass(environment, "java/util/ArrayList");

	if(!arrayListClass) {
		return NULL;
	}

	jmethodID arrayListConstructor = (*environment)->GetMethodID(environment, arrayListClass, "<init>", "()V");

	if(!arrayListConstructor) {
		return NULL;
	}

	jmethodID addMethod = (*environment)->GetMethodID(environment, arrayListClass, "add", "(Ljava/lang/Object;)Z");

	if(!addMethod) {
		return NULL;
	}

	jmethodID toArrayMethod = (*environment)->GetMethodID(environment, arrayListClass, "toArray", "()[Ljava/lang/Object;");

	if(!toArrayMethod) {
		return NULL;
	}

	jclass processEntryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/ProcessEntry");

	if(!processEntryClass) {
		return NULL;
	}

	jmethodID processEntryConstructor = (*environment)->GetMethodID(environment, processEntryClass, "<init>", "(IIIILjava/lang/String;)V");

	if(!processEntryConstructor) {
		return NULL;
	}

	jobject arrayListInstance = (*environment)->NewObject(environment, arrayListClass, arrayListConstructor);
	
	if(!arrayListInstance) {
		return NULL;
	}

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(snapshot == INVALID_HANDLE_VALUE) {
		KHJavaWin32ErrorW(environment, GetLastError(), L"CreateToolhelp32Snapshot");
		return NULL;
	}

	PROCESSENTRY32W processEntry = {0};
	processEntry.dwSize = sizeof(PROCESSENTRY32W);
	BOOL first = TRUE;

	while(TRUE) {
		BOOL result = first ? Process32FirstW(snapshot, &processEntry) : Process32NextW(snapshot, &processEntry);
		first = FALSE;

		if(!result) {
			break;
		}

		jint processIdentifier = (jint) processEntry.th32ProcessID;
		jint executionThreads = (jint) processEntry.cntThreads;
		jint parentProcessIdentifier = (jint) processEntry.th32ParentProcessID;
		jint threadBasePriority = processEntry.pcPriClassBase;
		jstring executableFile = KHJavaFromNativeStringW(environment, processEntry.szExeFile);
		jobject processEntryInstance = (*environment)->NewObject(environment, processEntryClass, processEntryConstructor, processIdentifier, executionThreads, parentProcessIdentifier, threadBasePriority, executableFile);

		if(!processEntryInstance) {
			continue;
		}

		(*environment)->CallBooleanMethod(environment, arrayListInstance, addMethod, processEntryInstance);
	}

	return (*environment)->CallObjectMethod(environment, arrayListInstance, toArrayMethod);
}

jint Native_currentIdentifier(JNIEnv* environment, jclass nativeLibraryClass) {
	return (jint) GetCurrentProcessId();
}
