#include <iostream>

extern "C" {
    double provaIf(double,double);
}

int main() {
    double x, y;
    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;
	std::cout << "Inserisci il valore di y: ";
    std::cin >> y;
    std::cout << "provaIf(" << x <<" "<< y <<") = " << provaIf(x,y) << std::endl;
}
