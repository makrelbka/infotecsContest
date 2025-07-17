#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "../log_lib/include/logger.hpp"

TEST(LoggerTest, LogLevelChange) {
    std::string testFile = "/tmp/test_log.txt";
    std::remove(testFile.c_str());
    Logger logger(testFile, "Low");

    std::string msgLow1 = "message with Low level 1";
    logger.log(msgLow1, LogLevel::Low);

    std::string levelHigh = "High";
    logger.setLogLevel(levelHigh);

    std::string msgHigh = "message with High level";
    logger.log(msgHigh, LogLevel::High);

    std::string msgLow2 = "message with Low level 2";
    logger.log(msgLow2, LogLevel::Low);

    std::ifstream in(testFile);
    std::string line;
    bool foundLow1 = false;
    bool foundHigh = false;
    bool foundLow2 = false;

    while (std::getline(in, line)) {
        if (line.find(msgLow1) != std::string::npos) {
            foundLow1 = true;
        }
        if (line.find(msgHigh) != std::string::npos) {
            foundHigh = true;
        }
        if (line.find(msgLow2) != std::string::npos) {
            foundLow2 = true;
        }
    }

    in.close();


    ASSERT_TRUE(foundLow1) << "First low level message should be logged";
    ASSERT_TRUE(foundHigh) << "High level message should be logged";
    ASSERT_FALSE(foundLow2) << "Second low level message should NOT be logged after level change to High";
}
