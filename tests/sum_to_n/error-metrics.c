#include <stdio.h>
#include <math.h>

float error_ratio = 1;

float error(float standard, float perforated){
    if(standard != 0)
        error_ratio = fabs(standard - perforated)/standard;
    return error_ratio;
}

int main(int argc, char const *argv[]) {
    float standard, perforated;
    scanf("%f%f", &standard, &perforated);
    printf("%.2f\n", error(standard, perforated));
}