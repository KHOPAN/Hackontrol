#include <libkhopan.h>
#include <libkhopanlist.h>

int main(int argc, char** argv) {
	LINKEDLIST list;

	if(!KHOPANLinkedInitialize(&list, sizeof(ULONGLONG))) {
		KHOPANLASTERRORMESSAGE_WIN32(L"KHOPANLinkedInitialize");
		return 1;
	}

	return 0;
}
