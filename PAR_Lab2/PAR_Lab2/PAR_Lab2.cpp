#include <iostream>
#include <Windows.h>
#include <vector>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

const int NUM_THREADS = 6;

int calc_straight(const std::vector<int>& matrix) {

    int result = 0;

    for (int nums : matrix) {
        if (nums % 9 == 0) {
            result ^= nums;
        }
    }

    return result;
}

int calc_mutex(const std::vector<int>& matrix) {

    std::mutex mutex;
    int result = 0;

    auto cycle = [&](int begin, int fin) {
        int local = 0;
        for (int i = begin; i < fin; ++i) {
            if (matrix[i] % 9 == 0)
                local ^= matrix[i];
        }

        std::lock_guard<std::mutex> lock(mutex);

        result ^= local;
    };

    std::vector<std::thread> threads;
    int chunk = matrix.size() / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        int begin = i * chunk;
        int fin = (i == NUM_THREADS - 1) ? matrix.size() : begin + chunk;
        threads.emplace_back(cycle, begin, fin);
    }

    for (auto& t : threads) t.join();

    return result;
}

int calc_cas(const std::vector<int>& matrix) {

    std::atomic<int> result{ 0 };

    auto cycle = [&](int begin, int fin) {

        int resultCycle = 0;

        for (int i = begin; i < fin; ++i) {
            if (matrix[i] % 9 == 0) {
                resultCycle ^= matrix[i];
            }
        }

        int expected = result.load();
        int desired;

        do {
            desired = expected ^ resultCycle;
        } while (!result.compare_exchange_weak(expected, desired));
    };

    std::vector<std::thread> threads;
    int chunk = matrix.size() / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        int begin = i * chunk;
        int fin = (i == NUM_THREADS - 1) ? matrix.size() : begin + chunk;
        threads.emplace_back(cycle, begin, fin);
    }

    for (auto& t : threads) t.join();

    return result;
}

std::vector<int> randomMatrix(int size, int min_val = 1, int max_val = 100000) {

    std::vector<int> matrix(size);
    std::mt19937 rng(13);
    std::uniform_int_distribution<int> dist(min_val, max_val);

    for (int& x : matrix) {
        x = dist(rng);
    }

    return matrix;
}

template<typename Func>

double measure_time(Func f, int& result) {

    auto start = std::chrono::high_resolution_clock::now();
    result = f();
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double>(end - start).count();
}

int main() {

    int sizes[] = { 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

    SetConsoleOutputCP(1251);

    for (int size : sizes) {

        std::cout << "\n=== Data size: " << size << " ===\n";
        std::vector<int> data = randomMatrix(size);

        int result = 0;

        double t_seq = measure_time([&]() { return calc_straight(data); }, result);
        std::cout << "Послідовне виконання: " << result << ", Час: " << t_seq << "s\n";

        double t_mutex = measure_time([&]() { return calc_mutex(data); }, result);
        std::cout << "М'ютекс:              " << result << ", Час: " << t_mutex << "s\n";

        double t_atomic = measure_time([&]() { return calc_cas(data); }, result);
        std::cout << "Атомік:               " << result << ", Час: " << t_atomic << "s\n";
    }

    return 0;
}
