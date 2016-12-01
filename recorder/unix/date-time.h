#ifndef DATETIME_H
#define DATETIME_H

// ansi C //
#include <stdio.h>
#include <time.h>


namespace plugin {

namespace rec {
/// date time utility inline functions
/// \brief The DateTime class
///
class DateTime
{
public:


    static inline long getTimeAsInteger()
    {
        time_t current_time;
        struct tm* time_info;
        time(&current_time);
        time_info = localtime(&current_time);
        return current_time;
    }

    /// get time HH:MM:SS
    /// \brief getTimeString
    /// \return time as str
    ///
    static  inline const char* getTimeString()
    {
        time_t current_time;
        struct tm * time_info;
        static char timeString[9];  // space for "HH:MM:SS\0"

        time(&current_time);
        time_info = localtime(&current_time);
        strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
        return timeString;
    }

    /// get time as format
    /// Day Month  HH:MM:SS YEAR
    /// \brief getDateTime
    /// \return string
    ///
    static  inline const char* getDateTime()
    {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        static char date_time[128] = {0};
        strftime(date_time, sizeof(date_time), "%c", tm);
        return date_time;
    }
};

} // rec
} // plugin

#endif // DATETIME_H
