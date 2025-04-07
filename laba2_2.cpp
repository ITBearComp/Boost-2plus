#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Order {
public:
    int id; // Идентификатор заказа
    int priority; // Приоритет заказа (меньшее число - выше приоритет)

    Order(int id, int priority) : id(id), priority(priority) {}
};

// Сравнение для очереди с приоритетами
struct CompareOrder {
    bool operator()(Order const& o1, Order const& o2) {
        return o1.priority > o2.priority; // Меньшее число - выше приоритет
    }
};

class Machine {
public:
    int id; // Идентификатор станка
    bool isOperational; // Состояние станка

    Machine(int id) : id(id), isOperational(true) {}

    void processOrder(Order order) {
        std::cout << "Stank " << id << " obrabatyvaet zakaz " << order.id << " s priorytetom " << order.priority << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Симуляция времени обработки
    }

    void breakDown() {
        isOperational = false;
        std::cout << "Stank " << id << " vyshel iz stroya!" << std::endl;
    }

    void repair() {
        isOperational = true;
        std::cout << "Stank " << id << " otremontirovan!" << std::endl;
    }
};

class ProductionLine {
private:
    std::priority_queue<Order, std::vector<Order>, CompareOrder> orderQueue; // Очередь заказов
    std::vector<Machine> machines; // Вектор станков
    std::mutex mtx; // Мьютекс для синхронизации
    std::condition_variable cv; // Условная переменная для уведомления
    std::atomic<bool> running{true}; // Флаг для управления работой потока

public:
    ProductionLine(int numMachines) {
        for (int i = 0; i < numMachines; ++i) {
            machines.emplace_back(i + 1); // Создание станков
        }
    }

    void addOrder(Order order) {
        std::lock_guard<std::mutex> lock(mtx);
        orderQueue.push(order); // Добавление заказа в очередь
        cv.notify_one(); // Уведомляем о новом заказе
    }

    void processOrders() {
        while (running) { 
            Order currentOrder(0, 0);
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !orderQueue.empty() || !running; }); 

                if (!running && orderQueue.empty()) break;

                if (!orderQueue.empty()) { 
                    currentOrder = orderQueue.top(); 
                    orderQueue.pop(); 
                }
            }

            for (auto& machine : machines) {
                if (machine.isOperational && currentOrder.id != 0) { 
                    machine.processOrder(currentOrder); 
                    break; 
                }
            }
        }
    }

    void startProcessing() {
        for (auto& machine : machines) {
            std::thread([this, &machine]() {
                while (running) { 
                    Order currentOrder(0, 0);
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { return !orderQueue.empty() || !running; });

                        if (!running && orderQueue.empty()) break;

                        if (!orderQueue.empty()) { 
                            currentOrder = orderQueue.top(); 
                            orderQueue.pop(); 
                        }
                    }

                    if (machine.isOperational && currentOrder.id != 0) { 
                        machine.processOrder(currentOrder); 
                    }
                }
            }).detach();
        }
    }

    void breakMachine(int machineId) { 
        if (machineId >= 1 && machineId <= machines.size()) { 
            machines[machineId - 1].breakDown(); 
        }
    }

    void repairMachine(int machineId) { 
        if (machineId >= 1 && machineId <= machines.size()) { 
            machines[machineId - 1].repair(); 
        }
    }

    void stop() { 
        running = false; 
        cv.notify_all(); 
    }
};

int main() {
    ProductionLine productionLine(4); 

   productionLine.startProcessing(); // Запускаем обработку заказов

   // Добавляем заказы
   productionLine.addOrder(Order(1, 2));
   productionLine.addOrder(Order(2, 1));
   productionLine.addOrder(Order(3, 3));
   productionLine.addOrder(Order(4, 1));
   productionLine.addOrder(Order(5, 3));
   productionLine.addOrder(Order(6, 3));
   productionLine.addOrder(Order(7, 3));
   productionLine.addOrder(Order(8, 1));

   // Симуляция выхода из строя станка
   //std::this_thread::sleep_for(std::chrono::seconds(2));
   
   //productionLine.breakMachine(2); 

   // Даем время на обработку заказов
   std::this_thread::sleep_for(std::chrono::seconds(5));

   //productionLine.repairMachine(2); 

   productionLine.stop(); 

   return 0;
}