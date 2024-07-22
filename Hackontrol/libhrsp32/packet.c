#include "packet.h"
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
