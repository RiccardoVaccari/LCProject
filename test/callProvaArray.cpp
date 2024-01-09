#include <iostream>

extern "C" {
    double provaArray(double);
}
extern "C" {
    double provaArray2();
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
    int V[5] = {0,0,0,0,0};
    V[6] = 1;
    std::cout<<V[6]<<std::endl;
    std::cout << "Inserisci il valore di x: ";
    std::cin >> x;
    std::cout << "provaArray() = " << provaArray(x) << std::endl;
    std::cout << "provaArray2() = " << provaArray2() << std::endl;

}
