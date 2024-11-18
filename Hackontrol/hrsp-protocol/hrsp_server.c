#include <WinSock2.h>
#include "hrsp_internal.h"

#define ERROR_WSA(sourceName, functionName)                 if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_NTSTATUS(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_NTSTATUS;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)     if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                         ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

typedef struct {
	BCRYPT_ALG_HANDLE asymmetricAlgorithm;
	BCRYPT_KEY_HANDLE asymmetricKey;
	ULONG publicKeyLength;
	PBYTE publicKey;
} INTERNALSERVERDATA, *PINTERNALSERVERDATA;

BOOL HRSPServerInitialize(const PHRSPSERVERDATA server, const PKHOPANERROR error) {
	if(!server) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPServerInitialize", NULL);
		return FALSE;
	}

	PINTERNALSERVERDATA data = KHOPAN_ALLOCATE(sizeof(INTERNALSERVERDATA));

	if(!data) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPServerInitialize", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	NTSTATUS status = BCryptOpenAlgorithmProvider(&data->asymmetricAlgorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptOpenAlgorithmProvider");
		goto freeData;
	}

	status = BCryptGenerateKeyPair(data->asymmetricAlgorithm, &data->asymmetricKey, HRSP_RSA_KEY_LENGTH * 8, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptGenerateKeyPair");
		goto closeAsymmetricAlgorithm;
	}

	status = BCryptFinalizeKeyPair(data->asymmetricKey, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptFinalizeKeyPair");
		goto destroyAsymmetricKey;
	}

	status = BCryptExportKey(data->asymmetricKey, NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, 0, &data->publicKeyLength, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptExportKey");
		goto destroyAsymmetricKey;
	}

	data->publicKey = KHOPAN_ALLOCATE(data->publicKeyLength);

	if(!data->publicKey) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPServerInitialize", L"KHOPAN_ALLOCATE");
		goto destroyAsymmetricKey;
	}

	status = BCryptExportKey(data->asymmetricKey, NULL, BCRYPT_RSAPUBLIC_BLOB, data->publicKey, data->publicKeyLength, &data->publicKeyLength, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptExportKey");
		goto freePublicKey;
	}

	ERROR_CLEAR;
	*server = (HRSPSERVERDATA) data;
	return TRUE;
freePublicKey:
	KHOPAN_DEALLOCATE(data->publicKey);
destroyAsymmetricKey:
	BCryptDestroyKey(data->asymmetricKey);
closeAsymmetricAlgorithm:
	BCryptCloseAlgorithmProvider(data->asymmetricAlgorithm, 0);
freeData:
	KHOPAN_DEALLOCATE(data);
	return FALSE;
}

BOOL HRSPServerSessionInitialize(const SOCKET socket, const PHRSPDATA data, const PHRSPSERVERDATA server, const PKHOPANERROR error) {
	if(!socket || !data || !server || !*server) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPServerSessionInitialize", NULL);
		return FALSE;
	}

	BYTE buffer[8];

	if(recv(socket, buffer, 8, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerSessionInitialize", L"recv");
		return FALSE;
	}

	if(memcmp(buffer, "HRSP", 4)) {
		ERROR_HRSP(ERROR_HRSP_INVALID_MAGIC, L"HRSPServerSessionInitialize", NULL);
		return FALSE;
	}

	buffer[0] = ((buffer[4] << 8) | buffer[5]) == HRSP_PROTOCOL_VERSION && ((buffer[6] << 8) | buffer[7]) == HRSP_PROTOCOL_VERSION_MINOR;

	if(send(socket, buffer, 1, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerSessionInitialize", L"send");
		return FALSE;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPServerSessionInitialize", NULL);
		return FALSE;
	}

	PINTERNALSERVERDATA internal = (PINTERNALSERVERDATA) *server;
	buffer[0] = (internal->publicKeyLength >> 24) & 0xFF;
	buffer[1] = (internal->publicKeyLength >> 16) & 0xFF;
	buffer[2] = (internal->publicKeyLength >> 8) & 0xFF;
	buffer[3] = internal->publicKeyLength & 0xFF;

	if(send(socket, buffer, 4, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerSessionInitialize", L"send");
		return FALSE;
	}

	if(send(socket, internal->publicKey, internal->publicKeyLength, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerSessionInitialize", L"send");
		return FALSE;
	}

	ERROR_CLEAR;
	return TRUE;
}

void HRSPServerSessionCleanup(const PHRSPDATA data) {

}

void HRSPServerCleanup(const PHRSPSERVERDATA server) {
	if(!server) {
		return;
	}

	PINTERNALSERVERDATA data = (PINTERNALSERVERDATA) *server;

	if(!data) {
		return;
	}

	KHOPAN_DEALLOCATE(data->publicKey);
	BCryptDestroyKey(data->asymmetricKey);
	BCryptCloseAlgorithmProvider(data->asymmetricAlgorithm, 0);
	KHOPAN_DEALLOCATE(data);
	*server = 0;
}

/*BOOL HRSPServerHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
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
}*/
