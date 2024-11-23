#include <WinSock2.h>
#include "hrsp.h"

#define ERROR_WSA(sourceName, functionName)                 if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_NTSTATUS(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_NTSTATUS;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)     if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                         ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

typedef struct {
	SOCKET socket;
	BCRYPT_KEY_HANDLE symmetricKey;
	PBYTE buffer;
	size_t bufferSize;
} INTERNALDATA, *PINTERNALDATA;

LPCWSTR HRSPErrorHRSPDecoder(const PKHOPANERROR error) {
	if(!error) {
		return NULL;
	}

	if(error->facility != ERROR_FACILITY_HRSP) {
		return KHOPANErrorCommonDecoder(error);
	}

	switch(error->code) {
	case ERROR_HRSP_INVALID_MAGIC:       return L"Invalid HRSP magic number";
	case ERROR_HRSP_UNSUPPORTED_VERSION: return L"Incompatible client and server version";
	case ERROR_HRSP_CONNECTION_CLOSED:   return L"The connection was already closed";
	case ERROR_HRSP_INVALID_HEADER_SIZE: return L"Invalid packet header size";
	default:                             return L"Unknown error code";
	}
}

static BOOL expandBuffer(const PINTERNALDATA data, const size_t size) {
	if(data->buffer && data->bufferSize >= size) {
		return TRUE;
	}

	PBYTE buffer = KHOPAN_ALLOCATE(size);

	if(!buffer) {
		return FALSE;
	}

	if(data->buffer) {
		KHOPAN_DEALLOCATE(data->buffer);
	}

	data->buffer = buffer;
	data->bufferSize = size;
	return TRUE;
}

static BOOL sendDataChunk(const SOCKET socket, const PBYTE data, const ULONG size) {
	printf("Chunk size: %lu\n", size);
	ULONG pointer = 0;

	while(pointer < size) {
		ULONG available = size - pointer;
		int sent = send(socket, data + pointer, (int) min(available, /*INT_MAX*/3), 0);
		printf("Subchunk size: %d\n", sent);

		if(sent == SOCKET_ERROR) {
			return FALSE;
		}

		pointer += sent;
	}

	return TRUE;
}

