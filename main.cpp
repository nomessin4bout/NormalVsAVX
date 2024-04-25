#include <iostream>
#include <immintrin.h>
#include <thread>
#include <chrono>
#include <mutex>

#define ITERATIONS 10000000

std::mutex mtx;

void normalOperation(float* a, float* b, char operation, int size) {
    std::unique_lock<std::mutex> lock(mtx);
    auto start = std::chrono::high_resolution_clock::now();

    for (int j = 0; j < ITERATIONS; ++j) {
        for (int i = 0; i < size; ++i) {
            switch (operation) {
                case '+':
                    a[i] += b[i];
                    break;
                case '-':
                    a[i] -= b[i];
                    break;
                case '*':
                    a[i] *= b[i];
                    break;
                case '/':
                    a[i] /= b[i];
                    break;
                default:
                    std::cerr << "Invalid operator." << std::endl;
                    return;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Regular calculation took " << duration.count() << "ms.\n";
    lock.unlock();
}

void avxOperation(float* a, float* b, char operation, int size) {
    std::unique_lock<std::mutex> lock(mtx);
    auto start = std::chrono::high_resolution_clock::now();

    int i = 0;
    for (int j = 0; j < ITERATIONS; ++j) {
        for (i = 0; i < size; i += 8) {
            __m256 va = _mm256_loadu_ps(&a[i]);
            __m256 vb = _mm256_loadu_ps(&b[i]);
            switch (operation) {
                case '+':
                    va = _mm256_add_ps(va, vb);
                    break;
                case '-':
                    va = _mm256_sub_ps(va, vb);
                    break;
                case '*':
                    va = _mm256_mul_ps(va, vb);
                    break;
                case '/':
                    va = _mm256_div_ps(va, vb);
                    break;
                default:
                    std::cerr << "Invalid operator." << std::endl;
                    return;
            }
            _mm256_storeu_ps(&a[i], va);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "AVX calculation took " << duration.count() << "ms.\n";
    lock.unlock();
}

int main() {
    std::cout << "Calculating using " << ITERATIONS << " iterations...\n";
    constexpr int size = 256;
    float a[size], b[size];
    std::fill_n(a, size, 1.0f);
    std::fill_n(b, size, 2.0f);
    char operation = '*';

    std::thread normalThread(normalOperation, a, b, operation, size);
    std::thread avxThread(avxOperation, a, b, operation, size);

    normalThread.join();
    avxThread.join();

    return 0;
}

