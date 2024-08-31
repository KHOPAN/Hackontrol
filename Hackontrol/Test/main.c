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
	status = BCryptEncrypt(key, inputData, sizeof(inputData) - 1, NULL, NULL, 0, NULL, 0, &encryptedSize, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptEncrypt");
		goto destroyKey;
	}

	printf("Encrypted size: %lu\n", encryptedSize);
	returnValue = 0;
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
