#ifndef DEBUGGING_INC
#define DEBUGGING_INC

#include <cstdio>
#define FMT_CONSTEVAL
#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include <system_error>

namespace debugging {

bool should_enable_from_start();
void show_error(const char* message);
void enable();
void disable();
bool enabled();

inline void log(const std::string_view message)
{
    fwrite(message.data(), 1, message.size(), stderr);
}

template <typename... Args>
inline void log(const fmt::format_string<Args...>& format, Args&&... args)
{
    if(enabled())
    {
        try {
            fmt::print(stderr, format, args...);
        }
        catch(const std::system_error& sys_err)
        {
            // debugging is disabled
        }
    }
}

}

#endif
