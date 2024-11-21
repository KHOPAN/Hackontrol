#include <WinSock2.h>
#include "hrsp.h"

#define ERROR_WSA(sourceName, functionName)                 if(error){error->facility=ERROR_FACILITY_WIN32;error->code=WSAGetLastError();error->source=sourceName;error->function=functionName;}
#define ERROR_NTSTATUS(codeError, sourceName, functionName) if(error){error->facility=ERROR_FACILITY_NTSTATUS;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_COMMON(codeError, sourceName, functionName)   if(error){error->facility=ERROR_FACILITY_COMMON;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_HRSP(codeError, sourceName, functionName)     if(error){error->facility=ERROR_FACILITY_HRSP;error->code=codeError;error->source=sourceName;error->function=functionName;}
#define ERROR_CLEAR                                         ERROR_COMMON(ERROR_COMMON_SUCCESS,NULL,NULL)

typedef struct {
	BCRYPT_ALG_HANDLE symmetricAlgorithm;
	BCRYPT_ALG_HANDLE asymmetricAlgorithm;
	BCRYPT_KEY_HANDLE asymmetricKey;
	ULONG publicKeyLength;
	PBYTE publicKey;
} INTERNALSERVERDATA, *PINTERNALSERVERDATA;

typedef struct {
	SOCKET socket;
	BCRYPT_KEY_HANDLE symmetricKey;
	PBYTE buffer;
	size_t bufferSize;
} INTERNALDATA, *PINTERNALDATA;

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

	NTSTATUS status = BCryptOpenAlgorithmProvider(&data->symmetricAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptOpenAlgorithmProvider");
		goto freeData;
	}

	status = BCryptOpenAlgorithmProvider(&data->asymmetricAlgorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerInitialize", L"BCryptOpenAlgorithmProvider");
		goto closeSymmetricAlgorithm;
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

	*server = (HRSPSERVERDATA) data;
	ERROR_CLEAR;
	return TRUE;
freePublicKey:
	KHOPAN_DEALLOCATE(data->publicKey);
destroyAsymmetricKey:
	BCryptDestroyKey(data->asymmetricKey);
closeAsymmetricAlgorithm:
	BCryptCloseAlgorithmProvider(data->asymmetricAlgorithm, 0);
closeSymmetricAlgorithm:
	BCryptCloseAlgorithmProvider(data->symmetricAlgorithm, 0);
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

	if(recv(socket, buffer, 4, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerSessionInitialize", L"recv");
		return FALSE;
	}

	ULONG encryptedLength = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
	PBYTE encryptedBytes = KHOPAN_ALLOCATE(encryptedLength);

	if(!encryptedBytes) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPServerSessionInitialize", L"KHOPAN_ALLOCATE");
		return FALSE;
	}

	if(recv(socket, encryptedBytes, encryptedLength, MSG_WAITALL) == SOCKET_ERROR) {
		ERROR_WSA(L"HRSPServerSessionInitialize", L"recv");
		KHOPAN_DEALLOCATE(encryptedBytes);
		return FALSE;
	}

	ULONG symmetricKeyLength;
	NTSTATUS status = BCryptDecrypt(internal->asymmetricKey, encryptedBytes, encryptedLength, NULL, NULL, 0, NULL, 0, &symmetricKeyLength, BCRYPT_PAD_PKCS1);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerSessionInitialize", L"BCryptDecrypt");
		KHOPAN_DEALLOCATE(encryptedBytes);
		return FALSE;
	}

	PBYTE symmetricKeyBytes = KHOPAN_ALLOCATE(symmetricKeyLength);

	if(!symmetricKeyBytes) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPServerSessionInitialize", L"KHOPAN_ALLOCATE");
		KHOPAN_DEALLOCATE(encryptedBytes);
		return FALSE;
	}

	status = BCryptDecrypt(internal->asymmetricKey, encryptedBytes, encryptedLength, NULL, NULL, 0, symmetricKeyBytes, symmetricKeyLength, &symmetricKeyLength, BCRYPT_PAD_PKCS1);
	KHOPAN_DEALLOCATE(encryptedBytes);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerSessionInitialize", L"BCryptDecrypt");
		KHOPAN_DEALLOCATE(symmetricKeyBytes);
		return FALSE;
	}

	PINTERNALDATA session = KHOPAN_ALLOCATE(sizeof(INTERNALDATA));

	if(!session) {
		ERROR_COMMON(ERROR_COMMON_ALLOCATION_FAILED, L"HRSPServerSessionInitialize", L"KHOPAN_ALLOCATE");
		KHOPAN_DEALLOCATE(symmetricKeyBytes);
		return FALSE;
	}

	session->socket = socket;
	session->buffer = NULL;
	session->bufferSize = 0;
	status = BCryptImportKey(internal->symmetricAlgorithm, NULL, BCRYPT_KEY_DATA_BLOB, &session->symmetricKey, NULL, 0, symmetricKeyBytes, symmetricKeyLength, 0);
	KHOPAN_DEALLOCATE(symmetricKeyBytes);

	if(!BCRYPT_SUCCESS(status)) {
		ERROR_NTSTATUS(status, L"HRSPServerSessionInitialize", L"BCryptImportKey");
		KHOPAN_DEALLOCATE(session);
		return FALSE;
	}

	*data = (HRSPDATA) session;
	ERROR_CLEAR;
	return TRUE;
}

void HRSPServerSessionCleanup(const PHRSPDATA data) {
	if(!data) {
		return;
	}

	PINTERNALDATA session = (PINTERNALDATA) *data;

	if(session) {
		if(session->buffer) {
			KHOPAN_DEALLOCATE(session->buffer);
		}

		BCryptDestroyKey(session->symmetricKey);
		KHOPAN_DEALLOCATE(session);
	}

	*data = 0;
}

void HRSPServerCleanup(const PHRSPSERVERDATA server) {
	if(!server) {
		return;
	}

	PINTERNALSERVERDATA data = (PINTERNALSERVERDATA) *server;

	if(data) {
		KHOPAN_DEALLOCATE(data->publicKey);
		BCryptDestroyKey(data->asymmetricKey);
		BCryptCloseAlgorithmProvider(data->asymmetricAlgorithm, 0);
		BCryptCloseAlgorithmProvider(data->symmetricAlgorithm, 0);
		KHOPAN_DEALLOCATE(data);
	}

	*server = 0;
}
