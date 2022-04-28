/*
   sLogging, v0.1

   Do this:
	  #define SEMPER_LOGGING_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define SEMPER_LOGGING_IMPLEMENTATION
   #include "sLogging.h"

 ============================    Contributors    =========================
    Jonathan Hoffstadt

 ============================    Documentation    =========================
      
  - Startup/shutdown
    * Call "s_create_logging_context()" before using the API.
    * Call "s_destroy_logging_context()" when finished with API.
  
  - Channel Settings
    * Use "s_set_channel_level(ID, LEVEL)" to set a channel's level.
    * Use "s_set_channel_name(ID, NAME)" to assign a name to a channel.
    * Styles can be adjusted with the macros in "sConfig.h 

  - Logging (to last channel in the logger stack)
    * s_log_trace(const char* format, ...)
    * s_log_debug(const char* format, ...)
    * s_log_info(const char* format, ...)
    * s_log_warn(const char* format, ...)
    * s_log_error(const char* format, ...)
    * s_log_fatal(const char* format, ...)
     
  - Logging (to specific channel)
    * s_log_trace2(ID, const char* format, ...)
    * s_log_debug2(ID, const char* format, ...)
    * s_log_info2(ID, const char* format, ...)
    * s_log_warn2(ID, const char* format, ...)
    * s_log_error2(ID, const char* format, ...)
    * s_log_fatal2(ID, const char* format, ...)  
 
  - Channel Stack
    * use "s_push_channel(ID)" to push a channel onto stack
    * use "s_pop_channel()" to pop channel from stac  
 
  - Levels
    * S_LOG_LEVEL_ALL
    * S_LOG_LEVEL_TRACE
    * S_LOG_LEVEL_DEBUG
    * S_LOG_LEVEL_INFO
    * S_LOG_LEVEL_WARN
    * S_LOG_LEVEL_ERROR
    * S_LOG_LEVEL_FATAL
    * S_LOG_LEVEL_OFF  
 
  - Colors
    * S_LOG_COLOR_DEFAULT
    * S_LOG_COLOR_RED
    * S_LOG_COLOR_GREEN
    * S_LOG_COLOR_YELLOW
    * S_LOG_COLOR_BLUE
    * S_LOG_COLOR_MAGENTA
    * S_LOG_COLOR_CYAN
    * S_LOG_COLOR_WHITE
    * S_LOG_COLOR_BLACK
    * S_LOG_COLOR_STRONG_RED
    * S_LOG_COLOR_STRONG_GREEN
    * S_LOG_COLOR_STRONG_YELLOW
    * S_LOG_COLOR_STRONG_BLUE
    * S_LOG_COLOR_STRONG_MAGENTA
    * S_LOG_COLOR_STRONG_CYAN
    * S_LOG_COLOR_STRONG_WHITE
    * S_LOG_COLOR_STRONG_BLAC  
  
  - Channels (Builtin)
    * S_LOG_CHANNEL_0
    * S_LOG_CHANNEL_1
    * S_LOG_CHANNEL_2
    * S_LOG_CHANNEL_3
    * S_LOG_CHANNEL_4
    * S_LOG_CHANNEL_5
    * S_LOG_CHANNEL_6
    * S_LOG_CHANNEL_7
    * S_LOG_CHANNEL_8
    * S_LOG_CHANNEL_9
    * Use "S_ADDITIONAL_CHANNEL_COUNT" macro in "sConfig.h" to add more  
 
  - Custom Channels
        * create your own macros and use Semper::log_custom(...)

*/

#ifndef SEMPER_LOGGING_H
#define SEMPER_LOGGING_H

#ifndef S_LOGGING_ADDITIONAL_CHANNEL_COUNT
#define S_LOGGING_ADDITIONAL_CHANNEL_COUNT 3
#endif

#ifndef S_LOGGING_MAX_PATH_LENGTH
#define S_LOGGING_MAX_PATH_LENGTH 512
#endif

// includes
#include <cstdio>  // printf
#include <cstdarg> // va_end, va_start
#include <cstring> // strncpy

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// forward declarations
struct sLoggingContext;
struct sLoggingChannel;

