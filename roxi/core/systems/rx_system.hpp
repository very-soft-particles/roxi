// =====================================================================================
//
//       Filename:  rx_system.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-12 6:24:40 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_log.hpp"
#include "rx_thread_pool.hpp"
//
//#define DECLARE_JOB_CREATE_FUNCTION(system, job_type)\
//  static Job create_##system##_##job_type##_job(void* obj, u64 start, u64 end, Counter* counter) {\
//    RX_TRACE("creating "#system" "#job_type);\
//    Job result{};                             \
//    result.set_entry_point(                   \
//          systems::system::job_type           \
//        ).set_job_failure(                   \
//          job_failed_no_op                    \
//        ).set_job_success(                   \
//          job_success_increment               \
//        ).set_obj(obj);\
//    return result;                            \
//  }
//
namespace roxi {
//
//  class System {
//  private:
//    using job_create_func_t = Job(*)(void*, u64, u64, Counter*);
//    void* _obj_ptr = nullptr;
//    job_create_func_t _create_init_job;
//    job_create_func_t _create_update_job;
//    job_create_func_t _create_terminate_job;
//  public:
//    static System create(void* obj, job_create_func_t create_init_job, job_create_func_t create_update_job, job_create_func_t create_terminate_job) {
//      RX_TRACE("creating system");
//      System result;
//      result._obj_ptr = obj;
//      result._create_init_job = create_init_job;
//      result._create_update_job = create_update_job;
//      result._create_terminate_job = create_terminate_job;
//      return result;
//    }
//
//    Job get_init_job(Counter* counter, const u64 start, const u64 end) {
//      RX_TRACE("creating initialization job");
//      return _create_init_job(_obj_ptr, start, end, counter);
//    }
//
//    Job get_update_job(Counter* counter, const u64 start, const u64 end) {
//      RX_TRACE("creating update job");
//      return _create_update_job(_obj_ptr, start, end, counter);
//    }
//
//    Job get_terminate_job(Counter* counter, const u64 start, const u64 end) {
//      RX_TRACE("creating terminate job");
//      return _create_terminate_job(_obj_ptr, start, end, counter);
//    }
//  };
//

  class ISystem {
  public:
    virtual Job get_init_job(Counter* counter, const u64 start, const u64 end) = 0;
    virtual Job get_update_job(Counter* counter, const u64 start, const u64 end) = 0;
    virtual Job get_terminate_job(Counter* counter, const u64 start, const u64 end) = 0;
  };

}		// -----  end of namespace roxi  ----- 


//
//#define RX_DECLARE_SYSTEM(system_name)\
//  namespace system_name##_private {\
//  DECLARE_JOB_CREATE_FUNCTION(system_name, init)\
//  DECLARE_JOB_CREATE_FUNCTION(system_name, update)\
//  DECLARE_JOB_CREATE_FUNCTION(system_name, terminate)\
//  static System create_##system_name##_system(void* obj) {\
//    return System::create(obj\
//        , create_##system_name##_init_job\
//        , create_##system_name##_update_job\
//        , create_##system_name##_terminate_job\
//        );\
//  }\
//  }
//
//#define RX_CREATE_SYSTEM(system_name, obj) system_name##_private::create_##system_name##_system(obj)
//
