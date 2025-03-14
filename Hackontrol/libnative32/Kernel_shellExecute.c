#include <khopanwin32.h>
#include <khopanstring.h>
#include <khopanjava.h>
#include "Kernel.h"
#include "exception.h"

#define BUFFER_SIZE 4096

void Kernel_shellExecute(JNIEnv* const environment, const jclass class, const jstring command, const jobject callback) {
	if(!command) {
		return;
	}

	if(!callback) {
		KHJavaThrowIllegalArgumentExceptionW(environment, L"Callback cannot be null");
		return;
	}

	jclass consumerClass = (*environment)->FindClass(environment, "java/util/function/Consumer");

	if(!consumerClass) {
		return;
	}

	jmethodID acceptMethod = (*environment)->GetMethodID(environment, consumerClass, "accept", "(Ljava/lang/Object;)V");

	if(!acceptMethod) {
		return;
	}

	HANDLE pipeRead;
	HANDLE pipeWrite;
	SECURITY_ATTRIBUTES attributes = {0};
	attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	attributes.bInheritHandle = TRUE;

	if(!CreatePipe(&pipeRead, &pipeWrite, &attributes, 0)) {
		HackontrolThrowWin32Error(environment, L"CreatePipe");
		return;
	}

	if(!SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0)) {
		HackontrolThrowWin32Error(environment, L"SetHandleInformation");
		goto closePipe;
	}

	STARTUPINFOW startupInformation = {0};
	startupInformation.cb = sizeof(STARTUPINFOW);
	startupInformation.dwFlags = STARTF_USESTDHANDLES;
	startupInformation.hStdOutput = pipeWrite;
	startupInformation.hStdError = pipeWrite;
	PROCESS_INFORMATION processInformation;
	const jchar* commandLine = (*environment)->GetStringChars(environment, command, NULL);
	BOOL result = CreateProcessW(NULL, (jchar*) commandLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startupInformation, &processInformation);
	(*environment)->ReleaseStringChars(environment, command, commandLine);

	if(!result) {
		HackontrolThrowWin32Error(environment, L"CreateProcessW");
		goto closePipe;
	}

	BYTE buffer[BUFFER_SIZE];
	DWORD bytesRead = 0;

	while(TRUE) {
		CloseHandle(pipeWrite);

		if(!ReadFile(pipeRead, buffer, BUFFER_SIZE, &bytesRead, NULL) || bytesRead == 0) {
			break;
		}

		buffer[bytesRead] = 0;
		jstring string = (*environment)->NewStringUTF(environment, (char*) buffer);
		(*environment)->CallVoidMethod(environment, callback, acceptMethod, string);
	}

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
closePipe:
	CloseHandle(pipeRead);
	CloseHandle(pipeWrite);
}