// enums/flags
typedef int sLogChannelID;    // -> enum sLogChannelID_
typedef int sLogChannelLevel; // -> enum sLogChannelLevel_
typedef int sLogChannelType;  // -> enum sLogChannelType_
typedef int sLogColor;        // -> enum sLogColor_

#ifndef GSemperLoggingCtx
extern sLoggingContext* GSemperLoggingCtx;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Semper end-user API functions
//-----------------------------------------------------------------------------

namespace Semper
{

    // context creation & access
    void             create_logging_context();
    void             destroy_logging_context();
    sLoggingContext* get_logging_context();

    // misc
    void             set_channel_level(sLogChannelID id, sLogChannelLevel level);
    void             set_channel_name (sLogChannelID id, const char* name);
    void             push_channel     (sLogChannelID id);
    void             pop_channel();

    // logging
    void             log_debug        (const char* fmt, ...);
    void             log_debugv       (const char* fmt, va_list args);
    void             log_debug2       (sLogChannelID id, const char* fmt, ...);
    void             log_trace        (const char* fmt, ...);
    void             log_tracev       (const char* fmt, va_list args);
    void             log_trace2       (sLogChannelID id, const char* fmt, ...);
    void             log_info         (const char* fmt, ...);
    void             log_infov        (const char* fmt, va_list args);
    void             log_info2        (sLogChannelID id, const char* fmt, ...);
    void             log_warn         (const char* fmt, ...);
    void             log_warnv        (const char* fmt, va_list args);
    void             log_warn2        (sLogChannelID id, const char* fmt, ...);
    void             log_error        (const char* fmt, ...);
    void             log_errorv       (const char* fmt, va_list args);
    void             log_error2       (sLogChannelID id, const char* fmt, ...);
    void             log_fatal        (const char* fmt, ...);
    void             log_fatalv       (const char* fmt, va_list args);
    void             log_fatal2       (sLogChannelID id, const char* fmt, ...);
    void             log_custom       (sLogChannelID channel, sLogChannelLevel level, sLogColor fgColor, sLogColor bgColor, bool bold, bool underlined, const char* fmt, ...);
}

//-----------------------------------------------------------------------------
// [SECTION] Enums
//-----------------------------------------------------------------------------

enum sLogChannelType_
{
    S_LOG_CHANNEL_TYPE_UNKNOWN,
    S_LOG_CHANNEL_TYPE_CONSOLE,
    S_LOG_CHANNEL_TYPE_FILE,    // not implemented yet
};

enum sLogChannelLevel_
{
    S_LOG_LEVEL_ALL   = 0,
    S_LOG_LEVEL_TRACE = 5000,
    S_LOG_LEVEL_DEBUG = 6000,
    S_LOG_LEVEL_INFO  = 7000,
    S_LOG_LEVEL_WARN  = 8000,
    S_LOG_LEVEL_ERROR = 9000,
    S_LOG_LEVEL_FATAL = 10000,
    S_LOG_LEVEL_OFF   = 11000,
};

enum sLogColor_
{
    S_LOG_COLOR_DEFAULT,
    S_LOG_COLOR_RED,
    S_LOG_COLOR_GREEN,
    S_LOG_COLOR_YELLOW,
    S_LOG_COLOR_BLUE,
    S_LOG_COLOR_MAGENTA,
    S_LOG_COLOR_CYAN,
    S_LOG_COLOR_WHITE,
    S_LOG_COLOR_BLACK,
    S_LOG_COLOR_STRONG_RED,
    S_LOG_COLOR_STRONG_GREEN,
    S_LOG_COLOR_STRONG_YELLOW,
    S_LOG_COLOR_STRONG_BLUE,
    S_LOG_COLOR_STRONG_MAGENTA,
    S_LOG_COLOR_STRONG_CYAN,
    S_LOG_COLOR_STRONG_WHITE,
    S_LOG_COLOR_STRONG_BLACK,
};

