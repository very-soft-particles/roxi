// =====================================================================================
//
//       Filename:  Application.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-08 9:41:18 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/Application.h"
#include "Application.h"
#include "rx_allocator.hpp"
#include "message_handler.h"
#include "rx_frame_manager.hpp"
#include "rx_gpu_device.hpp"
#include "rx_thread_pool.hpp"
#include "rx_vocab.h"
#include "vk_device.h"
#include <rpcndr.h>
#include <stdlib.h>
namespace roxi {

  static u64 start_times[RoxiNumFrames];

  void Application::handle_input(const u64 frame_id) noexcept {
    u32 count = 0;
    const u64 frame_index = frame_id % RoxiNumFrames;
    input::Message* input = win::READ_MSG(&count);
    for(u32 i = 0; i < count; i++) {
      // do stuff with message and frame_id
      input::Message current = input[i];
      if(i == 0) {
        start_times[frame_index] = current.time;
      }
      // dispatch on message here
      RX_TRACEF("input index = %u\n\tframe_id = %llu\n\tinput time = %llu\n\tinput code = %u\n\tinput type = %u", i, frame_id, current.time, current.code, current.event);
    }
  }

  DEFINE_JOB(initialize_base_job) {
    using app_t = Application;
    Job* jobs = (Job*)param;

    if(start == end)
      return false;

    RX_TRACE("initializing systems");

    Counter counter{0};
//    StackArray<Job> jobs;
//    jobs.push(end);

    RX_TRACEF("system count = %llu", end);
//    for(u16 i = 0; i < end; i++) {
//      //  the queue handle is zero ..|.. is a hack..
//      RX_TRACEF("creating system init job for system_id = %u", i);
//      jobs[i] = system_iter[i].get_init_job(&counter, 0, 1);
//    }

    RX_TRACE("kicking initialization jobs");
    RX_FIBER_KICK_HIGH_PRIORITY_JOBS(jobs, end);

    RX_TRACE("waiting for initialization jobs");
    RX_FIBER_WAIT(&counter, end);

    RX_TRACE("exiting initialization");
    return true;
  }

  DEFINE_JOB(update_base_job) {
    using app_t = Application;
    ISystem* system_iter = (ISystem*)param;
    if(start == end)
      return false;

    RX_TRACEF("updating frame %llu", start);

    Counter counter{0};
    StackArray<Job> jobs;
    jobs.push(end);

    RX_TRACEF("system count = %llu", end);

    for(u16 i = 0; i < end; i++) {
      RX_TRACEF("updating system = %u", i);
      jobs[i] = system_iter[i].get_update_job(&counter, start, start + 1);
    }

    RX_TRACE("kicking update jobs");
    RX_FIBER_KICK_HIGH_PRIORITY_JOBS(jobs.get_buffer(), end);

    RX_TRACEF("waiting for frame %llu udpate jobs", start);

    RX_FIBER_WAIT(&counter, end);

    return true;
  }

  DEFINE_JOB(terminate_base_job) {
    using app_t = Application;
    ISystem* system_iter = (ISystem*)param;
    if(start == end)
      return false;

    Counter counter{0};
    StackArray<Job> jobs;
    jobs.push(end);

    RX_TRACEF("system_count = %llu", end);

    for(u16 i = 0; i < end; i++) {
      RX_TRACEF("terminating system_id = %u", i);
      jobs[i] = system_iter[i].get_terminate_job(&counter, start, start + 1);
    }

    RX_TRACE("kicking termination jobs");
    RX_FIBER_KICK_HIGH_PRIORITY_JOBS(jobs.get_buffer(), end);

    RX_TRACE("waiting for termination jobs");
    RX_FIBER_WAIT(&counter, end);

    RX_TRACE("exiting termination");
    return true;
  }

  FrameManager& Application::obtain_frame_manager() {
    return _frame_manager;
  }

  const frame::ID Application::get_current_front_frame() const {
    return _current_front_frame;
  }

