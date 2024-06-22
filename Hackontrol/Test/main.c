#include <khopanwin32.h>
#include <jni.h>

typedef jint(__stdcall* CreateJavaVMFunction) (JavaVM** virtualMachine, void** environment, void* arguments);

int main(int argc, char** argv) {
	HMODULE libraryJava = LoadLibraryW(L"C:\\Program Files\\Java\\jdk-21\\bin\\server\\jvm.dll");

	if(!libraryJava) {
		KHWin32ConsoleErrorW(GetLastError(), L"LoadLibraryW");
		return 1;
	}

	CreateJavaVMFunction createJavaVM = (CreateJavaVMFunction) GetProcAddress(libraryJava, "JNI_CreateJavaVM");
	int returnValue = 1;

	if(!createJavaVM) {
		KHWin32ConsoleErrorW(GetLastError(), L"GetProcAddress");
		goto freeLibraryJava;
	}

	returnValue = 0;
freeLibraryJava:
	if(!FreeLibrary(libraryJava)) {
		KHWin32ConsoleErrorW(GetLastError(), L"LoadLibraryW");
		return 1;
	}

	return returnValue;
}