enum sLogChannelID_
{
    S_LOG_CHANNEL_0,
    S_LOG_CHANNEL_1,
    S_LOG_CHANNEL_2,
    S_LOG_CHANNEL_3,
    S_LOG_CHANNEL_4,
    S_LOG_CHANNEL_5,
    S_LOG_CHANNEL_6,
    S_LOG_CHANNEL_7,
    S_LOG_CHANNEL_8,
    S_LOG_CHANNEL_9,
    S_LOG_CHANNEL_COUNT,
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct sLoggingChannel
{
    sLogChannelID    id;
    sLogChannelLevel level;
    sLogChannelType  type;
    char             name[S_LOGGING_MAX_PATH_LENGTH];
    char             path[S_LOGGING_MAX_PATH_LENGTH];
};

struct sLoggingContext
{
    sLoggingChannel channels[S_LOG_CHANNEL_COUNT+S_LOGGING_ADDITIONAL_CHANNEL_COUNT];
    int             channelStack[S_LOG_CHANNEL_COUNT+S_LOGGING_ADDITIONAL_CHANNEL_COUNT];
    int             stackIndex;
    const size_t    numOfChannels = S_LOG_CHANNEL_COUNT+S_LOGGING_ADDITIONAL_CHANNEL_COUNT;
};

#endif // end of header

#ifdef SEMPER_LOGGING_IMPLEMENTATION

#include <assert.h>

#ifndef GSemperLoggingCtx
sLoggingContext* GSemperLoggingCtx = nullptr;
#endif

static void
push_foreground_color_(sLogColor color)
{
    switch(color)
    {
        #if defined(WIN32)
        case S_LOG_COLOR_BLACK:          printf("[30m"); return;
        case S_LOG_COLOR_RED:            printf("[31m"); return;
        case S_LOG_COLOR_GREEN:          printf("[32m"); return;
        case S_LOG_COLOR_YELLOW:         printf("[33m"); return;
        case S_LOG_COLOR_BLUE:           printf("[34m"); return;
        case S_LOG_COLOR_MAGENTA:        printf("[35m"); return;
        case S_LOG_COLOR_CYAN:           printf("[36m"); return;
        case S_LOG_COLOR_WHITE:          printf("[37m"); return;
        case S_LOG_COLOR_STRONG_BLACK:   printf("[90m"); return;
        case S_LOG_COLOR_STRONG_RED:     printf("[91m"); return;
        case S_LOG_COLOR_STRONG_GREEN:   printf("[92m"); return;
        case S_LOG_COLOR_STRONG_YELLOW:  printf("[93m"); return;
        case S_LOG_COLOR_STRONG_BLUE:    printf("[94m"); return;
        case S_LOG_COLOR_STRONG_MAGENTA: printf("[95m"); return;
        case S_LOG_COLOR_STRONG_CYAN:    printf("[96m"); return;
        case S_LOG_COLOR_STRONG_WHITE:   printf("[97m"); return;
        #elif defined(__APPLE__)
        case S_LOG_COLOR_BLACK:          printf("\033[30m"); return;
        case S_LOG_COLOR_RED:            printf("\033[31m"); return;
        case S_LOG_COLOR_GREEN:          printf("\033[32m"); return;
        case S_LOG_COLOR_YELLOW:         printf("\033[33m"); return;
        case S_LOG_COLOR_BLUE:           printf("\033[34m"); return;
        case S_LOG_COLOR_MAGENTA:        printf("\033[35m"); return;
        case S_LOG_COLOR_CYAN:           printf("\033[36m"); return;
        case S_LOG_COLOR_WHITE:          printf("\033[37m"); return;
        case S_LOG_COLOR_STRONG_BLACK:   printf("\033[90m"); return;
        case S_LOG_COLOR_STRONG_RED:     printf("\033[91m"); return;
        case S_LOG_COLOR_STRONG_GREEN:   printf("\033[92m"); return;
        case S_LOG_COLOR_STRONG_YELLOW:  printf("\033[93m"); return;
        case S_LOG_COLOR_STRONG_BLUE:    printf("\033[94m"); return;
        case S_LOG_COLOR_STRONG_MAGENTA: printf("\033[95m"); return;
        case S_LOG_COLOR_STRONG_CYAN:    printf("\033[96m"); return;
        case S_LOG_COLOR_STRONG_WHITE:   printf("\033[97m"); return;
        #endif
    }
}

static void
push_background_color_(sLogColor color)
{
    switch(color)
    {
        #if defined(WIN32)
        case S_LOG_COLOR_BLACK:          printf("[40m"); return;
        case S_LOG_COLOR_RED:            printf("[41m"); return;
        case S_LOG_COLOR_GREEN:          printf("[42m"); return;
        case S_LOG_COLOR_YELLOW:         printf("[43m"); return;
        case S_LOG_COLOR_BLUE:           printf("[44m"); return;
        case S_LOG_COLOR_MAGENTA:        printf("[45m"); return;
        case S_LOG_COLOR_CYAN:           printf("[46m"); return;
        case S_LOG_COLOR_WHITE:          printf("[47m"); return;
        case S_LOG_COLOR_STRONG_BLACK:   printf("[100m"); return;
        case S_LOG_COLOR_STRONG_RED:     printf("[101m"); return;
        case S_LOG_COLOR_STRONG_GREEN:   printf("[102m"); return;
        case S_LOG_COLOR_STRONG_YELLOW:  printf("[103m"); return;
        case S_LOG_COLOR_STRONG_BLUE:    printf("[104m"); return;
        case S_LOG_COLOR_STRONG_MAGENTA: printf("[105m"); return;
        case S_LOG_COLOR_STRONG_CYAN:    printf("[106m"); return;
        case S_LOG_COLOR_STRONG_WHITE:   printf("[107m"); return;
        #elif defined(__APPLE__)
        case S_LOG_COLOR_BLACK:          printf("\033[40m"); return;
        case S_LOG_COLOR_RED:            printf("\033[41m"); return;
        case S_LOG_COLOR_GREEN:          printf("\033[42m"); return;
        case S_LOG_COLOR_YELLOW:         printf("\033[43m"); return;
        case S_LOG_COLOR_BLUE:           printf("\033[44m"); return;
        case S_LOG_COLOR_MAGENTA:        printf("\033[45m"); return;
        case S_LOG_COLOR_CYAN:           printf("\033[46m"); return;
        case S_LOG_COLOR_WHITE:          printf("\033[47m"); return;
        case S_LOG_COLOR_STRONG_BLACK:   printf("\033[100m"); return;
        case S_LOG_COLOR_STRONG_RED:     printf("\033[101m"); return;
        case S_LOG_COLOR_STRONG_GREEN:   printf("\033[102m"); return;
        case S_LOG_COLOR_STRONG_YELLOW:  printf("\033[103m"); return;
        case S_LOG_COLOR_STRONG_BLUE:    printf("\033[104m"); return;
        case S_LOG_COLOR_STRONG_MAGENTA: printf("\033[105m"); return;
        case S_LOG_COLOR_STRONG_CYAN:    printf("\033[106m"); return;
        case S_LOG_COLOR_STRONG_WHITE:   printf("\033[107m"); return;
        #endif
    }
}

static void
pop_color_()
{
    #if defined(WIN32)
    printf("[0m");
    #elif defined(__APPLE__)
    printf("\033[0m");
    #endif
}

static void
push_bold_()
{
    #if defined(WIN32)
    printf("[1m");
    #elif defined(__APPLE__)
    printf("\033[1m");
    #endif
}

static void
push_underline_()
{
    #if defined(WIN32)
    printf("[4m");
    #elif defined(__APPLE__)
    printf("\033[4m");
    #endif
}

void
Semper::create_logging_context()
{
    if(GSemperLoggingCtx)
    {
        assert(false && "Logging context already created");
        return;
    }

    GSemperLoggingCtx = new sLoggingContext();
    GSemperLoggingCtx->stackIndex = 0u;

    for(int i = 0; i < GSemperLoggingCtx->numOfChannels; i++)
    {       
        GSemperLoggingCtx->channelStack[i] = -1;
        GSemperLoggingCtx->channels[i].level = S_LOG_LEVEL_OFF;
        GSemperLoggingCtx->channels[i].type = S_LOG_CHANNEL_TYPE_CONSOLE;
        GSemperLoggingCtx->channels[i].id = i;
        for(int j = 0; j < S_LOGGING_MAX_PATH_LENGTH; j++)
        {
            GSemperLoggingCtx->channels[i].path[j] = 0;
            GSemperLoggingCtx->channels[i].name[j] = 0;
        }
        sprintf(GSemperLoggingCtx->channels[i].name, "Channel %i", i);
    }
    GSemperLoggingCtx->channelStack[0] = S_LOG_CHANNEL_0;
    GSemperLoggingCtx->stackIndex = 0;
    GSemperLoggingCtx->channels[0].level = S_LOG_LEVEL_INFO;
    set_channel_name(S_LOG_CHANNEL_0, "Default Channel");
    log_info("Semper logging context created");
}

void
Semper::destroy_logging_context()
{
    if (GSemperLoggingCtx)
    {
        delete GSemperLoggingCtx;
        GSemperLoggingCtx = nullptr;
        return;
    }

    assert(false && "Logging Context already destroyed.");
}

sLoggingContext*
Semper::get_logging_context()
{
    return GSemperLoggingCtx;
}

void
Semper::push_channel(sLogChannelID id)
{
    assert(GSemperLoggingCtx && "Logging context not created");
    assert(GSemperLoggingCtx->stackIndex < GSemperLoggingCtx->numOfChannels && "Logger stack full.");

    if(GSemperLoggingCtx->stackIndex >= GSemperLoggingCtx->numOfChannels)
    {
        return;
    }

    GSemperLoggingCtx->stackIndex++;
    GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex] = id;
}

