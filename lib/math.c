#include<math.h>

double fmin(double x, double y)
{
    if (x<y) {
        return x;
    } else {
        return y;
    }
}

float fminf(float x, float y)
{
    if (x<y) {
        return x;
    } else {
        return y;
    }
}