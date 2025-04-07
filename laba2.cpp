#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

class AccessControlSystem {
public:
    AccessControlSystem(int num_entries) : entries(num_entries), high_priority_count(0) {}

    void employee(int id) {
        std::unique_lock<std::mutex> lock(mtx);

        if (id % 5 == 0) {
            high_priority_count++;
            std::cout << "High-priority employee " << id << " is entering.\n";
            openTurnstile();
        } else {
            std::cout << "Employee " << id << " is waiting.\n";
            cv.wait(lock); // Ожидание открытия турникета
            std::cout << "Employee " << id << " is entering.\n";
        }

        if (high_priority_count > 0) {
            high_priority_count--;
        }

        if (high_priority_count == 0) {
            cv.notify_all(); // Уведомляем всех ожидающих
        }
    }

    void openTurnstile() {
        std::cout << "Turnstile opened for high-priority employee.\n";
        cv.notify_one(); // Уведомление одного ожидающего сотрудника
    }

private:
    int entries;
    int high_priority_count;
    std::mutex mtx;
    std::condition_variable cv;
};

int main() {
    const int num_employees = 5;
    AccessControlSystem access_control(5);

    std::vector<std::thread> threads;

    for (int i = 0; i < num_employees; ++i) {
        threads.push_back(std::thread(&AccessControlSystem::employee, &access_control, i));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All employees have entered the building." << std::endl;

    return 0;
}