#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "../log_lib/include/logger.hpp"


TEST(LoggerExtraTest, LogMessageAtEqualLevel) {
    std::string testFile = "/tmp/test_log_equal.txt";
    std::remove(testFile.c_str());
    Logger logger(testFile, "Low");

    std::string msg = "Equal level message";
    logger.log(msg, LogLevel::Low);

    std::ifstream in(testFile);
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    in.close();

    ASSERT_NE(content.find(msg), std::string::npos)
        << "Message at equal level should be logged";
}

TEST(LoggerExtraTest, LogMessageBelowCurrentLevel) {
    std::string testFile = "/tmp/test_log_below.txt";
    std::remove(testFile.c_str());
    Logger logger(testFile, "Mid");

    std::string msgLow = "Low level message not logged";
    logger.log(msgLow, LogLevel::Low);

    std::ifstream in(testFile);
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    in.close();

    ASSERT_EQ(content.find(msgLow), std::string::npos)
        << "Message below current level should not be logged";
}

TEST(LoggerExtraTest, LogMessageAboveCurrentLevel) {
    std::string testFile = "/tmp/test_log_above.txt";
    std::remove(testFile.c_str());
    Logger logger(testFile, "Mid");

    std::string msgHigh = "High level message logged";
    logger.log(msgHigh, LogLevel::High);

    std::ifstream in(testFile);
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    in.close();

    ASSERT_NE(content.find(msgHigh), std::string::npos)
        << "Message above current level should be logged";
}

TEST(LoggerExtraTest, SetInvalidLogLevelDoesNotChange) {
    std::string testFile = "/tmp/test_log_invalid.txt";
    std::remove(testFile.c_str());
    Logger logger(testFile, "Low");

    std::string invalidLevel = "INVALID";
    logger.setLogLevel(invalidLevel);

    std::string msg = "Message at Low level after invalid set";
    logger.log(msg, LogLevel::Low);

    std::ifstream in(testFile);
    std::string content((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    in.close();

    ASSERT_NE(content.find(msg), std::string::npos)
        << "After invalid log level set, logger should retain previous level";
}

TEST(LoggerExtraTest, StringToLevelConversion) {
    std::string levelLow = "Low";
    std::string levelMid = "Mid";
    std::string levelHigh = "High";
    std::string levelCommand = "Level";
    std::string invalid = "UnknownLevel";

    Logger dummyLogger("/tmp/dummy.txt", "Low");

    LogLevel low = dummyLogger.stringToLevel(levelLow);
    LogLevel mid = dummyLogger.stringToLevel(levelMid);
    LogLevel high = dummyLogger.stringToLevel(levelHigh);
    LogLevel command = dummyLogger.stringToLevel(levelCommand);
    LogLevel unk = dummyLogger.stringToLevel(invalid);

    ASSERT_EQ(low, LogLevel::Low);
    ASSERT_EQ(mid, LogLevel::Mid);
    ASSERT_EQ(high, LogLevel::High);
    ASSERT_EQ(command, LogLevel::Level);
    ASSERT_EQ(unk, LogLevel::Unknown);
}