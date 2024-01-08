#include <iostream>

extern "C" {
    double provaArray();
}

int main() {
    // double x;
    // std::cout << "Inserisci il valore di x: ";
    // std::cin >> x;
    std::cout << "provaArray() = " << provaArray() << std::endl;
}
