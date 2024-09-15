#include <d3d9.h>
#include <khopanwin32.h>

int main(int argc, char** argv) {
	IDirect3D9* direct = Direct3DCreate9(D3D_SDK_VERSION);

	if(!direct) {
		KHWIN32_ERROR_CONSOLE(ERROR_FUNCTION_FAILED, L"Direct3DCreate9");
		return 1;
	}

	direct->lpVtbl->Release(direct);
	return 0;
}
