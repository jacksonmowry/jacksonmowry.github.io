#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    double a, b;
    char c;

    scanf("%lf %c %lf", &a, &c, &b);

    switch (c) {
    case '+':
        printf("%f %c %f = %.3f\n", a, c, b, a + b);
        break;
    case '-':
        printf("%f %c %f = %.3f\n", a, c, b, a - b);
        break;
    case '*':
        printf("%f %c %f = %.3f\n", a, c, b, a * b);
        break;
    case '/':
        printf("%f %c %f = %.3f\n", a, c, b, a / b);
        break;
    case '%':
        printf("%f %c %f = %.3f\n", a, c, b, fmod(a, b));
        break;
    default:
        printf("The operation %c is invalid!\n", c);
    }
}
