#include <libkhopan.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	BCRYPT_ALG_HANDLE provider;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&provider, BCRYPT_AES_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	status = BCryptCloseAlgorithmProvider(provider, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHOPANERRORMESSAGE_NTSTATUS(status, L"BCryptCloseAlgorithmProvider");
		return 1;
	}

	return 0;
}