  b8 Application::init_sequence() {
    _arena.move_ptr(ALLOCATE(KB(64)));

    RX_TRACE("testing error");
    Error test_error(__LINE__, __FILE__, "test error");

    RX_TRACE("testing LOG_ERROR");
    LOG_ERROR(test_error, Trace);

    if(!_window.init(WINDOW_WIDTH, WINDOW_HEIGHT, "Roxi Window")) {
      RX_FATAL("failed to initialize Window");
      return false;
    }

    vk::QueueType queue_type = vk::QueueType::Generic;

    RX_TRACE("initializing gpu device");
    if(!_gpu_device.init((void*)&_window, 1, &queue_type)) {
      RX_FATAL("gpu device failed to initialize");
      return false;
    }

    RX_TRACE("creating graphics system");
    create_system<Graphics>();

    RX_TRACE("initializing start times");
    for(u64 i = 0; i < RoxiNumFrames; i++) {
      start_times[i] = 0;
    }
    ThreadPool::job_node_t node{};
    Job init_job;
    Counter counter{0};

//    RX_TRACE("creating initialization jobs");
    const u64 system_count = _systems.get_size();
//    init_job.set_table(
//        { initialize_base_job
//        , job_success_increment
//        , job_failed_minimal
//        , _current_front_frame
//        , system_count
//        , &counter
//        });

    //StackArray<Job> jobs;
    //Job* jobs_begin = jobs.push(system_count);

    RX_TRACE("getting init jobs");
    //for(u16 i = 0; i < system_count; i++) {
    //  jobs_begin[i] = _systems[i]->get_init_job(&counter, 0, 1);
    //}

    //RX_TRACE("pushing initialization jobs to queue");
    //RX_THREAD_POOL->push_high_priority_job(0, &node);
    //

    if((_systems[0]->get_init_job(&counter, 0, PTR2INT(&_gpu_device)).run())) {
      RX_TRACE("graphics init succeeded");
      return true;
    } else {
      RX_TRACE("graphics init failed");
      return false;
    }

    RX_TRACE("starting thread pool");
    RX_RUN_THREAD_POOL();
    {
      const u8 count_down = 8;
      u8 current = 0;
      RX_TRACE("running initialization tasks");
      while(counter.get_count() < 1) {
        if(!RX_RUN_REACTOR()) {
          RX_FATAL("reactor returned false in init sequence");
          RX_THREAD_POOL->terminate();
          return false;
        }
        current++;
        if(current == count_down) {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(5ms);
          current = 0;
        }
      }
    }

    return true;
  }

  b8 Application::update_sequence() {
//    ThreadPool::job_node_t node{};
//    Job update_job;
    Counter counter;
//
    const u64 system_count = _systems.get_size();
//    update_job.set_table(
//        { update_base_job
//        , job_success_increment
//        , job_failed_minimal
//        , _current_front_frame
//        , system_count
//        , &counter
//        });
//    update_job.set_obj(_systems.get_buffer());
//
//    node.clear();
//    node.data = update_job;

    RX_TRACE("entering main loop");
    b8 br = false;
    b8 loop = true;
    const auto start_time = Time::now();
    do {
      for(u32 i = 0; i < system_count; i++) {
        if(!_systems[i]->get_update_job(&counter, 0, 1).run()) {
          RX_ERRORF("system %u failed to update", i);
          br = true;
        }
      }
      if(br) {
        break;
      }
    }
    while((loop = win::PROCESS()) && counter.get_count() < 1);
//      {
//        //using namespace std::chrono_literals;
//        //std::this_thread::sleep_for(10ms);
//        if(!RX_RUN_REACTOR()) {
//          RX_FATAL("reactor kernel returned false in update loop");
//          br = true;
//          break;
//        }
//      }
//      if(br) {
//        break;
//      }
//      handle_input(_current_front_frame);
//      _current_front_frame++;
//      while((loop = win::PROCESS()) && (_current_front_frame * RX_FRAME_TIME) + start_time < Time::now()) {
//      }
//      _frame_manager.reset_frame(_current_front_frame);
//      counter.reset();
//      node.clear();
//      node.data.set_job_start(_current_front_frame - 1);
//      node.data.set_job_end(_current_front_frame);
//    } while(loop);
//    _frame_manager.reset_frame(_current_front_frame);
    return !br;
  }

  b8 Application::terminate_sequence() {
    //ThreadPool::job_node_t node{};
    //Job term_job;
    Counter counter{0};

    const u64 system_count = _systems.get_size();
    //term_job.set_table(
    //    { terminate_base_job
    //    , job_success_increment
    //    , job_failed_minimal
    //    , _current_front_frame
    //    , system_count
    //    , &counter
    //    });
    //term_job.set_obj(_systems.get_buffer());
    //node.data = term_job;
    //node.data.set_job_counter(&counter);
    //node.data.set_job_start(0);
    //node.data.set_job_end(1);
    //RX_THREAD_POOL->push_high_priority_job(0, &node);
    //{
    //  const u8 count_down = 8;
    //  u8 current = 0;
    //  while(counter.get_count() < 1) {
    //    if(!RX_THREAD_POOL->reactor_kernel()) {
    //      RX_ERROR("reactor returned false in terminate sequence");
    //      RX_THREAD_POOL->terminate();
    //      return false;
    //    }
    //    current++;
    //    if(current == count_down) {
    //      using namespace std::chrono_literals;
    //      std::this_thread::sleep_for(5ms);
    //      current = 0;
    //    }
    //  }
    //}

    for(u32 i = 0; i < system_count; i++) {
      RX_CHECKF(_systems[i]->get_terminate_job(&counter, 0, 1)
          , "failed to terminate system %u"
          , i);
    }

    RX_THREAD_POOL->terminate();
    return true;
  }
  
  b8 Application::run() noexcept {
    RX_TRACE("attempting init sequence");
    if(!init_sequence()) {
      RX_FATAL("initialization sequence failed, halting application");
      return 0;
    }
    RX_TRACE("attempting update sequence");
    if(!update_sequence()) {
      RX_FATAL("update sequence failed, halting application");
      //RX_THREAD_POOL->terminate();
      return 0;
    }
    RX_TRACE("attempting terminate sequence");
    if(!terminate_sequence()) {
      RX_FATAL("terminate sequence failed, halting application");
      return 0;
    }
    RX_TRACE("exiting application...");
    return 1;
  }
}		// -----  end of namespace roxi  ----- 
