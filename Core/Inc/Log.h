/**
 * @file Log.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef LOG_H
	#define LOG_H
#else
	#error "You cannot use Log.h in header file!"
#endif // LOG_H

#include <stdio.h>

#ifndef LOG_FORCE_FLUSH
	#define LOG_FORCE_FLUSH 0
#endif // LOG_FORCE_FLUSH

#ifndef LOG_TAG
	#define LOG_TAG ""
#endif // LOG_TAG

#ifndef LOG_ENABLE
	#define LOG_ENABLE 1
#endif // LOG_ENABLE


#if LOG_FORCE_FLUSH
	#error "Function 'LOG_FLUSH' not implemented!"
#else // LOG_FORCE_FLUSH
	#define LOG_FLUSH()
#endif // LOG_FORCE_FLUSH

#define LOG_LEVEL_NONE    0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_DEBUG   5
#define LOG_LEVEL_VERBOSE 6

#if LOG_COLOR_ENABLE
	#define LOG_NO_COLOR  "\033[0m"
	#define LOG_DARK_RED  "\033[0;31m"
	#define LOG_LIGHT_RED "\033[1;31m"
	#define LOG_GREEN     "\033[1;32m"
	#define LOG_YELLOW    "\033[1;33m"
	#define LOG_GRAY      "\033[1;30m"
#else
	#define LOG_NO_COLOR  ""
	#define LOG_DARK_RED  ""
	#define LOG_LIGHT_RED ""
	#define LOG_GREEN     ""
	#define LOG_YELLOW    ""
	#define LOG_GRAY      ""
#endif // LOG_COLOR_ENABLE

#ifndef LOG_LEVEL_LIMIT
	#define LOG_LEVEL_LIMIT LOG_LEVEL_VERBOSE
#endif // LOG_LEVEL_LIMIT

#define LOG_WITH_TIMESTAMP	(1)
#ifndef LOG_LEVEL
	#error "Place '#define LOG_LEVEL LOG_LEVEL_xxx' before '#include Log.h' !!"
#endif // LOG_LEVEL

#if !LOG_ENABLE && defined(LOG_LEVEL)
	#undef  LOG_LEVEL
	#define LOG_LEVEL LOG_LEVEL_NONE
#endif

#ifndef LOG_OUTPUT_STREAM
extern FILE* logOutputStream;
#define LOG_OUTPUT_STREAM logOutputStream
#endif

#if (LOG_WITH_TIMESTAMP==1)

/**
 * @brief 
 * 
 */
typedef struct {
    unsigned long hours;
    unsigned long minutes;
    unsigned long seconds;
    unsigned long milliseconds;
} SystemTime;

extern SystemTime get_log_timeStamp(void);
#endif

#if LOG_ENABLE
#include <stdio.h>
#if (LOG_WITH_TIMESTAMP == 1)
#define LOG_PRINTF(...) do { \
    SystemTime time = get_log_timeStamp(); \
    fprintf(LOG_OUTPUT_STREAM, "[%02lu:%02lu:%02lu:%03lu] ", \
        time.hours, time.minutes, time.seconds, time.milliseconds); \
    fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__); \
    fprintf(LOG_OUTPUT_STREAM, LOG_NO_COLOR "\n"); \
} while(0)
#else
#define LOG_PRINTF(...) do { \
    fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__); \
    fprintf(LOG_OUTPUT_STREAM, LOG_NO_COLOR "\n"); \
} while(0)
#endif // LOG_WITH_TIMESTAMP

#endif // LOG_ENABLE


/*
	LOG_XXX      - LOG with printed name and with lock
	LOG_XXX_E    - LOG without printed name and with lock
	LOG_XXX_xLCK - LOG without printed name and without lock => manual locking needed
*/
#if LOG_LEVEL >= LOG_LEVEL_FATAL && LOG_LEVEL_LIMIT >= LOG_LEVEL_FATAL
	#define LOG_FATAL(...)      do{if(LOG_StdoutLock()){LOG_PRINTF(LOG_DARK_RED"[FATAL] "LOG_TAG" "__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_FATAL_E(...)    do{if(LOG_StdoutLock()){LOG_PRINTF(__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_FATAL_xLCK(...) do{LOG_PRINTF(__VA_ARGS__);LOG_FLUSH();} while(0)
#else
	#define LOG_FATAL(...)      do{} while(0)
	#define LOG_FATAL_E(...)    do{} while(0)
	#define LOG_FATAL_xLCK(...) do{} while(0)
#endif // LOG_LEVEL_FATAL

#if LOG_LEVEL >= LOG_LEVEL_ERROR && LOG_LEVEL_LIMIT >= LOG_LEVEL_ERROR
	#define LOG_ERROR(...)      do{if(LOG_StdoutLock()){LOG_PRINTF(LOG_LIGHT_RED"[ERROR] "LOG_TAG" "__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_ERROR_E(...)    do{if(LOG_StdoutLock()){LOG_PRINTF(__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_ERROR_xLCK(...) do{LOG_PRINTF(__VA_ARGS__);LOG_FLUSH();} while(0)
