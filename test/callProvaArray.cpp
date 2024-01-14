#include <iostream>

extern "C" {
    double provaArray(double);
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
    std::cout << "provaArray() "<<std::endl<< provaArray(x) << std::endl;

}
