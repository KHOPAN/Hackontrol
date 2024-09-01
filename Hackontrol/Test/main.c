#include <stdio.h>
#include <khopanwin32.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	BYTE inputData[] = "Hello, world! This is example text";
	BCRYPT_ALG_HANDLE algorithm;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptGenerateKeyPair(algorithm, &key, 2048, 0);
	int returnValue = 1;

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptGenerateKeyPair");
		goto closeAlgorithmProvider;
	}

	status = BCryptFinalizeKeyPair(key, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptFinalizeKeyPair");
		goto destroyKey;
	}

	ULONG encryptedSize;
	status = BCryptEncrypt(key, inputData, sizeof(inputData) - 1, NULL, NULL, 0, NULL, 0, &encryptedSize, BCRYPT_PAD_PKCS1);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptEncrypt");
		goto destroyKey;
	}

	BYTE* encryptedBuffer = LocalAlloc(LMEM_FIXED, encryptedSize + 1);

	if(!encryptedBuffer) {
		KHWIN32_LAST_ERROR_CONSOLE(L"LocalAlloc");
		goto destroyKey;
	}

	status = BCryptEncrypt(key, inputData, sizeof(inputData) - 1, NULL, NULL, 0, encryptedBuffer, encryptedSize, &encryptedSize, BCRYPT_PAD_PKCS1);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptEncrypt");
		goto freeEncryptedBuffer;
	}

	encryptedBuffer[encryptedSize] = 0;
	printf("Encrypted size: %lu\nEncrypted data: %s\n", encryptedSize, encryptedBuffer);
	ULONG decryptedSize;
	status = BCryptDecrypt(key, encryptedBuffer, encryptedSize, NULL, NULL, 0, NULL, 0, &decryptedSize, BCRYPT_PAD_PKCS1);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptDecrypt");
		goto freeEncryptedBuffer;
	}

	BYTE* decryptedBuffer = LocalAlloc(LMEM_FIXED, decryptedSize + 1);

	if(!decryptedBuffer) {
		KHWIN32_LAST_ERROR_CONSOLE(L"LocalAlloc");
		goto freeEncryptedBuffer;
	}

	status = BCryptDecrypt(key, encryptedBuffer, encryptedSize, NULL, NULL, 0, decryptedBuffer, decryptedSize, &decryptedSize, BCRYPT_PAD_PKCS1);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptDecrypt");
		goto freeDecryptedBuffer;
	}

	printf("Decrypted size: %lu\nDecrypted data: %s\n", decryptedSize, decryptedBuffer);
	returnValue = 0;
freeDecryptedBuffer:
	if(LocalFree(decryptedBuffer)) {
		KHWIN32_LAST_ERROR_CONSOLE(L"LocalAlloc");
		returnValue = 1;
	}
freeEncryptedBuffer:
	if(LocalFree(encryptedBuffer)) {
		KHWIN32_LAST_ERROR_CONSOLE(L"LocalAlloc");
		returnValue = 1;
	}
destroyKey:
	status = BCryptDestroyKey(key);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptDestroyKey");
		returnValue = 1;
	}
closeAlgorithmProvider:
	status = BCryptCloseAlgorithmProvider(algorithm, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptCloseAlgorithmProvider");
		return 1;
	}

	return returnValue;
}
