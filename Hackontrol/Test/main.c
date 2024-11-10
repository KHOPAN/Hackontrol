#include <libkhopan.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	BCRYPT_ALG_HANDLE provider;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&provider, BCRYPT_AES_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	PBYTE buffer = KHOPAN_ALLOCATE(32);
	int codeExit = 1;

	if(!buffer) {
		KHOPANERRORMESSAGE_WIN32(ERROR_FUNCTION_FAILED, L"KHOPAN_ALLOCATE");
		goto closeProvider;
	}

	BCRYPT_KEY_HANDLE key;
	status = BCryptGenerateSymmetricKey(provider, &key, NULL, 0, (PUCHAR) buffer, 32, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptGenerateSymmetricKey");
		goto freeBuffer;
	}

	codeExit = 0;
destroyKey:
	status = BCryptDestroyKey(key);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptDestroyKey");
		codeExit = 1;
	}
freeBuffer:
	KHOPAN_DEALLOCATE(buffer);
closeProvider:
	status = BCryptCloseAlgorithmProvider(provider, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptCloseAlgorithmProvider");
		codeExit = 1;
	}

	return codeExit;
}
