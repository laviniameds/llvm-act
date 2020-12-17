#include <stdio.h>
#include <math.h>

float error_ratio = 1;

float error(float standard, float perforated){
    if(standard != 0)
        error_ratio = (standard - perforated)/standard;

    return error_ratio;
}

int main(int argc, char const *argv[]) {
    
}