#include <stdio.h>
#include <khopanwin32.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	BCRYPT_ALG_HANDLE handle;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&handle, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		KHNTSTATUS_ERROR(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	return 0;
}