void
Semper::pop_channel()
{
    assert(GSemperLoggingCtx && "Logging context not created");
    assert(GSemperLoggingCtx->stackIndex > -1 && "Logger stack already empty.");
    
    // don't pop default logger
    if(GSemperLoggingCtx->stackIndex < 0)
        return;
    GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex] = -1;
    GSemperLoggingCtx->stackIndex--;
}

void
Semper::set_channel_name(sLogChannelID id, const char* name)
{
    assert(GSemperLoggingCtx && "Logging context not created");
    assert(id < GSemperLoggingCtx->numOfChannels && "Channel outside valid range.");

    if(GSemperLoggingCtx)
    {
        if(id < GSemperLoggingCtx->numOfChannels)
            strncpy(GSemperLoggingCtx->channels[id].name, name, sizeof(GSemperLoggingCtx->channels[id].name)/sizeof(GSemperLoggingCtx->channels[id].name[0]));
    }
}

void
Semper::set_channel_level(sLogChannelID id, sLogChannelLevel level)
{
    assert(GSemperLoggingCtx && "Logging context not created");
    assert(id < GSemperLoggingCtx->numOfChannels && "Channel outside valid range.");
    
    if(GSemperLoggingCtx)
    {
        if(id < GSemperLoggingCtx->numOfChannels)
        GSemperLoggingCtx->channels[id].level = level;
    }

}

