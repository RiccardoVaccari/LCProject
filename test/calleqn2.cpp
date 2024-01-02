#include <iostream>

extern "C" {
    double eqn2(double,double);
}

// extern "C" {
//     double printval(double, double, double);
// }

// double printval(double x1, double x2, double flag) {
//     if (x2 == 0) {
//        std::cout << "Due soluzioni reali e coincidenti: x1=x2=" << x1 << std::endl;
//     } else if (flag == 0) {
//        std::cout << "Due soluzioni complesse e coniugate: x1 = " << x1 << "+" << x2 << "i  ";
//        std::cout << "x2 = " << x1 << "-" << x2 << "i\n";
//     } else {
//        std::cout << "Due soluzioni reali e distinte: x1 = " << x1 << ", x2 = " << x2 << std::endl;
//     };
//     return 0.0;
// };

int main() {
    double a,b;
    std::cout << "Inserisci a,b: ";
    std::cin >> a >> b;
    std::cout << eqn2(a,b) << std::endl;
    return 0;
}