#else
	#define LOG_ERROR(...)       do{} while(0)
	#define LOG_ERROR_E(...)     do{} while(0)
	#define LOG_ERROR_xLCK(...)  do{} while(0)
#endif // LOG_LEVEL_ERROR

#if LOG_LEVEL >= LOG_LEVEL_WARNING && LOG_LEVEL_LIMIT >= LOG_LEVEL_WARNING
	#define LOG_WARNING(...)      do{if(LOG_StdoutLock()){LOG_PRINTF(LOG_YELLOW"[WARN.] "LOG_TAG" "__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_WARNING_E(...)    do{if(LOG_StdoutLock()){LOG_PRINTF(__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_WARNING_xLCK(...) do{LOG_PRINTF(__VA_ARGS__);LOG_FLUSH();} while(0)
#else
	#define LOG_WARNING(...)      do{} while(0)
	#define LOG_WARNING_E(...)    do{} while(0)
	#define LOG_WARNING_xLCK(...) do{} while(0)
#endif // LOG_LEVEL_WARNING

#if LOG_LEVEL >= LOG_LEVEL_INFO && LOG_LEVEL_LIMIT >= LOG_LEVEL_INFO
	#define LOG_INFO(...)      do{if(LOG_StdoutLock()){LOG_PRINTF(LOG_GREEN"[INFO ] "LOG_TAG" "__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_INFO_E(...)    do{if(LOG_StdoutLock()){LOG_PRINTF(__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_INFO_xLCK(...) do{LOG_PRINTF(__VA_ARGS__);LOG_FLUSH();} while(0)
#else
	#define LOG_INFO(...)      do{} while(0)
	#define LOG_INFO_E(...)    do{} while(0)
	#define LOG_INFO_xLCK(...) do{} while(0)
#endif // LOG_LEVEL_INFO

#if LOG_LEVEL >= LOG_LEVEL_DEBUG && LOG_LEVEL_LIMIT >= LOG_LEVEL_DEBUG
	#define LOG_DEBUG(...)       do{if(LOG_StdoutLock()){LOG_PRINTF(LOG_NO_COLOR"[DEBUG] "LOG_TAG" "__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_DEBUG_E(...)     do{if(LOG_StdoutLock()){LOG_PRINTF(__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_DEBUG_xLCK(...)  do{LOG_PRINTF(__VA_ARGS__);LOG_FLUSH();} while(0)
#else
	#define LOG_DEBUG(...)       do{} while(0)
	#define LOG_DEBUG_E(...)     do{} while(0)
	#define LOG_DEBUG_xLCK(...)  do{} while(0)
#endif // LOG_LEVEL_DEBUG

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE && LOG_LEVEL_LIMIT >= LOG_LEVEL_VERBOSE
	#define LOG_VERBOSE(...)       do{if(LOG_StdoutLock()){LOG_PRINTF(LOG_GRAY"[VERB.] "LOG_TAG" "__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_VERBOSE_E(...)     do{if(LOG_StdoutLock()){LOG_PRINTF(__VA_ARGS__);LOG_StdoutUnlock();LOG_FLUSH();}} while(0)
	#define LOG_VERBOSE_xLCK(...)  do{LOG_PRINTF(__VA_ARGS__);LOG_FLUSH();} while(0)
#else
	#define LOG_VERBOSE(...)       do{} while(0)
	#define LOG_VERBOSE_E(...)     do{} while(0)
	#define LOG_VERBOSE_xLCK(...)  do{} while(0)
#endif // LOG_LEVEL_DEBUG

#if LOG_ENABLE
	#define LOG_INITIALISE()    LOG_Initialise()
	#define LOG_STDOUT_LOCK()   LOG_StdoutLock()
	#define LOG_STDOUT_UNLOCK() LOG_StdoutUnlock()

	void LOG_Initialise(void);
	int LOG_StdoutLock(void);
	void LOG_StdoutUnlock(void);
	void LOG_GeneralEnable(void);
	void LOG_GeneralDisable(void);
#else
	#define LOG_INITIALISE()    do{} while(0)
	#define LOG_STDOUT_LOCK()   (0)
	#define LOG_STDOUT_UNLOCK() do{} while(0)
	#define LOG_PRINTF(...)     do{} while(0)
#endif // LOG_ENABLE

// If included STMicroelectronics "dbg_trace.h" then override
//   STMicroelectronics debug trace function calls
#if defined(__DBG_TRACE_H) && (LOG_LEVEL >= LOG_LEVEL_DEBUG)
	#ifdef PRINT_MESG_DBG
		#undef PRINT_MESG_DBG
	#endif // PRINT_MESG_DBG

	#ifdef PRINT_NO_MESG
		#undef PRINT_NO_MESG
	#endif // PRINT_NO_MESG

	#ifdef CFG_DEBUG_APP_TRACE
		#undef CFG_DEBUG_APP_TRACE
	#endif // CFG_DEBUG_APP_TRACE

	#define CFG_DEBUG_APP_TRACE 1
	#define PRINT_MESG_DBG(...) LOG_DEBUG(__VA_ARGS__)
	#define PRINT_NO_MESG(...)  LOG_DEBUG(__VA_ARGS__) // Force to debug

#endif /* __DBG_TRACE_H */