void
Semper::log_debugv(const char* fmt, va_list args)
{
    assert(GSemperLoggingCtx && "Logging context not created");

    if(S_LOG_LEVEL_DEBUG < GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].level)
        return;

    #ifdef S_LOG_DEBUG_BOLD
    push_bold_();
    #endif
    #ifdef S_LOG_DEBUG_UNDERLINE
    push_underline_();
    #endif
    #ifdef S_LOG_DEBUG_FOREGROUND_COLOR
    push_foreground_color_(S_LOG_DEBUG_FOREGROUND_COLOR);
    #endif
    #ifdef S_LOG_DEBUG_BACKGROUND_COLOR
    push_background_color_(S_LOG_DEBUG_BACKGROUND_COLOR);
    #endif
    printf("[%s][DEBUG]        ", GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].name);
    char dest[1024 * 16];
    vsprintf(dest, fmt, args);
    printf("%s", dest);
    printf("\n");
    pop_color_();
}

void
Semper::log_tracev(const char* fmt, va_list args)
{
    assert(GSemperLoggingCtx && "Logging context not created");

    if(S_LOG_LEVEL_TRACE < GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].level)
        return;

    #ifdef S_LOG_TRACE_BOLD
    push_bold_();
    #endif
    #ifdef S_LOG_TRACE_UNDERLINE
    push_underline_();
    #endif
    #ifdef S_LOG_TRACE_FOREGROUND_COLOR
    push_foreground_color_(S_LOG_TRACE_FOREGROUND_COLOR);
    #endif
    #ifdef S_LOG_DEBUG_LOW_BACKGROUND_COLOR
    push_background_color_(S_LOG_TRACE_BACKGROUND_COLOR);
    #endif
    printf("[%s][TRACE]        ", GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].name);
    char dest[1024 * 16];
    vsprintf(dest, fmt, args);
    printf("%s", dest);
    printf("\n");
    pop_color_();
}

