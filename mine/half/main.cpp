#include <cstdio>
#include "half.h"
using Eigen::half;
int main() {
    float num;
    while (~scanf("%f", &num)) {
        half a(num);
        printf("%f\n", float(a));
    }
    return 0;
}