#pragma once

#include <sstream>
#include <iostream>
#include <string>

#define MLOG_DEBUG  0
#define MLOG_INFO   1
#define MLOG_WARN   2
#define MLOG_ERROR  3
#define MLOG_FATAL  4

#define kDefaultLogLevel MLOG_DEBUG

namespace VideoPlayer {
class LogMessage {
public:
    LogMessage(const char* file, int line, const char* func, int severity)
        :_file(file)
        ,_line(line)
        ,_func(func)
        ,_severity(severity) {}

    ~LogMessage() {
      if (_severity >= kDefaultLogLevel)
        std::cout << _file << "@" << _func << 
          "(" << _line << ") " << _stream.str() << std::endl;
    }

    std::ostringstream &stream() { return _stream; }

private:
    std::string _file;
    std::string _func;
    int _severity;
    int _line;
    std::ostringstream _stream;
};

#define LOG_DEBUG LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_DEBUG).stream()
#define LOG_INFO  LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_INFO).stream()
#define LOG_WARN  LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_WARN).stream()
#define LOG_ERROR LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_ERROR).stream()
#define LOG_FATAL LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_FATAL).stream()
}//namespace