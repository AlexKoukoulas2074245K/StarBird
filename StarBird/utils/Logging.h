///------------------------------------------------------------------------------------------------
///  Logging.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///-----------------------------------------------------------------------------------------------

#ifndef Logging_h
#define Logging_h

///-----------------------------------------------------------------------------------------------

#include <stdarg.h> // va_list, va_start, va_end
#include <stdio.h>  // vprintf, printf
#include <string>

///-----------------------------------------------------------------------------------------------

#define LOG_IN_RELEASE

///-----------------------------------------------------------------------------------------------
/// Different types of logging available
enum class LogType
{
    INFO, WARNING, ERROR
};

///-----------------------------------------------------------------------------------------------
/// Logs a message to the std out, with a custom log type tag \see LogType
/// @param[in] logType the category of logging message 
/// @param[in] message the message itself as a c-string
#if defined(DEBUG) || defined(LOG_IN_RELEASE)
inline void Log(const LogType logType, const char* message, ...)
{

    switch(logType)
    {
        case LogType::INFO: printf("[INFO] "); break;
        case LogType::WARNING: printf("[WARNING] "); break;
        case LogType::ERROR: printf("[ERROR] "); break;
    }
    
    va_list args;
    va_start (args, message);
    vprintf (message, args);
    va_end (args);
    
    printf("\n");
}
#else
inline void Log(const LogType, const char*, ...) {}
#endif /* not NDEBUG */

///-----------------------------------------------------------------------------------------------

#endif /* Logging_h */
