#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

const int MATRIX_SIZE = 1000; 
const int NUM_THREADS = 4;     

std::mutex sum_mutex;          // Мьютекс для синхронизации доступа к общей сумме
std::condition_variable cv;     // Условная переменная для ожидания завершения потоков
int completed_threads = 0;      
long long total_sum = 0;        

void sum_matrix_part(const std::vector<std::vector<int>>& matrix, int start_row, int end_row) {
    long long local_sum = 0;

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            local_sum += matrix[i][j];
        }
    }

    {
        std::lock_guard<std::mutex> lock(sum_mutex);
        total_sum += local_sum;
        completed_threads++;
    }

    cv.notify_one();
}

int main() {
    std::vector<std::vector<int>> matrix(MATRIX_SIZE, std::vector<int>(MATRIX_SIZE));
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(1, 100);

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            matrix[i][j] = dist(rng);
        }
    }

    /*std::cout << "Initial Matrix:" << std::endl;
    print_matrix(matrix); 
    */

    // Измерение времени выполнения
    auto start_time = std::chrono::high_resolution_clock::now();

    // Создание и запуск потоков
    std::vector<std::thread> threads;
    int rows_per_thread = MATRIX_SIZE / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        int start_row = i * rows_per_thread;
        int end_row = (i == NUM_THREADS - 1) ? MATRIX_SIZE : start_row + rows_per_thread;
        threads.emplace_back(sum_matrix_part, std::ref(matrix), start_row, end_row);
    }

    // Ожидание завершения всех потоков
    {
        std::unique_lock<std::mutex> lock(sum_mutex);
        cv.wait(lock, [] { return completed_threads == NUM_THREADS; });
    }

    // Ожидание завершения потоков
    for (auto& thread : threads) {
        thread.join();
    }

    // Измерение времени выполнения
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    // Вывод результатов
    std::cout << "Main summ of matrix elementsssssssss: " << total_sum << std::endl;
    std::cout << "Totaly time: " << duration.count() << " seconds" << std::endl;

    return 0;
}