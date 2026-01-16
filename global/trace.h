#ifndef TRACE_H
#define TRACE_H

#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#ifdef _MSC_VER
#include <process.h>
#endif
#ifdef __GNUC__
#include <unistd.h>
#endif

#ifndef ENABLE_TRACE
/**
 * @brief Enable the trace macro by setting to a non-zero value.
 */
#define ENABLE_TRACE 0
#endif

/**
* @brief Return the current time as a string.
* @return The current time.
*/
inline std::string GetTimestamp()
{
    const auto current_time_point {std::chrono::system_clock::now()};
    const auto current_time {std::chrono::system_clock::to_time_t(current_time_point)};
    const auto current_localtime {*std::localtime (&current_time)};
    const auto current_time_since_epoch {current_time_point.time_since_epoch()};
    const auto current_milliseconds {std::chrono::duration_cast<std::chrono::milliseconds> (current_time_since_epoch).count() % 1000};

    std::ostringstream stream;
#ifdef _MSC_VER
    stream << "PID#" << std::dec << _getpid() << " " << std::put_time(&current_localtime, "%H:%M:%S") << "." << std::setw(3) <<
        std::setfill('0') << current_milliseconds << ": ";
#elif defined __GNUC__
    stream << "PID#" << std::dec << getpid() << " " << std::put_time(&current_localtime, "%H:%M:%S") << "." << std::setw(3) <<
        std::setfill('0') << current_milliseconds << ": ";
#else
#error The current OS is not supported!
#endif
    return stream.str();
}

#include <cstring>
#include <iostream>

#if ENABLE_TRACE != 0

#ifdef _MSC_VER
#define TRACE(...) Trace(GetTimestamp(), __FUNCTION__, ": ", __VA_ARGS__)
#elif defined __GNUC__
#define TRACE(...) Trace(GetTimestamp(), __PRETTY_FUNCTION__, ": ", ##__VA_ARGS__)
#else
#error Other compiler are not supported.
#endif

inline void Trace(std::stringstream& /*rsstream*/)
{}

template <typename TArg, typename... TArgs>
inline void Trace(std::stringstream& rsstream, TArg tArg, TArgs... tArgs)
{
    rsstream << tArg;
    Trace(rsstream, tArgs...);
}

template <typename... TArgs>
inline void Trace(TArgs... tArgs)
{
    std::stringstream sstream;
    Trace(sstream, tArgs...);
    sstream << std::endl;
    std::cout << sstream.str();
}

#else // ENABLE_TRACE == 0
template <typename... TArgs>
inline void Trace(TArgs...)
{}

#ifdef __GNUC__
#endif
#define TRACE(...) Trace()
#endif

#endif // !defined TRACE_H