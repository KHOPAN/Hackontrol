#include <WinSock2.h>
#include "hrsp.h"

#define ERROR_WSA(sourceName, functionName)                 if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_NTSTATUS(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_NTSTATUS;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)     if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                         ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

#define RSA_KEY_LENGTH 512

#pragma warning(disable: 6385)

typedef struct {
	SOCKET socket;
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

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPClientHandshake", NULL);
		return FALSE;
	}

	BYTE buffer[8];
	buffer[0] = 'H';
	buffer[1] = 'R';
	buffer[2] = 'S';
	buffer[3] = 'P';
	buffer[4] = (HRSP_PROTOCOL_VERSION >> 8) & 0xFF;
	buffer[5] = HRSP_PROTOCOL_VERSION & 0xFF;
	buffer[6] = (HRSP_PROTOCOL_VERSION_MINOR >> 8) & 0xFF;
	buffer[7] = HRSP_PROTOCOL_VERSION_MINOR & 0xFF;
	BOOL codeExit = FALSE;

	if(send(socket, buffer, 8, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientHandshake", L"send");
		return FALSE;
	}

	if(recv(socket, buffer, 1, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientHandshake", L"recv");
		return FALSE;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPClientHandshake", NULL);
		return FALSE;
	}

	BCRYPT_ALG_HANDLE algorithm;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientHandshake", L"BCryptOpenAlgorithmProvider");
		return FALSE;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptGenerateKeyPair(algorithm, &key, RSA_KEY_LENGTH * 8, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientHandshake", L"BCryptGenerateKeyPair");
		goto closeAlgorithm;
	}

	status = BCryptFinalizeKeyPair(key, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientHandshake", L"BCryptFinalizeKeyPair");
		goto destroyKey;
	}

	ULONG publicKeyLength;
	status = BCryptExportKey(key, NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, 0, &publicKeyLength, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientHandshake", L"BCryptExportKey");
		goto destroyKey;
	}

	buffer[0] = (publicKeyLength >> 24) & 0xFF;
	buffer[1] = (publicKeyLength >> 16) & 0xFF;
	buffer[2] = (publicKeyLength >> 8) & 0xFF;
	buffer[3] = publicKeyLength & 0xFF;

	if(send(socket, buffer, 4, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientHandshake", L"send");
		goto destroyKey;
	}

	PBYTE publicKey = KHOPAN_ALLOCATE(publicKeyLength);

	if(!publicKey) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientHandshake", L"KHOPAN_ALLOCATE");
		goto destroyKey;
	}

	status = BCryptExportKey(key, NULL, BCRYPT_RSAPUBLIC_BLOB, publicKey, publicKeyLength, &publicKeyLength, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientHandshake", L"BCryptExportKey");
		KHOPAN_DEALLOCATE(publicKey);
		goto destroyKey;
	}

	if(send(socket, publicKey, publicKeyLength, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientHandshake", L"send");
		KHOPAN_DEALLOCATE(publicKey);
		goto destroyKey;
	}

	KHOPAN_DEALLOCATE(publicKey);
	data->internal = KHOPAN_ALLOCATE(sizeof(INTERNALDATA));

	if(!data->internal) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPClientHandshake", L"KHOPAN_ALLOCATE");
		goto destroyKey;
	}

	((PINTERNALDATA) data->internal)->socket = socket;
	printf("Client: Handshake Done\n");
	ERROR_CLEAR;
	codeExit = TRUE;
destroyKey:
	BCryptDestroyKey(key);
closeAlgorithm:
	BCryptCloseAlgorithmProvider(algorithm, 0);
	return codeExit;
}

BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	PBYTE buffer = KHOPAN_ALLOCATE(max(RSA_KEY_LENGTH, 8));

	if(!buffer) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPServerHandshake", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	BOOL codeExit = FALSE;

	if(recv(socket, buffer, 8, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		goto freeBuffer;
	}

	if(memcmp(buffer, "HRSP", 4)) {
		ERROR_HRSP(ERROR_HRSP_INVALID_MAGIC, L"HRSPServerHandshake", NULL);
		goto freeBuffer;
	}

	buffer[0] = ((buffer[4] << 8) | buffer[5]) == HRSP_PROTOCOL_VERSION && ((buffer[6] << 8) | buffer[7]) == HRSP_PROTOCOL_VERSION_MINOR;

	if(send(socket, buffer, 1, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"send");
		goto freeBuffer;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPServerHandshake", NULL);
		goto freeBuffer;
	}

	if(recv(socket, buffer, 4, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		goto freeBuffer;
	}

	ULONG publicKeyLength = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
	PBYTE publicKey = KHOPAN_ALLOCATE(publicKeyLength);

	if(!publicKey) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPServerHandshake", L"KHOPAN_ALLOCATE");
		goto freeBuffer;
	}

	if(recv(socket, publicKey, publicKeyLength, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		KHOPAN_DEALLOCATE(publicKey);
		goto freeBuffer;
	}

	printf("Key Length: %lu\nData: 0x", publicKeyLength);

	for(ULONG i = 0; i < publicKeyLength; i++) {
		printf("%02X", publicKey[i]);
	}

	printf("\n");
	KHOPAN_DEALLOCATE(publicKey);
	data->internal = KHOPAN_ALLOCATE(sizeof(INTERNALDATA));

	if(!data->internal) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPServerHandshake", L"KHOPAN_ALLOCATE");
		goto freeBuffer;
	}

	((PINTERNALDATA) data->internal)->socket = socket;
	printf("Server: Handshake Done\n");
	ERROR_CLEAR;
	codeExit = TRUE;
freeBuffer:
	KHOPAN_DEALLOCATE(buffer);
	return codeExit;
}

BOOL HRSPPacketSend(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	/*if(!data || !packet) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPPacketSend", NULL);
		return FALSE;
	}

	BYTE buffer[13];
	buffer[0] = packet->size > 0;
	buffer[1] = (packet->type >> 24) & 0xFF;
	buffer[2] = (packet->type >> 16) & 0xFF;
	buffer[3] = (packet->type >> 8) & 0xFF;
	buffer[4] = packet->type & 0xFF;

	if(!buffer[0]) {
		if(send(data->socket, buffer, 5, 0) == SOCKET_ERROR) {
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

	ERROR_CLEAR;*/
	return TRUE;
}

BOOL HRSPPacketReceive(const PHRSPDATA data, const PHRSPPACKET packet, const PKHOPANERROR error) {
	/*if(!data || !packet) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	BYTE buffer[5];
	int status = recv(data->socket, buffer, sizeof(buffer), MSG_WAITALL);

	if(status == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPPacketReceive", L"recv");
		return FALSE;
	}

	if(!status) {
		ERROR_COMMON(ERROR_HRSP_CONNECTION_CLOSED, L"HRSPPacketReceive", NULL);
		return FALSE;
	}

	if(!buffer[0]) {
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

void HRSPCleanup(const PHRSPDATA data) {
	if(!data || !data->internal) {
		return;
	}

	KHOPAN_DEALLOCATE(data->internal);
	printf("Deallocated\n");
}
