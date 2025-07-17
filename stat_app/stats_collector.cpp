#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include <deque>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>


using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

struct Stats {
    unsigned int totalMessages = 0;
    unsigned int low = 0;
    unsigned int mid = 0;
    unsigned int high = 0;
    unsigned int level = 0;
    unsigned int unknown = 0;
    size_t minLength = SIZE_MAX;
    size_t maxLength = 0;
    double totalLength = 0.0; 
    std::deque<TimePoint> allTimestamps; 
};

unsigned int countLastHourMessages(const Stats &stats, TimePoint now) {
    unsigned int count = 0;
    for (auto it = stats.allTimestamps.rbegin(); it != stats.allTimestamps.rend(); ++it) {
        if (std::chrono::duration_cast<std::chrono::seconds>(now - *it).count() <= 3600) {
            ++count;
        } else {
            break; 
        }
    }
    return count;
}

void printStats(const Stats &stats) {
    TimePoint now = Clock::now();
    unsigned int lastHourCount = countLastHourMessages(stats, now);
    double avgLength = stats.totalMessages ? stats.totalLength / stats.totalMessages : 0.0;

    std::cout << "===== Stat =====" << std::endl;
    std::cout << "All messages: " << stats.totalMessages << std::endl;
    std::cout << "Messages by level:" << std::endl;
    std::cout << "  Low: " << stats.low << std::endl;
    std::cout << "  Mid: " << stats.mid << std::endl;
    std::cout << "  High: " << stats.high << std::endl;
    std::cout << "  Level: " << stats.level << std::endl;
    std::cout << "  Unknown: " << stats.unknown << std::endl;
    std::cout << "Messages in the last hour: " << lastHourCount << std::endl;
    std::cout << "Message lengths:" << std::endl;
    std::cout << "  Min: " << (stats.minLength == SIZE_MAX ? 0 : stats.minLength) << std::endl;
    std::cout << "  Max: " << stats.maxLength << std::endl;
    std::cout << "  Avg: " << avgLength << std::endl;
    std::cout << "================" << std::endl;
}

void updateStats(Stats &stats, const std::string &message) {
    stats.totalMessages++;
    size_t len = message.length();
    if(len < stats.minLength) stats.minLength = len;
    if(len > stats.maxLength) stats.maxLength = len;
    stats.totalLength += len;

    if (message.find("Level: Low") != std::string::npos) {
        stats.low++;
    } else if (message.find("Level: Mid") != std::string::npos) {
        stats.mid++;
    } else if (message.find("Level: High") != std::string::npos) {
        stats.high++;
    } else if (message.find("Level: Level") != std::string::npos) {
        stats.level++;
    } else {
        stats.unknown++;
    }

    stats.allTimestamps.push_back(Clock::now());
}

int main(int argc, char* argv[]) {
    if(argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <N> <T_seconds>" << std::endl;
        return 1;
    }
    int port = std::stoi(argv[1]);
    unsigned int N = std::stoi(argv[2]);
    unsigned int T = std::stoi(argv[3]);

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverFd == -1) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(serverFd);
        return 1;
    }

    if(listen(serverFd, 5) < 0) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(serverFd);
        return 1;
    }

    std::cout << "Waiting for client... " << port << std::endl;
    int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        close(serverFd);
        return 1;
    }
    std::cout << "Client connected." << std::endl;

    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    Stats stats;
    auto lastPrintTime = Clock::now();
    bool statsChanged = false;

    char buffer[1024];
    std::string dataAccumulator;

    while(true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientFd, &readfds);

        timeval timeout;
        timeout.tv_sec = T;
        timeout.tv_usec = 0;
        int activity = select(clientFd + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            break;
        }

        if(FD_ISSET(clientFd, &readfds)) {
            ssize_t bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
            if(bytes > 0) {
                buffer[bytes] = '\0';
                dataAccumulator.append(buffer);

                size_t pos = 0;
                while ((pos = dataAccumulator.find('\n')) != std::string::npos) {
                    std::string line = dataAccumulator.substr(0, pos);
                    dataAccumulator.erase(0, pos + 1);

                    std::cout << "Received: " << line << std::endl;
                    updateStats(stats, line);
                    statsChanged = true;

                    if (stats.totalMessages % N == 0) {
                        printStats(stats);
                        lastPrintTime = Clock::now();
                        statsChanged = false;
                    }
                }
            } else if(bytes == 0) {
                std::cout << "Client disconnected." << std::endl;
                break;
            }
        }

        auto now = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastPrintTime).count();
        if(elapsed >= T && statsChanged) {
            printStats(stats);
            lastPrintTime = now;
            statsChanged = false;
        }
    }

    close(clientFd);
    close(serverFd);
    return 0;
}
