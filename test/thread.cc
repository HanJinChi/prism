#include <iostream>
#include <thread>
#include <vector>
#include <numeric> // For std::accumulate

// 计算部分数组的总和
int partialSum(const std::vector<int>& array, int start, int end) {
    return std::accumulate(array.begin() + start, array.begin() + end, 0);
}

int main() {
    const int numThreads = 4; // 线程数量
    const int arraySize = 100; // 数组大小
    const int chunkSize = arraySize / numThreads; // 每个线程处理的数组块大小

    std::vector<int> array(arraySize);
    // 初始化数组
    for (int i = 0; i < arraySize; ++i) {
        array[i] = i + 1;
    }

    std::vector<std::thread> threads(numThreads);
    std::vector<int> partialSums(numThreads);

    // 创建并启动线程
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? arraySize : start + chunkSize;
        threads[i] = std::thread([&, start, end, i]() {
            partialSums[i] = partialSum(array, start, end);
        });
    }

    // 等待线程执行完毕
    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }

    // 计算总和
    int totalSum = std::accumulate(partialSums.begin(), partialSums.end(), 0);

    std::cout << "Total sum: " << totalSum << std::endl;

    return 0;
}
