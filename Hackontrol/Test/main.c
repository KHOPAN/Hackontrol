#include <stdio.h>
#include <Windows.h>
#include <khopanerror.h>

int main(int argc, char** argv) {
	LPSTR message = KHGetWin32ErrorMessageA(ERROR_ALL_USER_TRUST_QUOTA_EXCEEDED, "testFunction");
	printf("%s\n", message);
	LocalFree(message);
	return 0;
}
