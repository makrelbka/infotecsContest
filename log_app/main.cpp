#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include "../log_lib/include/logger.hpp"

std::queue<std::pair<std::string, LogLevel>> logQueue;
std::mutex queueMutex;
std::condition_variable cv;
bool done = false;

void loggerThread(Logger& logger) {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [] { return !logQueue.empty() || done; });

        while (!logQueue.empty()) {
            auto entry = std::move(logQueue.front());
            logQueue.pop();
            lock.unlock();
            logger.log(entry.first, entry.second);
            lock.lock();
        }

        if (done) break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./log_app <logfile> <LogLevel>\n";
        return 1;
    }
    std::cout << "Starting logger with file: " << argv[1] << " and level: " << argv[2] << std::endl;
    std::cout << "Usage: <LogLevel>: <message>" << std::endl;
    std::cout << "To Change Default Level: Level: <new level>" << std::endl;

    Logger logger(argv[1], argv[2]);
    std::thread t(loggerThread, std::ref(logger));

    std::string line;
    while (std::getline(std::cin, line)) {
        LogLevel level = logger.getLevel(); 
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string levelStr = line.substr(0, pos);
            std::string message = line.substr(pos + 1);
            level = logger.stringToLevel(levelStr);
            line = message;
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            logQueue.emplace(std::move(line), level);
        }
        cv.notify_one();
    }

    done = true;
    cv.notify_all();
    t.join();
    return 0;
}
