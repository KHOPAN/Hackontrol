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
	default:                             return L"Unknown error code";
	}
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

	if(!internal->buffer || internal->bufferSize < size) {
		PBYTE temporary = KHOPAN_ALLOCATE(size);

		if(!temporary) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketSend", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		if(internal->buffer) {
			KHOPAN_DEALLOCATE(internal->buffer);
		}

		internal->buffer = temporary;
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

	return TRUE;
	/*if(!buffer[0]) {
		if(send(internal->socket, buffer, 5, 0) == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketSend", L"send");
			return FALSE;
		}

		ERROR_CLEAR;
		return TRUE;
	}

	buffer[5] = (packet->size >> 56) & 0xFF;
	buffer[6] = (packet->size >> 48) & 0xFF;
	buffer[7] = (packet->size >> 40) & 0xFF;
	buffer[8] = (packet->size >> 32) & 0xFF;
	buffer[9] = (packet->size >> 24) & 0xFF;
	buffer[10] = (packet->size >> 16) & 0xFF;
	buffer[11] = (packet->size >> 8) & 0xFF;
	buffer[12] = packet->size & 0xFF;

	if(send(data->socket, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketSend", L"send");
		return FALSE;
	}

	size_t pointer = 0;

	while(pointer < packet->size) {
		size_t size = packet->size - pointer;
		int sent = send(data->socket, ((PBYTE) packet->data) + pointer, (int) min(size, INT_MAX), 0);

		if(sent == SOCKET_ERROR) {
			ERROR_WSA(L"HRSPPacketSend", L"send");
			return FALSE;
		}

		pointer += sent;
	}

	ERROR_CLEAR;
	return TRUE;*/
}

