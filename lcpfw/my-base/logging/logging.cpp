#include "logging.h"
#include <debugapi.h>
#include <iomanip>
#include <processthreadsapi.h>
#include <time.h>

#include "strings/string_util.h"

#define input_to_string(enum_value) #enum_value
#define input_to_wstring(enum_value) L#enum_value

namespace
{
    const char* const log_severity_names[mctm::LOG_NUM_SEVERITIES] = {
        "INFO", "WARNING", "ERROR", "FATAL" 
    };

    bool BeingDebugged()
    {
        return ::IsDebuggerPresent() != 0;
    }

    void BreakDebugger()
    {
        /*if (IsDebugUISuppressed())
            _exit(1);*/
        __debugbreak();
#if defined(NDEBUG)
        _exit(1);
#endif
    }
}

namespace mctm
{
    LogMessage::LogMessage(const char* file, int line, LogSeverity severity)
        : file_(file)
        , line_(line)
        , severity_(severity)
    {
        Init();
    }

    /*LogMessage::LogMessage(const char* file, int line, std::string* result)
    {

    }

    LogMessage::LogMessage(const char* file, int line, LogSeverity severity, std::string* result)
    {

    }*/

    LogMessage::~LogMessage()
    {
        stream_ << std::endl;
        std::string str_newline(stream_.str());

        //////////////////////////////////////////////////////////////////////////

        auto wstr = UTF8ToWide(str_newline);
        ::OutputDebugStringW(wstr.c_str());
        wprintf_s(wstr.c_str());

        if (severity_ == LOG_FATAL)
        {
            if (BeingDebugged())
            {
                BreakDebugger();
            }
        }
    }

    void LogMessage::Init()
    {
        std::string file_path(file_);
        std::string filename = file_path.substr(file_path.rfind('\\') + 1);
        stream_ << '[';
        stream_ << ::GetCurrentProcessId() << ':';
        stream_ << ::GetCurrentThreadId() << ':';

        time_t t = ::time(NULL);
        struct tm local_time = { 0 };
#if _MSC_VER >= 1400
        localtime_s(&local_time, &t);
#else
        localtime_r(&t, &local_time);
#endif
        struct tm* tm_time = &local_time;
        stream_ << std::setfill('0')
            << std::setw(2) << 1 + tm_time->tm_mon
            << std::setw(2) << tm_time->tm_mday
            << '/'
            << std::setw(2) << tm_time->tm_hour
            << std::setw(2) << tm_time->tm_min
            << std::setw(2) << tm_time->tm_sec
            << ':';

        if (severity_ >= 0)
            stream_ << log_severity_names[severity_];
        else
            stream_ << "VERBOSE";

        stream_ << ":" << filename << "(" << line_ << ")] ";
    }

    LogSeverity GetMinLogLevel()
    {
        return LogSeverity::LOG_INFO;
    }

}
