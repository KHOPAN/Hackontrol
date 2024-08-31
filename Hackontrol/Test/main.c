#include <stdio.h>
#include <khopanwin32.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	BCRYPT_ALG_HANDLE algorithm;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptGenerateKeyPair(algorithm, &key, 2048, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptGenerateKeyPair");
		return 1;
	}

	status = BCryptDestroyKey(key);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptDestroyKey");
		return 1;
	}

	status = BCryptCloseAlgorithmProvider(algorithm, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR_CONSOLE(status, L"BCryptCloseAlgorithmProvider");
		return 1;
	}

	return 0;
}
