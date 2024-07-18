#include "packet.h"
#include <khopandatastream.h>

BOOL SendPacket(const SOCKET socket, const PACKET* packet) {
	if(!socket || !packet) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	char headerBuffer[5];
	headerBuffer[0] = (packet->size >> 24) & 0xFF;
	headerBuffer[1] = (packet->size >> 16) & 0xFF;
	headerBuffer[2] = (packet->size >> 8) & 0xFF;
	headerBuffer[3] = packet->size & 0xFF;
	headerBuffer[4] = packet->packetType;
	DataStream stream = {0};

	if(!KHDataStreamAdd(&stream, headerBuffer, sizeof(headerBuffer))) {
		return FALSE;
	}

	if(!KHDataStreamAdd(&stream, packet->data, packet->size)) {
		return FALSE;
	}

	char* remainingBuffer = stream.data;
	size_t remainingSize = stream.size;

	while(remainingSize) {
		int sendSize;

		if(remainingSize > INT_MAX) {
			sendSize = INT_MAX;
			remainingSize -= INT_MAX;
		} else {
			sendSize = (int) remainingSize;
			remainingSize = 0;
		}

		if(send(socket, remainingBuffer, sendSize, 0) == SOCKET_ERROR) {
			SetLastError(WSAGetLastError());
			KHDataStreamFree(&stream);
			return FALSE;
		}

		remainingBuffer += sendSize;
	}

	KHDataStreamFree(&stream);
	return TRUE;
}
