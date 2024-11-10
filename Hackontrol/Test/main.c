#include <libkhopan.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	BCRYPT_ALG_HANDLE provider;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&provider, BCRYPT_AES_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	PBYTE buffer[16];
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = 0;
	buffer[5] = 0;
	buffer[6] = 0;
	buffer[7] = 0;
	buffer[8] = 0;
	buffer[9] = 0;
	buffer[10] = 0;
	buffer[11] = 0;
	buffer[12] = 0;
	buffer[13] = 0;
	buffer[14] = 0;
	buffer[15] = 0;
	BCRYPT_KEY_HANDLE key;
	status = BCryptGenerateSymmetricKey(provider, &key, NULL, 0, (PUCHAR) buffer, 16, 0);
	int codeExit = 1;

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptGenerateSymmetricKey");
		goto closeProvider;
	}

	codeExit = 0;
destroyKey:
	status = BCryptDestroyKey(key);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptDestroyKey");
		codeExit = 1;
	}
closeProvider:
	status = BCryptCloseAlgorithmProvider(provider, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptCloseAlgorithmProvider");
		codeExit = 1;
	}

	return codeExit;
}