BOOL HRSPPacketReceive(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!data || !*data || !packet) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	BYTE buffer[4];
	PINTERNALDATA internal = (PINTERNALDATA) *data;
	int status = recv(internal->socket, buffer, 4, MSG_WAITALL);

	if(status == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketReceive", L"recv");
		return FALSE;
	}

	if(!status) {
		ERROR_COMMON(ERROR_HRSP_CONNECTION_CLOSED, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	ULONG size = ((buffer[0] & 0xFF) << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);

	if(!internal->buffer || internal->bufferSize < size) {
		PBYTE temporary = KHOPAN_ALLOCATE(size);

		if(!temporary) {
			ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPPacketReceive", L"KHOPAN_ALLOCATE");
			return FALSE;
		}

		if(internal->buffer) {
			KHOPAN_DEALLOCATE(internal->buffer);
		}

		internal->buffer = temporary;
	}

	if(recv(internal->socket, internal->buffer, size, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketReceive", L"recv");
		return FALSE;
	}

	printf("Header size: %lu\n", size);
	/*if(!buffer[0]) {
		packet->type = ((buffer[1] & 0xFF) << 24) | ((buffer[2] & 0xFF) << 16) | ((buffer[3] & 0xFF) << 8) | (buffer[4] & 0xFF);
		packet->size = 0;
		packet->data = NULL;
		ERROR_CLEAR;
		return TRUE;
	}

	BYTE sizeBuffer[8];
	status = recv(data->socket, sizeBuffer, sizeof(sizeBuffer), MSG_WAITALL);

	if(status == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketReceive", L"recv");
		return FALSE;
	}

	LARGE_INTEGER size;
	size.HighPart = ((sizeBuffer[0] & 0xFF) << 24) | ((sizeBuffer[1] & 0xFF) << 16) | ((sizeBuffer[2] & 0xFF) << 8) | (sizeBuffer[3] & 0xFF);
	size.LowPart = ((sizeBuffer[4] & 0xFF) << 24) | ((sizeBuffer[5] & 0xFF) << 16) | ((sizeBuffer[6] & 0xFF) << 8) | (sizeBuffer[7] & 0xFF);
	printf("Size: %llu\n", size.QuadPart);
	ERROR_CLEAR;*/
	return TRUE;
}

/*#include <WinSock2.h>
#include "hrsp.h"

#define ERROR_HRSP(errorCode, functionName) if(error){error->win32=FALSE;error->code=errorCode;error->function=functionName;}
#define ERROR_WIN32(errorCode, functionName) if(error){error->win32=TRUE;error->code=errorCode;error->function=functionName;}
#define ERROR_NEUTRAL if(error){error->win32=FALSE;error->code=0;error->function=NULL;}

BOOL HRSPSendPacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPSendPacket");
		return FALSE;
	}

	int size = packet->size < 1 ? 0 : packet->size;
	BYTE header[8];
	header[0] = (size >> 24) & 0xFF;
	header[1] = (size >> 16) & 0xFF;
	header[2] = (size >> 8) & 0xFF;
	header[3] = size & 0xFF;
	header[4] = (packet->type >> 24) & 0xFF;
	header[5] = (packet->type >> 16) & 0xFF;
	header[6] = (packet->type >> 8) & 0xFF;
	header[7] = packet->type & 0xFF;

	if(send(socket, header, sizeof(header), 0) == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"send");
		return FALSE;
	}

	if(size && send(socket, packet->data, packet->size, 0) == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"send");
		return FALSE;
	}

	ERROR_NEUTRAL;
	return TRUE;
}

BOOL HRSPReceivePacket(const SOCKET socket, const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!socket || !data || !packet) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPReceivePacket");
		return FALSE;
	}

	BYTE header[8];
	int result = recv(socket, header, sizeof(header), MSG_WAITALL);

	if(result == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"recv");
		return FALSE;
	}

	if(!result) {
		ERROR_HRSP(HRSP_ERROR_CONNECTION_CLOSED, L"HRSPReceivePacket");
		return FALSE;
	}

	int size = ((header[0] & 0xFF) << 24) | ((header[1] & 0xFF) << 16) | ((header[2] & 0xFF) << 8) | (header[3] & 0xFF);
	UINT type = ((header[4] & 0xFF) << 24) | ((header[5] & 0xFF) << 16) | ((header[6] & 0xFF) << 8) | (header[7] & 0xFF);

	if(size < 1) {
		packet->size = 0;
		packet->type = type;
		packet->data = NULL;
		ERROR_NEUTRAL;
		return TRUE;
	}

	PBYTE buffer = LocalAlloc(LMEM_FIXED, size);

	if(!buffer) {
		ERROR_WIN32(GetLastError(), L"LocalAlloc");
		return FALSE;
	}

	result = recv(socket, buffer, size, MSG_WAITALL);

	if(result == SOCKET_ERROR) {
		ERROR_WIN32(WSAGetLastError(), L"recv");
		LocalFree(buffer);
		return FALSE;
	}

	if(!result) {
		ERROR_HRSP(HRSP_ERROR_CONNECTION_CLOSED, L"HRSPReceivePacket");
		LocalFree(buffer);
		return FALSE;
	}

	packet->size = size;
	packet->type = type;
	packet->data = buffer;
	ERROR_NEUTRAL;
	return TRUE;
}

BOOL HRSPFreePacket(const PHRSPPACKET packet, const PKHOPANERROR error) {
	if(!packet) {
		ERROR_HRSP(HRSP_ERROR_INVALID_FUNCTION_PARAMETER, L"HRSPFreePacket");
		return FALSE;
	}

	if(!packet->data || packet->size < 1) {
		ERROR_NEUTRAL;
		return TRUE;
	}

	if(LocalFree(packet->data)) {
		ERROR_WIN32(GetLastError(), L"LocalFree");
		return FALSE;
	}

	packet->size = 0;
	packet->type = 0;
	packet->data = NULL;
	ERROR_NEUTRAL;
	return TRUE;
}

BOOL HRSPSendTypePacket(const SOCKET socket, const PHRSPDATA data, const UINT type, const PKHOPANERROR error) {
	HRSPPACKET packet = {0};
	packet.type = type;
	return HRSPSendPacket(socket, data, &packet, error);
	return TRUE;
}*/