void
Semper::log_infov(const char* fmt, va_list args)
{
    assert(GSemperLoggingCtx && "Logging context not created");

    if(S_LOG_LEVEL_INFO < GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].level)
        return;

    #ifdef S_LOG_INFO_BOLD
    push_bold_();
    #endif
    #ifdef S_LOG_INFO_UNDERLINE
    push_underline_();
    #endif
    #ifdef S_LOG_INFO_FOREGROUND_COLOR
    push_foreground_color_(S_LOG_INFO_FOREGROUND_COLOR);
    #endif
    #ifdef S_LOG_INFO_BACKGROUND_COLOR
    push_background_color_(S_LOG_INFO_BACKGROUND_COLOR);
    #endif
    printf("[%s][INFO]         ", GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].name);
    char dest[1024 * 16];
    vsprintf(dest, fmt, args);
    printf("%s", dest);
    printf("\n");
    pop_color_();
}

void
Semper::log_warnv(const char* fmt, va_list args)
{
    assert(GSemperLoggingCtx && "Logging context not created");

    if(S_LOG_LEVEL_WARN < GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].level)
        return;

    #ifdef S_LOG_WARN_BOLD
    push_bold_();
    #endif
    #ifdef S_LOG_WARN_UNDERLINE
    push_underline_();
    #endif
    #ifdef S_LOG_WARN_FOREGROUND_COLOR
    push_foreground_color_(S_LOG_WARN_FOREGROUND_COLOR);
    #endif
    #ifdef S_LOG_WARN_BACKGROUND_COLOR
    push_background_color_(S_LOG_WARN_BACKGROUND_COLOR);
    #endif
    printf("[%s][WARN]         ", GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].name);
    char dest[1024 * 16];
    vsprintf(dest, fmt, args);
    printf("%s", dest);
    printf("\n");
    pop_color_();
}

void
Semper::log_errorv(const char* fmt, va_list args)
{
    assert(GSemperLoggingCtx && "Logging context not created");

    if(S_LOG_LEVEL_ERROR < GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].level)
        return;

    #ifdef S_LOG_ERROR_BOLD
    push_bold_();
    #endif
    #ifdef S_LOG_ERROR_UNDERLINE
    push_underline_();
    #endif
    #ifdef S_LOG_ERROR_FOREGROUND_COLOR
    push_foreground_color_(S_LOG_ERROR_FOREGROUND_COLOR);
    #endif
    #ifdef S_LOG_ERROR_BACKGROUND_COLOR
    push_background_color_(S_LOG_ERROR_BACKGROUND_COLOR);
    #endif
    printf("[%s][ERROR]        ", GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].name);
    char dest[1024 * 16];
    vsprintf(dest, fmt, args);
    printf("%s", dest);
    printf("\n");
    pop_color_();
}

