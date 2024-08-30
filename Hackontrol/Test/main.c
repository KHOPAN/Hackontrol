#include <stdio.h>
#include <khopanwin32.h>
//#include <ntstatus.h>
//#include <bcrypt.h>

int main(int argc, char** argv) {
	KHWIN32_ERROR(ERROR_OUTOFMEMORY, L"BCryptOpenAlgorithmProvider");
	/*BCRYPT_ALG_HANDLE handle;
	NTSTATUS status = BCryptOpenAlgorithmProvider(&handle, BCRYPT_RSA_ALGORITHM, NULL, 0);

	if(!BCRYPT_SUCCESS(status)) {
		//printf("Error: %d\n", status);
		KHWIN32_ERROR(ERROR_OUTOFMEMORY, L"BCryptOpenAlgorithmProvider");
		return 1;
	}*/

	return 0;
}
