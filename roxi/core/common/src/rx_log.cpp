// =====================================================================================
//
//       Filename:  rx_log.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-30 5:47:37 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_log.hpp"
#include "rx_thread_pool.hpp"

namespace roxi {

  LogSystem s_log_system;
  String LogSystem::filepath = String{nullptr, 0};
  FILE* LogSystem::file = nullptr;

  LogSystem* LogSystem::instance() {
    return &s_log_system;
  }

  bool LogSystem::enable_file_output(const char* new_filepath){
    SizedStackArena<KB(1)> scratch;
    if (file != nullptr){
      fclose(file);
    }
    tm timestamp = Time::localtime();
    String f_string = lofi::str_cstring("%s[%d]-[%d]-[%d]__roxilog.txt");
    String temp_filepath = lofi::str_pushf(&scratch, (char*)f_string.str, new_filepath, timestamp.tm_year + 1900, timestamp.tm_mday, timestamp.tm_mon + 1);

    StringList list{};
    lofi::str_list_push(&scratch, &list, temp_filepath);

    filepath = str_join(&arena, &list, nullptr);
    auto err = fopen_s(&file, (char*)filepath.str, "a");
    // RX_CHECK(file);
    if(err)
      return false;
    return true;
  }

  u32 LogSystem::register_logger(Logger* new_logger) {
    const u32 index = loggers.get_size();
    *(loggers.push(1)) = new_logger;
    return index;
  }

  Logger* LogSystem::get_logger() {
    const u64 id = GET_HOST_WORKER(ThreadPool)->get_thread_id();
    const u32 logger_count = loggers.get_size();
    if(id >= logger_count) {

      PRINT("[ROXI FATAL]: attempting to access invalid log id %llu", id);
      return nullptr;
    }
    
    return loggers[id];
  }

  void LogSystem::dump_logs() {
    for(size_t i = 0; i < loggers.get_size(); i++) {
      StringList list = loggers[i]->retrieve_logs();
      log_queue.enqueue(&list);
    }
    if(file) {
      String logs = log_queue.get_logs();
      fprintf(file, "%s\n", (char*)logs.str);
      fclose(file);
      file = nullptr;
    }
  }

}		// -----  end of namespace roxi  ----- 
