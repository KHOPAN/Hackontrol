#include <stdio.h>
#include <omp.h>

int main(int argc, char** argv) {
	//MessageBoxW(NULL, L"Hello, world!", L"Information", MB_OK | MB_ICONINFORMATION | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
    int primes = 0;
    int limit = 10000000;

    #pragma omp parallel for schedule(dynamic) reduction(+ : primes)
    for(int number = 1; number <= limit; number++) {
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

    printf("Primes: %d\nLimit: %d\n", primes, limit);
	return 0;
}
