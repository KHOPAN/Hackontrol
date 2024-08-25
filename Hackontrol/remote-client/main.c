#include "hrsp_client.h"

int main(int argc, char** argv) {
	HRSPCLIENTSTRUCT client = {0};
	HRSPConnectToServer("localhost", "42485", &client);
	return 0;
}
