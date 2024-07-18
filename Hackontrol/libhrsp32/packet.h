#pragma once

#include <WinSock2.h>

#define PACKET_TYPE_STREAM_FRAME 1

typedef struct {
	long size;
	char packetType;
	void* data;
} PACKET;

BOOL SendPacket(const SOCKET socket, const PACKET* packet);
