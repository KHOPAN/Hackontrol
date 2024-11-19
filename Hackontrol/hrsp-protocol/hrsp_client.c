#include <WinSock2.h>
#include "hrsp_internal.h"

#define ERROR_WSA(sourceName, functionName)                 if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_NTSTATUS(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_NTSTATUS;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)     if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                         ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

typedef struct {
	BCRYPT_ALG_HANDLE symmetricAlgorithm;
	BCRYPT_KEY_HANDLE symmetricKey;
} INTERNALDATA, *PINTERNALDATA;

BOOL encryptSymmetricKey(const PBYTE publicKey, const ULONG publicKeyLength, const BCRYPT_KEY_HANDLE symmetricKey, const PKHOPANERROR error) {
	/*BCRYPT_ALG_HANDLE algorithm;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptOpenAlgorithmProvider");
		return FALSE;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptImportKeyPair(algorithm, NULL, BCRYPT_RSAPUBLIC_BLOB, &key, publicKey, publicKeyLength, 0);
	BOOL codeExit = FALSE;

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptImportKeyPair");
		goto closeAlgorithm;
	}

	BCryptDestroyKey(key);
	codeExit = TRUE;
closeAlgorithm:
	BCryptCloseAlgorithmProvider(algorithm, 0);
	return codeExit;*/
	return TRUE;
}

BOOL HRSPClientInitialize(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPClientInitialize", NULL);
		return FALSE;
	}

	PINTERNALDATA internal = KHOPAN_ALLOCATE(sizeof(INTERNALDATA));

	if(!internal) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPClientInitialize", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	NTSTATUS status = BCryptOpenAlgorithmProvider(&internal->symmetricAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptOpenAlgorithmProvider");
		goto freeInternal;
	}

	PBYTE bytes = KHOPAN_ALLOCATE(HRSP_AES_KEY_LENGTH);

	if(!bytes) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPClientInitialize", L"KHOPAN_ALLOCATE");
		goto closeSymmetricAlgorithm;
	}

	status = BCryptGenRandom(NULL, bytes, HRSP_AES_KEY_LENGTH, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptGenRandom");
		KHOPAN_DEALLOCATE(bytes);
		goto closeSymmetricAlgorithm;
	}

	status = BCryptGenerateSymmetricKey(internal->symmetricAlgorithm, &internal->symmetricKey, NULL, 0, bytes, HRSP_AES_KEY_LENGTH, 0);
	KHOPAN_DEALLOCATE(bytes);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptGenerateSymmetricKey");
		goto closeSymmetricAlgorithm;
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

	if(send(socket, buffer, 8, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientInitialize", L"send");
		goto destroySymmetricKey;
	}

	if(recv(socket, buffer, 1, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientInitialize", L"recv");
		goto destroySymmetricKey;
	}

	if(!buffer[0]) {
		ERROR_HRSP(ERROR_HRSP_UNSUPPORTED_VERSION, L"HRSPClientInitialize", NULL);
		goto destroySymmetricKey;
	}

	if(recv(socket, buffer, 4, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientInitialize", L"recv");
		goto destroySymmetricKey;
	}

	ULONG publicKeyLength = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
	bytes = KHOPAN_ALLOCATE(publicKeyLength);

	if(!bytes) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPClientInitialize", L"KHOPAN_ALLOCATE");
		goto destroySymmetricKey;
	}

	if(recv(socket, bytes, publicKeyLength, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientInitialize", L"recv");
		KHOPAN_DEALLOCATE(bytes);
		goto destroySymmetricKey;
	}

	buffer[0] = encryptSymmetricKey(bytes, publicKeyLength, internal->symmetricKey, error);
	KHOPAN_DEALLOCATE(bytes);

	if(!buffer[0]) {
		goto destroySymmetricKey;
	}

	*data = (HRSPDATA) internal;
 	return TRUE;
destroySymmetricKey:
	BCryptDestroyKey(internal->symmetricKey);
closeSymmetricAlgorithm:
	BCryptCloseAlgorithmProvider(internal->symmetricAlgorithm, 0);
freeInternal:
	KHOPAN_DEALLOCATE(internal);
	return FALSE;
}

void HRSPClientCleanup(const PHRSPDATA data) {
	if(!data) {
		return;
	}

	PINTERNALDATA internal = (PINTERNALDATA) *data;

	if(!internal) {
		return;
	}

	BCryptDestroyKey(internal->symmetricKey);
	BCryptCloseAlgorithmProvider(internal->symmetricAlgorithm, 0);
	KHOPAN_DEALLOCATE(internal);
}

/*#pragma warning(disable: 6385)

BOOL HRSPClientHandshake(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	BCRYPT_ALG_HANDLE algorithm;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientHandshake", L"BCryptOpenAlgorithmProvider");
		return FALSE;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptGenerateKeyPair(algorithm, &key, HRSP_RSA_KEY_LENGTH * 8, 0);
	BOOL codeExit = FALSE;

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
}*/
