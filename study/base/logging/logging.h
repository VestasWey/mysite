#pragma once
#include <string>
#include <cstring>
#include <sstream>

namespace mctm
{
    enum LogSeverity
    {
#define LOG_LEVEL_DEF(name, lv)\
    name = lv, \
    LOG_##name = lv,
        
        LOG_VERBOSE = -1,

        LOG_LEVEL_DEF(INFO, 0)
        LOG_LEVEL_DEF(WARNING, 1)
        LOG_ERROR = 2,
        LOG_0 = LOG_ERROR,
        LOG_LEVEL_DEF(FATAL, 3)

        LOG_NUM_SEVERITIES = 4,
    };

    class LogMessage
    {
    public:
        // Used for LOG(severity).
        LogMessage(const char* file, int line, LogSeverity severity);

        //// Used for CHECK_EQ(), etc. Takes ownership of the given string.
        //// Implied severity = LOG_FATAL.
        //LogMessage(const char* file, int line, std::string* result);

        //// Used for DCHECK_EQ(), etc. Takes ownership of the given string.
        //LogMessage(const char* file,
        //    int line,
        //    LogSeverity severity,
        //    std::string* result);

        ~LogMessage();

        std::ostream& stream() { return stream_; }

    private:
        void Init();

    private:
        std::ostringstream stream_;
        LogSeverity severity_;

        // The file and line information passed in to the constructor.
        const char* file_ = nullptr;
        const int line_ = 0;
    };

    class LogMessageVoidify
    {
    public:
        LogMessageVoidify() {}
        // This has to be an operator with a precedence lower than << but
        // higher than ?:
        void operator&(std::ostream&) {}
    };

    LogSeverity GetMinLogLevel();
}

#ifndef DCHECK

#define COMPACT_MCTM_LOG_INFO_VA(ClassName, ...)\
    mctm::ClassName(__FILE__, __LINE__, mctm::LogSeverity::LOG_INFO, ##__VA_ARGS__)
#define COMPACT_MCTM_LOG_WARNING_VA(ClassName, ...)\
    mctm::ClassName(__FILE__, __LINE__, mctm::LogSeverity::LOG_WARNING, ##__VA_ARGS__)
#define COMPACT_MCTM_LOG_ERROR_VA(ClassName, ...)\
    mctm::ClassName(__FILE__, __LINE__, mctm::LogSeverity::LOG_ERROR, ##__VA_ARGS__)
#define COMPACT_MCTM_LOG_FATAL_VA(ClassName, ...)\
    mctm::ClassName(__FILE__, __LINE__, mctm::LogSeverity::LOG_FATAL, ##__VA_ARGS__)

#define COMPACT_MCTM_LOG_DCHECK COMPACT_MCTM_LOG_FATAL_VA(LogMessage)
#define COMPACT_MCTM_LOG_INFO COMPACT_MCTM_LOG_INFO_VA(LogMessage)
#define COMPACT_MCTM_LOG_WARNING COMPACT_MCTM_LOG_WARNING_VA(LogMessage)
#define COMPACT_MCTM_LOG_ERROR COMPACT_MCTM_LOG_ERROR_VA(LogMessage)
#define COMPACT_MCTM_LOG_FATAL COMPACT_MCTM_LOG_FATAL_VA(LogMessage)
#define COMPACT_MCTM_LOG_0 COMPACT_MCTM_LOG_ERROR

#define LOG_STREAM(severity) COMPACT_MCTM_LOG_##severity.stream()

#define LAZY_STREAM(stream, condition) \
    !(condition) ? (void)0 : mctm::LogMessageVoidify() & (stream)

#define LOG_IS_ON(severity) \
  ((mctm::LogSeverity::LOG_##severity) >= mctm::GetMinLogLevel())

#define LOG(severity) LAZY_STREAM(LOG_STREAM(severity), LOG_IS_ON(severity))
#define LOG_IF(severity, condition) \
  LAZY_STREAM(LOG_STREAM(severity), LOG_IS_ON(severity) && (condition))

#if defined(_DEBUG) || defined(DEBUG)
#define DCHECK_IS_ON() 1
#define DLOG_IS_ON(severity) LOG_IS_ON(severity)
#define DLOG_IF(severity, condition) LOG_IF(severity, condition)
#else
#define DCHECK_IS_ON() 0
#define DLOG_IS_ON(severity) 0
#endif

// check
#define DCHECK(condition)                                         \
    LAZY_STREAM(LOG_STREAM(DCHECK), DCHECK_IS_ON() && !(condition)) \
    << "Check failed: " #condition ". "

#define NOTREACHED() DCHECK(false)

// log
#define DLOG(severity)                                          \
    LAZY_STREAM(LOG_STREAM(severity), DLOG_IS_ON(severity))

#endif