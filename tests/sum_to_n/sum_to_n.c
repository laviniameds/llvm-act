#include <stdio.h>

float limit;

int sum_to_n(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
       sum += i;
    }
    return sum;
}

int main(int argc, char const *argv[]) {
    scanf("%f", &limit);
    printf("%d\n", sum_to_n(limit));
}