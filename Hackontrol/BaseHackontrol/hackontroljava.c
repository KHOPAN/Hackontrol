#include "hackontroljava.h"
#include <khopanstring.h>
#include <khopanjava.h>
#include <khopandatastream.h>

BOOL SendPacket(const SOCKET socket, const PACKET* packet) {
	if(!socket || !packet) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	char header[5];
	header[0] = (packet->size >> 24) & 0xFF;
	header[1] = (packet->size >> 16) & 0xFF;
	header[2] = (packet->size >> 8) & 0xFF;
	header[3] = packet->size & 0xFF;
	header[4] = packet->packetType;
	DataStream stream = {0};

	if(!KHDataStreamAdd(&stream, header, sizeof(header))) {
		return FALSE;
	}

	if(packet->size > 0 && !KHDataStreamAdd(&stream, packet->data, packet->size)) {
		return FALSE;
	}

	int status = send(socket, stream.data, (int) stream.size, 0);
	KHDataStreamFree(&stream);

	if(status == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL ReceivePacket(const SOCKET socket, PACKET* packet) {
	if(!socket || !packet) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	char header[5];

	if(recv(socket, header, sizeof(header), MSG_WAITALL) == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		return FALSE;
	}

	long size = ((header[0] & 0xFF) << 24) | ((header[1] & 0xFF) << 16) | ((header[2] & 0xFF) << 8) | (header[3] & 0xFF);

	if(size < 1) {
		packet->size = 0;
		packet->packetType = header[4];
		packet->data = NULL;
		return TRUE;
	}

	void* buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		return FALSE;
	}

	if(recv(socket, buffer, size, MSG_WAITALL) == SOCKET_ERROR) {
		SetLastError(WSAGetLastError());
		LocalFree(buffer);
		return FALSE;
	}

	packet->size = size;
	packet->packetType = header[4];
	packet->data = buffer;
	return TRUE;
}

static BOOL isSpaceW(WCHAR character) {
	return character == L' ' || character == L'\t' || character == L'\r' || character == L'\n';
}

static LPWSTR trimWhitespaceW(LPWSTR text) {
	LPWSTR end;
	while(isSpaceW(*text)) text++;

	if(*text == 0) {
		return text;
	}

	end = text + wcslen(text) - 1;
	while(end > text && isSpaceW(*end)) end--;
	end[1] = 0;
	return text;
}

void HackontrolThrowWin32Error(JNIEnv* const environment, const LPWSTR functionName) {
	DWORD errorCode = GetLastError();
	LPWSTR messageBuffer = NULL;

	if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL)) {
		KHJavaThrowInternalErrorW(environment, L"FormatMessageW() failed to format the error message");
		return;
	}

	LPWSTR message = KHFormatMessageW(L"%ws() %ws (Error code: %u)", functionName, trimWhitespaceW(messageBuffer), errorCode);
	LocalFree(messageBuffer);

	if(!message) {
		KHJavaThrowInternalErrorW(environment, L"KHFormatMessageW() failed to format the error message");
		return;
	}

	KHJavaThrowW(environment, "com/khopan/hackontrol/Win32Error", message);
	LocalFree(message);
}
