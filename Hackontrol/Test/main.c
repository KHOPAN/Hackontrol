#include <stdio.h>
#include <d3d9.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	IDirect3D9* direct = Direct3DCreate9(D3D_SDK_VERSION);

	if(!direct) {
		KHWIN32_ERROR_CONSOLE(ERROR_FUNCTION_FAILED, L"Direct3DCreate9");
		return 1;
	}

	D3DDISPLAYMODE mode;
	HRESULT result = direct->lpVtbl->GetAdapterDisplayMode(direct, D3DADAPTER_DEFAULT, &mode);
	int returnValue = 1;

	if(FAILED(result)) {
		KHWIN32_ERROR_CONSOLE(KHHRESULT_DECODE(result), L"IDirect3D9::GetAdapterDisplayMode");
		goto releaseDirect;
	}

	printf("Width: %u Height: %u\n", mode.Width, mode.Height);
	D3DPRESENT_PARAMETERS parameters = {0};
	parameters.Windowed = TRUE;
	parameters.BackBufferCount = 1;
	parameters.BackBufferWidth = mode.Width;
	parameters.BackBufferHeight = mode.Height;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	IDirect3DDevice9* device;
	result = direct->lpVtbl->CreateDevice(direct, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &parameters, &device);

	if(FAILED(result)) {
		KHWIN32_ERROR_CONSOLE(KHHRESULT_DECODE(result), L"IDirect3D9::CreateDevice");
		goto releaseDirect;
	}

	IDirect3DSurface9* surface;
	result = device->lpVtbl->CreateOffscreenPlainSurface(device, mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, NULL);

	if(FAILED(result)) {
		KHWIN32_ERROR_CONSOLE(KHHRESULT_DECODE(result), L"IDirect3DDevice9::CreateOffscreenPlainSurface");
		goto releaseDevice;
	}

	D3DLOCKED_RECT bounds;
	result = surface->lpVtbl->LockRect(surface, &bounds, NULL, 0);

	if(FAILED(result)) {
		KHWIN32_ERROR_CONSOLE(KHHRESULT_DECODE(result), L"IDirect3DSurface9::LockRect");
		goto releaseSurface;
	}

	printf("Pitch: %d\n", bounds.Pitch);
	result = surface->lpVtbl->UnlockRect(surface);

	if(FAILED(result)) {
		KHWIN32_ERROR_CONSOLE(KHHRESULT_DECODE(result), L"IDirect3DSurface9::UnlockRect");
		goto releaseSurface;
	}

	returnValue = 0;
releaseSurface:
	surface->lpVtbl->Release(surface);
releaseDevice:
	device->lpVtbl->Release(device);
releaseDirect:
	direct->lpVtbl->Release(direct);
	return returnValue;
}
