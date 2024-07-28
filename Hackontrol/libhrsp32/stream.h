#pragma once

#include <WinSock2.h>
#include <jni.h>

typedef struct {
	JavaVM* virtualMachine;
	SOCKET clientSocket;
} STREAMPARAMETER;

DWORD WINAPI ScreenStreamThread(_In_ STREAMPARAMETER* parameter);
void SetStreamParameter(unsigned char streamSettings);
