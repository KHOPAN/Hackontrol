#include <libkhopan.h>

int main(int argc, char** argv) {
	LPWSTR message = KHOPANGetErrorMessageHRESULT(ERROR_IPSEC_IKE_NO_PUBLIC_KEY);
	printf("Message: %ws\n", message);

	if(message) {
		LocalFree(message);
	}

	return 0;
}
