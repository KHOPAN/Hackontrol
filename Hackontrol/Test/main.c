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
	returnValue = 0;
releaseDirect:
	direct->lpVtbl->Release(direct);
	return returnValue;
}
