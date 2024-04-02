#include <stdio.h>
#include <omp.h>

int main() {
    int sum = 0;

    // 设置线程数量为4
    omp_set_num_threads(4);

    // 使用OpenMP并行计算1到10的平方和
    #pragma omp parallel for reduction(+:sum)
    for(int i = 1; i <= 100; i++) {
        sum += i * i;
    }

    printf("1到10的平方和: %d\n", sum);

    return 0;
}
