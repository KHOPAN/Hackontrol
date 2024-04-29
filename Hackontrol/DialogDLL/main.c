#include <Windows.h>
#include <omp.h>

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	//MessageBoxW(NULL, L"Hello, world!", L"Dialog", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
    int primes = 0;
    int limit = 10000000;

    #pragma omp parallel for schedule(dynamic) reduction(+ : primes)
    for(int number = 1; number <= limit; number++) { // Stress the processor
        int i = 2;

        while(i <= number) {
            if(number % i != 0) {
                i++;
            }
        }

        if(i == number) {
            primes++;
        }
    }
}
