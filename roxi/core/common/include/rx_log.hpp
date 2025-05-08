// =====================================================================================
//
//       Filename:  rx_log.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-30 5:00:30 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_string.hpp"
#include "rx_thread_pool.hpp"
#include "rx_container.hpp"
#include "rx_arena.hpp"
#include "rx_time.h"

#ifdef RX_USE_LOGGING

  #define LOG(message, priority) LogSystem::instance()->get_logger()->log(LogPriority::priority, message)
  
  #define LOGF(priority, message, ...) LogSystem::instance()->get_logger()->logf(LogPriority::priority, message, __VA_ARGS__)
  
  #define LOG_ERROR(error,priority) LOG(error.what(), priority)
   
  #define DUMP_LOGS() LogSystem::instance()->dump_logs()

  #define RX_TRACEF(message, ...) LogSystem::instance()->get_logger()->logf(LogPriority::Trace, message, __VA_ARGS__)
  
  #define RX_ERRORF(message, ...) LogSystem::instance()->get_logger()->logf(LogPriority::Error, message, __VA_ARGS__)
  
  #define RX_FATALF(message, ...) LogSystem::instance()->get_logger()->logf(LogPriority::Fatal, message, __VA_ARGS__)

#define RX_TRACE(message) LOG(message, Trace)

#define RX_ERROR(message) LOG(message, Error)

#define RX_FATAL(message) LOG(message, Fatal)

#else

  #define LOG(message, priority) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n%s\n", RX_THREAD_ID, get_priority_string(LogPriority::priority), Time::localtime_to_string(), message)

  #define LOGF(priority, message, ...) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n%s\n", RX_THREAD_ID, get_priority_string(LogPriority::priority), Time::localtime_to_string(), message, __VA_ARGS__)

  #define LOG_ERROR(error,priority) LOG(error.what(), priority)

  #define DUMP_LOGS() (void)0

  #define RX_TRACEF(message, ...) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n\t", RX_THREAD_ID, get_priority_string(LogPriority::Trace), Time::localtime_to_string());\
    PRINT(message, __VA_ARGS__);\
    PRINT_C('\n')

  #define RX_ERRORF(message, ...) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n\t", RX_THREAD_ID, get_priority_string(LogPriority::Error), Time::localtime_to_string());\
    PRINT(message, __VA_ARGS__);\
    PRINT_C('\n')
  
  #define RX_FATALF(message, ...) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n\t", RX_THREAD_ID, get_priority_string(LogPriority::Fatal), Time::localtime_to_string());\
    PRINT(message, __VA_ARGS__);\
    PRINT_C('\n')

  #define RX_TRACE(message) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n\t%s\n", RX_THREAD_ID, get_priority_string(LogPriority::Trace), Time::localtime_to_string(), message)
  
  #define RX_ERROR(message) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n\t%s\n", RX_THREAD_ID, get_priority_string(LogPriority::Error), Time::localtime_to_string(), message)
  
  #define RX_FATAL(message) PRINT("[:LOGGER::%llu::PRIORITY::%s] ::: [:TIME::%s] ::: \n\t%s\n", RX_THREAD_ID, get_priority_string(LogPriority::Fatal), Time::localtime_to_string(), message)

#endif

#define RX_PRIORITIES(X)\
  X(Trace) \
  X(Debug) \
  X(Info) \
  X(Warn) \
  X(Error) \
  X(Critical) \
  X(Fatal) \
  X(Vulkan) \
  X(RLSL) \
  X(MAX)

namespace roxi {

  class LogQueue {
    private:
      StringList log_list;
      StackArena arena;

    public:
      String get_logs() {
        String result = lofi::str_join(&arena, &log_list, nullptr);
        return result;
      }

      void enqueue(StringList* list) {
        String str = lofi::str_join
          ( &arena
            , list
            , nullptr
          );

        lofi::str_list_push
          (&arena
           , &log_list
           , str);
      }
  };


  enum class LogPriority {
#define ENUM(priority) priority, 
  RX_PRIORITIES(ENUM)
#undef ENUM
  };

  static constexpr const char* get_priority_string(LogPriority priority) {
#define STRING(X) if (priority == LogPriority::X) return (const char*)STRINGIFY(X);
    RX_PRIORITIES(STRING)
#undef STRING
      else
        return "";
  }

  class LogSystem;

  class Logger {
  public:
    Logger() {}

  private:
    // default to setting priority at info level
    // only logs which register this priority or higher will
    // be printed into the log

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger) = delete;

    LogPriority priority = LogPriority::Info;
    StringList buffer_list;
    StackArena arena;
  public:
    void set_priority(LogPriority new_priority){
      priority = new_priority;
    }

    void log(LogPriority priority, const char* message) {
      String priority_str = lofi::str_cstring(get_priority_string(priority));
      String localtime_str = lofi::str_cstring(Time::localtime_to_string());
      String format_str = lofi::str_cstring("[:LOGGER::%llu::PRIORITY::%s:] ::: [:TIME::%s] :::\n\t%s\n");
      String result = lofi::str_pushf(&arena, (char*)format_str.str, (char*)priority_str.str, (char*)localtime_str.str, message);
      lofi::str_list_push(&arena, &buffer_list, result);
    }

    void logf(LogPriority priority, const char* message, ...) {
      va_list args;
      va_start(args, message);
      String priority_str = lofi::str_cstring(get_priority_string(priority));
      String localtime_str = lofi::str_cstring(Time::localtime_to_string());
      String format_str = lofi::str_cstring("[:LOGGER::%llu::PRIORITY::%s:] ::: [:TIME::%s] :::\n\t%s\n");
      String intermed = lofi::str_pushfv(&arena, message, args);
      String result = lofi::str_pushf(&arena, (char*)format_str.str, (char*)priority_str.str, (char*)localtime_str.str, intermed.str);
      lofi::str_list_push(&arena, &buffer_list, result);
    }

    StringList retrieve_logs() {
      return buffer_list;
    }
  };

  class LogSystem {
    private:
      friend class Logger;
      Arena arena;

      static String filepath;
      static FILE* file;

      LogQueue log_queue;
      StackArray<Logger*> loggers;

    public:
      u32 register_logger(Logger* new_logger);

      static LogSystem* instance();

      Logger* get_logger();
      bool enable_file_output(const char* new_filepath = "");
      void dump_logs();
  };

}		// -----  end of namespace roxi  ----- 
