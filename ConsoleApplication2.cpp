#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>

std::mutex mtx; // Мьютекс для защиты доступа к общему ресурсу
int prime_count = 0; // Общее количество найденных простых чисел

// Функция для проверки, является ли число простым
bool is_prime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i <= std::sqrt(num); ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

// Функция для поиска простых чисел в заданном диапазоне
void find_primes(int start, int end) {
    int local_count = 0;
    for (int i = start; i <= end; ++i) {
        if (is_prime(i)) {
            local_count++;
        }
    }
    // Защита доступа к общему ресурсу
    std::lock_guard<std::mutex> lock(mtx);
    prime_count += local_count;
}

int main() {
    int N, K;
    std::cout << "Enter count of numbers (N): ";
    std::cin >> N;
    std::cout << "Enter count of threads (K): ";
    std::cin >> K;

    // Однопоточный режим
    auto start_single = std::chrono::high_resolution_clock::now();
    prime_count = 0; // Сброс счетчика
    find_primes(1, N);
    auto end_single = std::chrono::high_resolution_clock::now();
    auto duration_single = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single).count();

    std::cout << "One thread count simple numbers: " << prime_count << std::endl;
    std::cout << "Time: " << duration_single << " ms" << std::endl;

    // Многопоточный режим
    std::vector<std::thread> threads;
    prime_count = 0; // Сброс счетчика
    auto start_multi = std::chrono::high_resolution_clock::now();

    int range_per_thread = N / K;
    for (int i = 0; i < K; ++i) {
        int start = i * range_per_thread + 1;
        int end = (i == K - 1) ? N : (i + 1) * range_per_thread; // Последний поток обрабатывает остаток
        threads.emplace_back(find_primes, start, end);
    }

    // Ожидание завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_multi = std::chrono::high_resolution_clock::now();
    auto duration_multi = std::chrono::duration_cast<std::chrono::milliseconds>(end_multi - start_multi).count();

    std::cout << "Many threads count simple numbers: " << prime_count << std::endl;
    std::cout << "Time: " << duration_multi << " ms" << std::endl;

    return 0;
}