#pragma once

#include <WS2tcpip.h>
#include <jni.h>

typedef struct {
	JavaVM* virtualMachine;
	SOCKET clientSocket;
} STREAMPARAMETER, *PSTREAMPARAMETER;

DWORD WINAPI StreamThread(_In_ PSTREAMPARAMETER parameter);
