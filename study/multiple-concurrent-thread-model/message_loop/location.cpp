#include "location.h"

#include <intrin.h>
extern "C" {
    void* _ReturnAddress();
}

namespace mctm
{
    Location::Location(const char* function_name, 
        const char* file_name, 
        int line_number, 
        const void* program_counter)
        : function_name_(function_name)
        , file_name_(file_name)
        , line_number_(line_number)
        , program_counter_(program_counter)
    {
    }

    Location::~Location()
    {
    }

    std::string Location::ToString() const
    {
        return std::string(function_name_) + "@" + file_name_ + ":" +
            std::to_string(line_number_);
    }

    const void* GetProgramCounter()
    {
        return _ReturnAddress();
    }

}
