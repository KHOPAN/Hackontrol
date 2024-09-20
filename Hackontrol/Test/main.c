#include <libhackontrolcurl.h>
#include <libkhopancurl.h>

int main(int argc, char** argv) {
	DATASTREAM stream = {0};
	CURLcode code = HackontrolDownload("www.google.com", &stream, FALSE, FALSE);

	if(code != CURLE_OK) {
		KHOPANERRORMESSAGE_CURL(code, L"HackontrolDownload");
		return 1;
	}

	for(size_t i = 0; i < stream.size; i++) {
		putchar(stream.data[i]);
	}

	KHOPANStreamFree(&stream);
	_flushall();
	return 0;
}
