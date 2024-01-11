#include <iostream>

extern "C" {
    double provaFor(double);
}


extern "C" {
    double printval(double);
}

double printval(double x1) {
	std::cout<<x1<<std::endl;
    return 0.0;
};


int main() {
    double x;
    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;
    provaFor(x);
    // std::cout << "provaFor(" << x << ") = " << provaFor(x) << std::endl;
}
