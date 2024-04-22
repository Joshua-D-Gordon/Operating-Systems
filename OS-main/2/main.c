#include <stdio.h>
#include <math.h>
// need to add a margin of error epsilon for marginal errors?
double alpha = 0.0001;

int main()
{

    printf("enter 3 integers for triangle sides\n");
    double a, b, c;
    scanf("%lf %lf %lf", &a, &b, &c);
    double calculatePitagoras = pow(a, 2) + pow(b, 2) - pow(c, 2);
    if (calculatePitagoras < alpha && calculatePitagoras > -alpha)
    {
        double aa, ba, ca;
        // furmula found on calculator.net/triangle
        aa = acos((pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c));
        ba = acos((pow(a, 2) + pow(c, 2) - pow(b, 2)) / (2 * a * c));
        ca = acos((pow(a, 2) + pow(b, 2) - pow(c, 2)) / (2 * a * b));
        printf("%f %f %f\n", aa, ba, ca);
    }
    else
    {
        printf("\nError\n");
        return 0;
    }
    return 0;
}