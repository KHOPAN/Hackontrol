#pragma once

#include <WinSock2.h>

/*
 * Client: x+8-bytes Screen size & Username
 * Server: 0-bytes Termination signal
 */
#define PACKET_TYPE_INFORMATION  1

 /*
  * Client: x-bytes Stream frame
  * Server: 1-byte Stream settings
  */
#define PACKET_TYPE_STREAM_FRAME 2

typedef struct {
	long size;
	char packetType;
	void* data;
} PACKET;

BOOL SendPacket(const SOCKET socket, const PACKET* packet);
BOOL ReceivePacket(const SOCKET socket, PACKET* packet);
