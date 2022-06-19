#pragma once
#include <string>

namespace mctm
{
    class Location
    {
    public:
        Location() = default;
        Location(const char* function_name,
            const char* file_name,
            int line_number,
            const void* program_counter);
        ~Location();

        bool operator < (const Location& other) const
        {
            if (line_number_ != other.line_number_)
                return line_number_ < other.line_number_;
            if (file_name_ != other.file_name_)
                return file_name_ < other.file_name_;
            return function_name_ < other.function_name_;
        }

        std::string ToString() const;

        const char* function_name()   const { return function_name_; }
        const char* file_name()       const { return file_name_; }
        int line_number()             const { return line_number_; }
        const void* program_counter() const { return program_counter_; }

    private:
        const char* function_name_ = nullptr;
        const char* file_name_ = nullptr;
        int line_number_ = 0;
        const void* program_counter_ = nullptr;
    };

    const void* GetProgramCounter();
}

// Define a macro to record the current source location.
#define FROM_HERE FROM_HERE_WITH_EXPLICIT_FUNCTION(__FUNCTION__)

#define FROM_HERE_WITH_EXPLICIT_FUNCTION(function_name) \
    mctm::Location(function_name, __FILE__, __LINE__, mctm::GetProgramCounter())