BOOL HRSPPacketSend(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!data || !*data || !packet) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPPacketSend", NULL);
		return FALSE;
	}

	BYTE buffer[12];
	buffer[0] = (packet->type >> 24) & 0xFF;
	buffer[1] = (packet->type >> 16) & 0xFF;
	buffer[2] = (packet->type >> 8) & 0xFF;
	buffer[3] = packet->type & 0xFF;

	if(packet->size > 0) {
		buffer[4] = (packet->size >> 56) & 0xFF;
		buffer[5] = (packet->size >> 48) & 0xFF;
		buffer[6] = (packet->size >> 40) & 0xFF;
		buffer[7] = (packet->size >> 32) & 0xFF;
		buffer[8] = (packet->size >> 24) & 0xFF;
		buffer[9] = (packet->size >> 16) & 0xFF;
		buffer[10] = (packet->size >> 8) & 0xFF;
		buffer[11] = packet->size & 0xFF;
	}

	PINTERNALDATA internal = (PINTERNALDATA) *data;
	ULONG size;
	NTSTATUS status = BCryptEncrypt(internal->symmetricKey, buffer, packet->size > 0 ? 12 : 4, NULL, NULL, 0, NULL, 0, &size, BCRYPT_BLOCK_PADDING);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPPacketSend", L"BCryptEncrypt");
		return FALSE;
	}

	if(!expandBuffer(internal, size)) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketSend", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	status = BCryptEncrypt(internal->symmetricKey, buffer, packet->size > 0 ? 12 : 4, NULL, NULL, 0, internal->buffer, size, &size, BCRYPT_BLOCK_PADDING);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPPacketSend", L"BCryptEncrypt");
		return FALSE;
	}

	buffer[0] = (size >> 24) & 0xFF;
	buffer[1] = (size >> 16) & 0xFF;
	buffer[2] = (size >> 8) & 0xFF;
	buffer[3] = size & 0xFF;

	if(send(internal->socket, buffer, 4, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketSend", L"send");
		return FALSE;
	}

	if(send(internal->socket, internal->buffer, size, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketSend", L"send");
		return FALSE;
	}

	size_t pointer = 0;

	while(pointer < packet->size) {
		size_t available = packet->size - pointer;
		size = (ULONG) min(available, 13);
		ULONG requiredSize;
		status = BCryptEncrypt(internal->symmetricKey, ((PBYTE) packet->data) + pointer, size, NULL, NULL, 0, NULL, 0, &requiredSize, BCRYPT_BLOCK_PADDING);

		if(!BCRYPT_SUCCESS(status)) {
			ERROR_NTSTATUS(status, L"HRSPPacketSend", L"BCryptEncrypt");
			return FALSE;
		}

		if(!expandBuffer(internal, requiredSize)) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketSend", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		status = BCryptEncrypt(internal->symmetricKey, ((PBYTE) packet->data) + pointer, size, NULL, NULL, 0, internal->buffer, requiredSize, &requiredSize, BCRYPT_BLOCK_PADDING);

		if(!BCRYPT_SUCCESS(status)) {
			ERROR_NTSTATUS(status, L"HRSPPacketSend", L"BCryptEncrypt");
			return FALSE;
		}

		buffer[0] = (requiredSize >> 24) & 0xFF;
		buffer[1] = (requiredSize >> 16) & 0xFF;
		buffer[2] = (requiredSize >> 8) & 0xFF;
		buffer[3] = requiredSize & 0xFF;

		if(send(internal->socket, buffer, 4, 0) == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketSend", L"send");
			return FALSE;
		}

		if(!sendDataChunk(internal->socket, internal->buffer, requiredSize)) {
			ERROR_WSA(L"HRSPPacketSend", L"send");
			return FALSE;
		}

		pointer += size;
	}

	printf("Data size: %llu\n", packet->size);
	ERROR_CLEAR;
	return TRUE;
}

BOOL HRSPPacketReceive(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!data || !*data || !packet) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	PINTERNALDATA internal = (PINTERNALDATA) *data;
	BYTE buffer[4];
	NTSTATUS status = recv(internal->socket, buffer, 4, MSG_WAITALL);

	if(status == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketReceive", L"recv");
		return FALSE;
	}

	if(!status) {
		ERROR_HRSP(ERROR_HRSP_CONNECTION_CLOSED, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	ULONG size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];

	if(!expandBuffer(internal, size)) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketReceive", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	if(recv(internal->socket, internal->buffer, size, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketReceive", L"recv");
		return FALSE;
	}

	status = BCryptDecrypt(internal->symmetricKey, internal->buffer, size, NULL, NULL, 0, internal->buffer, size, &size, BCRYPT_BLOCK_PADDING);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPPacketReceive", L"BCryptDecrypt");
		return FALSE;
	}

	if(size != 4 && size != 12) {
		ERROR_HRSP(ERROR_HRSP_INVALID_HEADER_SIZE, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	if(size == 4) {
		packet->type = (internal->buffer[0] << 24) | (internal->buffer[1] << 16) | (internal->buffer[2] << 8) | internal->buffer[3];
		packet->size = 0;
		packet->data = NULL;
		ERROR_CLEAR;
		return TRUE;
	}

	size = (internal->buffer[0] << 24) | (internal->buffer[1] << 16) | (internal->buffer[2] << 8) | internal->buffer[3];
	size_t dataSize = (((size_t) internal->buffer[4]) << 56) | (((size_t) internal->buffer[5]) << 48) | (((size_t) internal->buffer[6]) << 40) | (((size_t) internal->buffer[7]) << 32) | (((size_t) internal->buffer[8]) << 24) | (((size_t) internal->buffer[9]) << 16) | (((size_t) internal->buffer[10]) << 8) | ((size_t) internal->buffer[11]);
	PBYTE packetBuffer = KHOPAN_ALLOCATE(dataSize);

	if(!packetBuffer) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketReceive", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	size_t pointer = 0;

	while(pointer < dataSize) {
		if(recv(internal->socket, buffer, 4, MSG_WAITALL) == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketReceive", L"recv");
			KHOPAN_DEALLOCATE(packetBuffer);
			return FALSE;
		}

		ULONG chunkSize = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
		printf("Chunk size: %lu\n", chunkSize);

		if(!expandBuffer(internal, chunkSize)) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketReceive", L"KHOPAN_ALLOCATE");
			KHOPAN_DEALLOCATE(packetBuffer);
			return FALSE;
		}

		if(recv(internal->socket, internal->buffer, chunkSize, MSG_WAITALL) == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketReceive", L"recv");
			KHOPAN_DEALLOCATE(packetBuffer);
			return FALSE;
		}

		ULONG available = (ULONG) (dataSize - pointer);
		available = (ULONG) min(chunkSize, available);
		status = BCryptDecrypt(internal->symmetricKey, internal->buffer, chunkSize, NULL, NULL, 0, packetBuffer + pointer, available, &chunkSize, BCRYPT_BLOCK_PADDING);

		if(!BCRYPT_SUCCESS(status)) {
			ERROR_NTSTATUS(status, L"HRSPPacketReceive", L"BCryptDecrypt");
			KHOPAN_DEALLOCATE(packetBuffer);
			return FALSE;
		}

		pointer += chunkSize;
	}

	packet->type = size;
	packet->size = dataSize;
	packet->data = packetBuffer;
	printf("Data size: %llu\n", packet->size);
	ERROR_CLEAR;
	return TRUE;
}
