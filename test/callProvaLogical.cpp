#include <iostream>

extern "C" {
    double provaLog(double);
}


int main() {
    double x;
    std::cout << "Inserisci il valore di n: ";
    std::cin >> x;
    std::cout << "provaFor(" << x << ") = " << provaLog(x) << std::endl;
}