void
Semper::log_fatalv(const char* fmt, va_list args)
{
    assert(GSemperLoggingCtx && "Logging context not created");

    if(S_LOG_LEVEL_FATAL < GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].level)
        return;

    #ifdef S_LOG_FATAL_BOLD
    push_bold_();
    #endif
    #ifdef S_LOG_FATAL_UNDERLINE
    push_underline_();
    #endif
    #ifdef S_LOG_FATAL_BACKGROUND_COLOR
    push_background_color_(S_LOG_FATAL_BACKGROUND_COLOR);
    #endif
    #ifdef S_LOG_FATAL_FOREGROUND_COLOR
    push_foreground_color_(S_LOG_FATAL_FOREGROUND_COLOR);
    #endif
    printf("[%s][FATAL]        ", GSemperLoggingCtx->channels[GSemperLoggingCtx->channelStack[GSemperLoggingCtx->stackIndex]].name);
    char dest[1024 * 16];
    vsprintf(dest, fmt, args);
    printf("%s", dest);
    printf("\n");
    pop_color_();
}

void
Semper::log_debug(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    char dest[1024 * 16];
    vsprintf(dest, fmt, argptr);
    va_end(argptr);
    log_debugv(fmt, argptr);
}

void
Semper::log_trace(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    log_tracev(fmt, argptr);
    va_end(argptr);
}

void
Semper::log_info(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    log_infov(fmt, argptr);
    va_end(argptr);
}

void
Semper::log_warn(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    log_warnv(fmt, argptr);
    va_end(argptr);
}

void
Semper::log_error(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    log_errorv(fmt, argptr);
    va_end(argptr);
}

void
Semper::log_fatal(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    log_fatalv(fmt, argptr);
    va_end(argptr);
}

void
Semper::log_debug2(sLogChannelID id, const char* fmt, ...)
{
    push_channel(id);
    va_list argptr;
    va_start(argptr, fmt);
    log_debug(fmt, argptr);
    va_end(argptr);
    pop_channel();
}

void
Semper::log_trace2(sLogChannelID id, const char* fmt, ...)
{
    push_channel(id);
    va_list argptr;
    va_start(argptr, fmt);
    log_trace(fmt, argptr);
    va_end(argptr);
    pop_channel();
}

void
Semper::log_info2(sLogChannelID id, const char* fmt, ...)
{
    push_channel(id);
    va_list argptr;
    va_start(argptr, fmt);
    log_info(fmt, argptr);
    va_end(argptr);
    pop_channel();
}

void
Semper::log_warn2(sLogChannelID id, const char* fmt, ...)
{
    push_channel(id);
    va_list argptr;
    va_start(argptr, fmt);
    log_warn(fmt, argptr);
    va_end(argptr);
    pop_channel();
}

void
Semper::log_error2(sLogChannelID id, const char* fmt, ...)
{
    push_channel(id);
    va_list args;
    va_start(args, fmt);
    log_errorv(fmt, args);
    va_end(args);
    pop_channel();
}

void
Semper::log_fatal2(sLogChannelID id, const char* fmt, ...)
{
    push_channel(id);
    va_list argptr;
    va_start(argptr, fmt);
    log_fatal(fmt, argptr);
    va_end(argptr);
    pop_channel();
}

void
Semper::log_custom(sLogChannelID channel, sLogChannelLevel level, sLogColor fgColor, sLogColor bgColor, bool bold, bool underlined, const char* fmt, ...)
{
    assert(GSemperLoggingCtx && "Logging context not created");
    assert(channel <  GSemperLoggingCtx->numOfChannels && "Channel out of range");

    if(channel >= GSemperLoggingCtx->numOfChannels)
        return;
    if(level < GSemperLoggingCtx->channels[channel].level)
        return;

    push_foreground_color_(fgColor);
    push_background_color_(bgColor);

    if(bold)
    {
        push_bold_();
    }

    if(underlined)
    {
        push_underline_();
    }

    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(dest, fmt, argptr);
    va_end(argptr);
    printf("%s", dest);

    pop_color_();
}

#endif

