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

static BOOL symmetricKeyEncrypt(const SOCKET socket, const PBYTE publicKey, const ULONG publicKeyLength, const BCRYPT_KEY_HANDLE symmetricKey, const PKHOPANERROR error) {
	BCRYPT_ALG_HANDLE algorithm;
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

	ULONG symmetricKeyLength;
	status = BCryptExportKey(symmetricKey, NULL, BCRYPT_KEY_DATA_BLOB, NULL, 0, &symmetricKeyLength, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptExportKey");
		goto destroyKey;
	}

	PBYTE symmetricKeyBytes = KHOPAN_ALLOCATE(symmetricKeyLength);

	if(!symmetricKeyBytes) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPClientInitialize", L"KHOPAN_ALLOCATE");
		goto destroyKey;
	}

	status = BCryptExportKey(symmetricKey, NULL, BCRYPT_KEY_DATA_BLOB, symmetricKeyBytes, symmetricKeyLength, &symmetricKeyLength, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptExportKey");
		KHOPAN_DEALLOCATE(symmetricKeyBytes);
		goto destroyKey;
	}

	ULONG encryptedLength;
	status = BCryptEncrypt(key, symmetricKeyBytes, symmetricKeyLength, NULL, NULL, 0, NULL, 0, &encryptedLength, BCRYPT_PAD_PKCS1);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptEncrypt");
		KHOPAN_DEALLOCATE(symmetricKeyBytes);
		goto destroyKey;
	}

	PBYTE encryptedBytes = KHOPAN_ALLOCATE(encryptedLength);

	if(!encryptedBytes) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPClientInitialize", L"KHOPAN_ALLOCATE");
		KHOPAN_DEALLOCATE(symmetricKeyBytes);
		goto destroyKey;
	}

	status = BCryptEncrypt(key, symmetricKeyBytes, symmetricKeyLength, NULL, NULL, 0, encryptedBytes, encryptedLength, &encryptedLength, BCRYPT_PAD_PKCS1);
	KHOPAN_DEALLOCATE(symmetricKeyBytes);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPClientInitialize", L"BCryptEncrypt");
		goto freeEncryptedBytes;
	}

	BYTE sizeBuffer[4];
	sizeBuffer[0] = (encryptedLength >> 24) & 0xFF;
	sizeBuffer[1] = (encryptedLength >> 16) & 0xFF;
	sizeBuffer[2] = (encryptedLength >> 8) & 0xFF;
	sizeBuffer[3] = encryptedLength & 0xFF;

	if(send(socket, sizeBuffer, sizeof(sizeBuffer), 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientInitialize", L"send");
		goto freeEncryptedBytes;
	}

	if(send(socket, encryptedBytes, encryptedLength, 0) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPClientInitialize", L"send");
		goto freeEncryptedBytes;
	}

	codeExit = TRUE;
freeEncryptedBytes:
	KHOPAN_DEALLOCATE(encryptedBytes);
destroyKey:
	BCryptDestroyKey(key);
closeAlgorithm:
	BCryptCloseAlgorithmProvider(algorithm, 0);
	return codeExit;
}

BOOL HRSPClientInitialize(const SOCKET socket, const PHRSPDATA data, const PKHOPANERROR error) {
	if(!socket || !data) {
		ERROR_COMMON(ERROR_COMMON_INVALID_PARAMETER, L"HRSPClientInitialize", NULL);
		goto functionExit;
	}

	PINTERNALDATA internal = KHOPAN_ALLOCATE(sizeof(INTERNALDATA));

	if(!internal) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPClientInitialize", L"KHOPAN_ALLOCATE");
		goto functionExit;
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

	buffer[0] = symmetricKeyEncrypt(socket, bytes, publicKeyLength, internal->symmetricKey, error);
	KHOPAN_DEALLOCATE(bytes);

	if(!buffer[0]) {
		goto destroySymmetricKey;
	}

	*data = (HRSPDATA) internal;
	ERROR_CLEAR;
 	return TRUE;
destroySymmetricKey:
	BCryptDestroyKey(internal->symmetricKey);
closeSymmetricAlgorithm:
	BCryptCloseAlgorithmProvider(internal->symmetricAlgorithm, 0);
freeInternal:
	KHOPAN_DEALLOCATE(internal);
functionExit:
	*data = 0;
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