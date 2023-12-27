#include <iostream>

extern "C" {
    double fibo(double);
}

int main() {
    int n;
    std::cout << "Inserisci il valore di n: ";
    std::cin >> n;
    std::cout << "fib(" << n << ") = " << fibo(n) << std::endl;
}
