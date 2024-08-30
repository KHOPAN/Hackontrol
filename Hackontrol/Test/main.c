#include <stdio.h>
#include <khopanwin32.h>
//#include <ntstatus.h>
#include <bcrypt.h>

int main(int argc, char** argv) {
	KHWIN32_ERROR(ERROR_OUTOFMEMORY, L"main");
	BCRYPT_ALG_HANDLE handle;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&handle, L"WRONG", NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		//printf("Error: %d\n", status);BCRYPT_RSA_ALGORITHM
		KHNTSTATUS_ERROR(status, L"BCryptOpenAlgorithmProvider");
		return 1;
	}

	return 0;
}