#if defined(S_GLOBAL_LOG_LEVEL_ALL)
#define S_GLOBAL_LOG_LEVEL 0
#elif defined(S_GLOBAL_LOG_LEVEL_TRACE)
#define S_GLOBAL_LOG_LEVEL 1
#elif defined(S_GLOBAL_LOG_LEVEL_DEBUG)
#define S_GLOBAL_LOG_LEVEL 2
#elif defined(S_GLOBAL_LOG_LEVEL_INFO)
#define S_GLOBAL_LOG_LEVEL 3
#elif defined(S_GLOBAL_LOG_LEVEL_WARN)
#define S_GLOBAL_LOG_LEVEL 4
#elif defined(S_GLOBAL_LOG_LEVEL_ERROR)
#define S_GLOBAL_LOG_LEVEL 5
#elif defined(S_GLOBAL_LOG_LEVEL_FATAL)
#define S_GLOBAL_LOG_LEVEL 6
#elif defined(S_GLOBAL_LOG_LEVEL_OFF)
#define S_GLOBAL_LOG_LEVEL 7
#else
#define S_GLOBAL_LOG_LEVEL 3
#endif

#if defined(S_LOGGING_ON)
#define s_create_logging_context()     Semper::create_logging_context()
#define s_destroy_logging_context()    Semper::destroy_logging_context()
#define s_set_channel_level(ID, LEVEL) Semper::set_channel_level(ID, LEVEL)
#define s_set_channel_name(ID, NAME)   Semper::set_channel_name(ID, NAME)
#define s_push_channel(ID)             Semper::push_channel(ID)
#define s_pop_channel()                Semper::pop_channel()
#else
#define s_create_logging_context()
#define s_destroy_logging_context()
#define s_set_channel_level(ID, LEVEL)
#define s_set_channel_name(ID, NAME)
#define s_push_channel(ID)
#define s_pop_channel()
#undef S_GLOBAL_LOG_LEVEL
#define S_GLOBAL_LOG_LEVEL 100
#endif

#ifndef S_GLOBAL_LOG_LEVEL
#define S_GLOBAL_LOG_LEVEL 0
#endif

#if S_GLOBAL_LOG_LEVEL < 7
#define s_log_fatal(...) Semper::log_fatal(__VA_ARGS__)
#define s_log_fatal2(...) Semper::log_fatal2(__VA_ARGS__)
#endif
#if S_GLOBAL_LOG_LEVEL < 6
#define s_log_error(...) Semper::log_error(__VA_ARGS__)
#define s_log_error2(...) Semper::log_error2(__VA_ARGS__)
#endif
#if S_GLOBAL_LOG_LEVEL < 5
#define s_log_warn(...) Semper::log_warn(__VA_ARGS__)
#define s_log_warn2(...) Semper::log_warn2(__VA_ARGS__)
#endif
#if S_GLOBAL_LOG_LEVEL < 4
#define s_log_info(...) Semper::log_info(__VA_ARGS__)
#define s_log_info2(...) Semper::log_info2(__VA_ARGS__)
#endif
#if S_GLOBAL_LOG_LEVEL < 3
#define s_log_debug(...) Semper::log_debug(__VA_ARGS__)
#define s_log_debug2(...) Semper::log_debug2(__VA_ARGS__)
#endif
#if S_GLOBAL_LOG_LEVEL < 2
#define s_log_trace(...)   Semper::log_trace(__VA_ARGS__)
#define s_log_trace2(...)   Semper::log_trace2(__VA_ARGS__)
#endif

// defaults
#ifndef s_log_debug
#define s_log_debug(...)
#define s_log_debug2(...)
#endif

#ifndef s_log_trace
#define s_log_trace(...)
#define s_log_trace2(...)
#endif

#ifndef s_log_info
#define s_log_info(...)
#define s_log_info2(...)
#endif

#ifndef s_log_warn
#define s_log_warn(...)
#define s_log_warn2(...)
#endif

#ifndef s_log_error
#define s_log_error(...)
#define s_log_error2(...)
#endif

#ifndef s_log_fatal
#define s_log_fatal(...)
#define s_log_fatal2(...)
#endif

#undef S_GLOBAL_LOG_LEVEL
