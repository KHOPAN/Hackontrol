#include <libkhopan.h>
#include <d3d11.h>

#define CLASS_NAME L"WindowClass"

static LRESULT CALLBACK procedure(_In_ HWND window, _In_ UINT message, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch(message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

int main(int argc, char** argv) {
	HINSTANCE instance = GetModuleHandleW(NULL);
	WNDCLASSW windowClass = {0};
	windowClass.lpfnWndProc = procedure;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	windowClass.lpszClassName = CLASS_NAME;

	if(!RegisterClassW(&windowClass)) {
		KHOPANLASTERRORCONSOLE_WIN32(L"RegisterClassW");
		return 1;
	}

	HWND window = CreateWindowExW(0L, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, instance, NULL);
	int codeExit = 1;

	if(!window) {
		KHOPANLASTERRORCONSOLE_WIN32(L"CreateWindowExW");
		goto unregisterClass;
	}

	D3D_FEATURE_LEVEL levels = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC description;
	memset(&description, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
	description.BufferCount = 1;
	description.BufferDesc.Width = 600;
	description.BufferDesc.Height = 400;
	description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.BufferDesc.RefreshRate.Numerator = 60;
	description.BufferDesc.RefreshRate.Denominator = 1;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.OutputWindow = window;
	description.SampleDesc.Count = 1;
	description.SampleDesc.Quality = 0;
	description.Windowed = TRUE;
	IDXGISwapChain* chain;
	ID3D11Device* device;
	D3D_FEATURE_LEVEL level;
	ID3D11DeviceContext* context;
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, &levels, 1, D3D11_SDK_VERSION, &description, &chain, &device, &level, &context);

	if(FAILED(result)) {
		KHOPANERRORCONSOLE_HRESULT(result, L"D3D11CreateDeviceAndSwapChain");
		goto unregisterClass;
	}

	MSG message;

	while(GetMessageW(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	codeExit = 0;
unregisterClass:
	UnregisterClassW(CLASS_NAME, instance);
	return codeExit;
}
