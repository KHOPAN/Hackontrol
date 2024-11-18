#include <WinSock2.h>
#include "hrsp_internal.h"

#define ERROR_WSA(sourceName, functionName)                 if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_NTSTATUS(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_NTSTATUS;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)     if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                         ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	BYTE buffer[8];

	if(recv(socket, buffer, 8, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		return FALSE;
	}

	if(memcmp(buffer, "HRSP", 4)) {
		ERROR_HRSP(ERROR_HRSP_INVALID_MAGIC, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	buffer[0] = ((buffer[4] << 8) | buffer[5]) == HRSP_PROTOCOL_VERSION && ((buffer[6] << 8) | buffer[7]) == HRSP_PROTOCOL_VERSION_MINOR;

	if(send(socket, buffer, 1, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"send");
		return FALSE;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPServerHandshake", NULL);
		return FALSE;
	}

	if(recv(socket, buffer, 4, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		return FALSE;
	}

	ULONG publicKeyLength = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
	PBYTE publicKey = KHOPAN_ALLOCATE(publicKeyLength);

	if(!publicKey) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPServerHandshake", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	if(recv(socket, publicKey, publicKeyLength, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerHandshake", L"recv");
		KHOPAN_DEALLOCATE(publicKey);
		return FALSE;
	}

	BCRYPT_ALG_HANDLE algorithm;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerHandshake", L"BCryptOpenAlgorithmProvider");
		KHOPAN_DEALLOCATE(publicKey);
		return FALSE;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptImportKeyPair(algorithm, NULL, BCRYPT_RSAPUBLIC_BLOB, &key, publicKey, publicKeyLength, 0);
	KHOPAN_DEALLOCATE(publicKey);
	BOOL codeExit = FALSE;

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerHandshake", L"BCryptImportKeyPair");
		goto closeAlgorithm;
	}

	ULONG keyLength;
	status = BCryptEncrypt(key, "Hell", 4, NULL, NULL, 0, NULL, 0, &keyLength, BCRYPT_PAD_PKCS1);
	BCryptDestroyKey(key);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerHandshake", L"BCryptEncrypt");
		goto closeAlgorithm;
	}

	printf("Length: %lu\n", keyLength);
	data->internal = KHOPAN_ALLOCATE(sizeof(INTERNALDATA));

	if(!data->internal) {
		ERROR_COMMON(ERROR_COMMON_FUNCTION_FAILED, L"HRSPServerHandshake", L"KHOPAN_ALLOCATE");
		goto closeAlgorithm;
	}

	((PINTERNALDATA) data->internal)->socket = socket;
	printf("Server: Handshake Done\n");
	ERROR_CLEAR;
	codeExit = TRUE;
closeAlgorithm:
	BCryptCloseAlgorithmProvider(algorithm, 0);
	return codeExit;
}
