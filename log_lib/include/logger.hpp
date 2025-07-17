#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    Unknown = -1, 
    Low = 0,
    Mid = 1,
    High = 2,
    Level = 3,
};

class Logger {
public:
    Logger(const std::string& target, std::string level);
    ~Logger();
    void log(std::string& message, const LogLevel level);
    void log(std::string& message, std::string& level);
    void log(std::string& message);
    void setLogLevel(std::string& level);
    LogLevel getLevel() const;
    std::string levelToString(const LogLevel level);
    LogLevel stringToLevel(std::string& level);

private:
    std::string getCurrentTime();
    void writeMessage(std::string& message, const LogLevel level);
    bool initSocket(const std::string& target);
    void trimWhiteSpace(std::string& str);
    std::ofstream logFile_;
    int socketFd_ = -1;
    bool useSocket_ = false;
    std::mutex mutex_;
    LogLevel currentLevel_;
    std::string currentLevelStr_;
    
};

#endif // LOGGER_HPP
