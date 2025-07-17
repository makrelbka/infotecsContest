#include "logger.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Logger::Logger(const std::string& target, std::string level)
    : currentLevel_(stringToLevel(level))
{
    if (target.rfind("socket:", 0) == 0) {
        useSocket_ = initSocket(target);
        if (!useSocket_) {
            std::cerr << "Failed to initialize socket logging. Falling back to console output." << std::endl;
        }
    } else {
        logFile_.open(target, std::ios::app);
    }
}

Logger::~Logger() {
    if (useSocket_ && socketFd_ != -1) {
        close(socketFd_);
    }
}

bool Logger::initSocket(const std::string& target) {
    size_t firstColon = target.find(':');
    size_t secondColon = target.find(':', firstColon + 1);
    if (firstColon == std::string::npos || secondColon == std::string::npos) {
        std::cerr << "Invalid socket target format." << std::endl;
        return false;
    }
    
    std::string host = target.substr(firstColon + 1, secondColon - firstColon - 1);
    int port = std::stoi(target.substr(secondColon + 1));
    
    socketFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd_ == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return false;
    }
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if(inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << host << std::endl;
        return false;
    }
    
    if(connect(socketFd_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void Logger::log(std::string& message) {
    log(message, currentLevelStr_);
}

void Logger::log(std::string& message, std::string& level) {
    log(message, stringToLevel(level));
}

void Logger::log(std::string& message, const LogLevel level) {
    trimWhiteSpace(message);
    
    if (level == LogLevel::Level) {
        setLogLevel(message);
        return;
    }
    
    if (level >= currentLevel_) {
        writeMessage(message, level);
        return;
    }
    
    std::cout << "Log level '" << levelToString(level)
              << "' is lower than current level '" << currentLevelStr_
              << "'. Message not logged." << std::endl;
}

void Logger::setLogLevel(std::string& level) {
    std::lock_guard<std::mutex> lock(mutex_);
    LogLevel newLevel = stringToLevel(level);
    if (newLevel == LogLevel::Unknown) {
        std::cerr << "Unknown log level: '" << level << "'. No changes made." << std::endl;
        return;
    }
    currentLevel_ = newLevel;
    currentLevelStr_ = level;
    std::cout << "Log level changed to: " << currentLevelStr_ << std::endl;
    return;
}

void Logger::writeMessage(std::string& message, const LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << "Time: " << getCurrentTime() << ", "
        << "Level: " << levelToString(level) << ", "
        << "Message: '" << message << "'.\n";
    std::string output = oss.str();
    
    if (useSocket_ && socketFd_ != -1) {
        ssize_t sent = send(socketFd_, output.c_str(), output.size(), 0);
        if (sent == -1) {
            std::cerr << "Failed to send log message: " << strerror(errno) << std::endl;
        }
    } else if (logFile_.is_open()) {
        logFile_ << output;
        logFile_.flush();
    } else {
        std::cout << output;
    }
}

std::string Logger::getCurrentTime() {
    std::time_t t = std::time(nullptr);
    std::tm buf{};
    localtime_r(&t, &buf);
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &buf);
    return std::string(timeStr);
}

std::string Logger::levelToString(const LogLevel level) {
    switch (level) {
        case LogLevel::Low: return "Low";
        case LogLevel::Mid: return "Mid";
        case LogLevel::High: return "High";
        case LogLevel::Level: return "Level";
        default: return "Unknown";
    }
}

LogLevel Logger::stringToLevel(std::string& str) {
    trimWhiteSpace(str);
    if (str == "Low") return LogLevel::Low;
    if (str == "Mid") return LogLevel::Mid;
    if (str == "High") return LogLevel::High;
    if (str == "Level") return LogLevel::Level;
    return LogLevel::Unknown;
}

void Logger::trimWhiteSpace(std::string& str) {
    size_t start = str.find_first_not_of(' ');
    if (start == std::string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(' ');
    str = str.substr(start, end - start + 1);
}


LogLevel Logger::getLevel() const {
    return currentLevel_;
}