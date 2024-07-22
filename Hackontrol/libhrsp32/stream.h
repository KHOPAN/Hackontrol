#pragma once

#include <WS2tcpip.h>
#include <jni.h>

typedef struct {
	JavaVM* virtualMachine;
	SOCKET clientSocket;
} STREAMPARAMETER, *PSTREAMPARAMETER;

DWORD WINAPI ScreenStreamThread(_In_ PSTREAMPARAMETER parameter);
DWORD WINAPI InputStreamThread(_In_ PSTREAMPARAMETER parameter);
