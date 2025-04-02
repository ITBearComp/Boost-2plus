#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <random>

const int ARRAY_SIZE = 10000000; // Размер массива
std::vector<long long> data(ARRAY_SIZE); // Массив данных

// Функция для инициализации массива случайными числами
void initialize_array() {
    std::mt19937 rng(0); // Генератор случайных чисел
    std::uniform_int_distribution<long long> dist(1, 100);
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = dist(rng);
    }
}

// Суммирование с использованием std::atomic
void sum_atomic(int start, int end, std::atomic<long long>& total) {
    for (int i = start; i < end; ++i) {
        total += data[i];
    }
}

// Суммирование с использованием std::mutex
void sum_mutex(int start, int end, long long& total, std::mutex& mtx) {
    long long local_sum = 0;
    for (int i = start; i < end; ++i) {
        local_sum += data[i];
    }
    std::lock_guard<std::mutex> lock(mtx);
    total += local_sum;
}

// Суммирование без синхронизации
void sum_no_sync(int start, int end, long long& total) {
    for (int i = start; i < end; ++i) {
        total += data[i];
    }
}

int main() {
    initialize_array(); // Инициализация массива

    for (int num_threads : {2, 4, 8}) {
        std::cout << "Num threads: " << num_threads << std::endl;

        // Суммирование с std::atomic
        std::atomic<long long> atomic_total(0);
        auto start_atomic = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> atomic_threads;
        for (int i = 0; i < num_threads; ++i) {
            int start = i * (ARRAY_SIZE / num_threads);
            int end = (i + 1) * (ARRAY_SIZE / num_threads);
            atomic_threads.emplace_back(sum_atomic, start, end, std::ref(atomic_total));
        }
        for (auto& thread : atomic_threads) {
            thread.join();
        }
        auto end_atomic = std::chrono::high_resolution_clock::now();
        auto duration_atomic = std::chrono::duration_cast<std::chrono::milliseconds>(end_atomic - start_atomic).count();
        std::cout << "Summ (std::atomic): " << atomic_total.load() << ", Time: " << duration_atomic << " ms" << std::endl;

        // Суммирование с std::mutex
        long long mutex_total = 0;
        std::mutex mtx;
        auto start_mutex = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> mutex_threads;
        for (int i = 0; i < num_threads; ++i) {
            int start = i * (ARRAY_SIZE / num_threads);
            int end = (i + 1) * (ARRAY_SIZE / num_threads);
            mutex_threads.emplace_back(sum_mutex, start, end, std::ref(mutex_total), std::ref(mtx));
        }
        for (auto& thread : mutex_threads) {
            thread.join();
        }
        auto end_mutex = std::chrono::high_resolution_clock::now();
        auto duration_mutex = std::chrono::duration_cast<std::chrono::milliseconds>(end_mutex - start_mutex).count();
        std::cout << "Summ (std::mutex): " << mutex_total << ", Time: " << duration_mutex << " ms" << std::endl;

        // Суммирование без синхронизации
        long long no_sync_total = 0;
        auto start_no_sync = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> no_sync_threads;
        for (int i = 0; i < num_threads; ++i) {
            int start = i * (ARRAY_SIZE / num_threads);
            int end = (i + 1) * (ARRAY_SIZE / num_threads);
            no_sync_threads.emplace_back(sum_no_sync, start, end, std::ref(no_sync_total));
        }
        for (auto& thread : no_sync_threads) {
            thread.join();
        }
        auto end_no_sync = std::chrono::high_resolution_clock::now();
        auto duration_no_sync = std::chrono::duration_cast<std::chrono::milliseconds>(end_no_sync - start_no_sync).count();
        std::cout << "Summ (without synch): " << no_sync_total << ", Time: " << duration_no_sync << " ms" << std::endl;

        std::cout << "----------------------------------------" << std::endl;
    }

    return 0;
}