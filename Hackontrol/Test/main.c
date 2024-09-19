#include <libkhopanlist.h>
#include <libkhopan.h>
#include <libkhopancurl.h>
#include <libkhopanjava.h>

int main(int argc, char** argv) {
	KHOPANArrayInitialize(NULL, 0);
	KHOPANArrayAdd(NULL, NULL);
	KHOPANArrayRemove(NULL, 0);
	KHOPANArrayGet(NULL, 0, NULL);
	KHOPANArrayFree(NULL);
	KHOPANStreamAdd(NULL, NULL, 0);
	KHOPANStreamFree(NULL);
	KHOPANEnablePrivilege(NULL);
	KHOPANExecuteCommand(NULL, FALSE);
	KHOPANExecuteDynamicLibrary(NULL, NULL, NULL);
	KHOPANExecuteProcess(NULL, NULL, FALSE);
	KHOPANExecuteRundll32Function(NULL, NULL, NULL, FALSE);
	KHOPANFileGetCmd();
	KHOPANFileGetRundll32();
	KHOPANFolderGetWindows();
	KHOPANFormatMessage(NULL);
	KHOPANInternalGetErrorMessage(0, NULL, FALSE);
	KHOPANStringDuplicate(NULL);
	KHOPANCURLInternalGetErrorMessage(0, NULL);
	KHOPANJavaStandardOutput(NULL, NULL);
	KHOPANJavaStandardError(NULL, NULL);
	KHOPANJavaThrow(NULL, NULL, NULL);
	return 0;
}
