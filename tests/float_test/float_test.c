#include <stdio.h>

double float_test(double n) {
    double sum = 0;   
    // float perforation_rate = 0.2; //isso é obtido pelo pass, essa variavel nao precisa ser acrescentada ao codigo
    // //float y = (1 - perforation_rate); //essa linha é o que precisa ser acrescentado no LLVM IR
    // float k = (y*n);
    //int h = (int)k

    // o valor antigo do limite era "i < n"
    //o problema está na comparação entre i e h, ambos precisam ser do mesmo tipo
    for (int i = 0; i < 100; i++) {
       sum += (i*1.67);
    }
    int k = (int)sum;
    return k;
}

int main(int argc, char const *argv[]) {
    printf("%f\n", float_test(25.5));
}