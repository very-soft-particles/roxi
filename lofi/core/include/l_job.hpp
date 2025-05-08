// =====================================================================================
//
//       Filename:  l_job.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-07-22 2:03:04 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_vocab.hpp"
#include "l_sync.hpp"

#define DECLARE_JOB(name)         static b8   name(void* param, u64 start, u64 end)
#define DECLARE_JOB_SUCCESS(name) static void name(void* param, void* counter)           // increments the count
#define DECLARE_JOB_FAILURE(name) static void name(void* param, u64 start, u64 end)                  // logs failure, potentially halts program

#define DEFINE_JOB(name)         static b8   name(void* param, u64 start, u64 end)
#define DEFINE_JOB_SUCCESS(name) static void name(void* param, void* counter)           // increments the count
#define DEFINE_JOB_FAILURE(name) static void name(void* param, u64 start, u64 end)                  // logs failure, potentially halts program

namespace lofi {
  namespace job {
  
    using entry = b8(*)(void*, u64, u64);
    using success = void(*)(void*, void*);
    using failure = void(*)(void*, u64, u64);
 
    struct decl {
      entry entry_point= nullptr;
      success job_success = nullptr;
      failure job_failure = nullptr;
 
      u64 start = MAX_u64;
      u64 end = MAX_u64;
      atomic_counter<>* counter = 0;

      decl() {}

      decl(entry enter, success succeed, failure fail, u64 start, u64 end, atomic_counter<>* counter) 
        : entry_point{enter}
        , job_success{succeed}
        , job_failure{fail}
        , start{start}
        , end{end}
        , counter{counter}
        {}
      // count_to ??
    };
  }		// -----  end of namespace job  ----- 

  class Job {
  private:
    job::decl func_table;
    void* data = nullptr;
  public:

    Job() {};
    operator bool() {
      if(func_table.entry_point)
        return true;
      return false;
    }

    Job& set_obj(void* obj) {
      data = obj;
      return *this;
    }

    Job& set_table(job::decl&& declaration) {
      func_table = std::move(declaration);
      return *this;
    }

    Job& set_entry_point(job::entry enter) {
      func_table.entry_point = enter;
      return *this;
    }

    Job& set_job_success(job::success succeed) {
      func_table.job_success = succeed;
      return *this;
    }

    Job& set_job_failure(job::failure fail) {
      func_table.job_failure = fail;
      return *this;
    }

    Job& set_job_start(u64 start) {
      func_table.start = start;
      return *this;
    }

    Job& set_job_end(u64 end) {
      func_table.end = end;
      return *this;
    }

    Job& set_job_counter(atomic_counter<>* counter) {
      func_table.counter = counter;
      return *this;
    }

    b8 run() {
      if(func_table.entry_point) {
        b8 result = func_table.entry_point(data, func_table.start, func_table.end);
        if (result && func_table.job_success && func_table.counter) {
          func_table.job_success(data, func_table.counter);
          return true;
        } else if (!result && func_table.job_failure) {
          func_table.job_failure(data, func_table.start, func_table.end);
          return false;
        }
        return false;
      }
      return false;
    }
  };




}		// -----  end of namespace lofi  ----- 
