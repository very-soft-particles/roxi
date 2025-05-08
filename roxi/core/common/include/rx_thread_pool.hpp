// =====================================================================================
//
//       Filename:  rx_thread_pool.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:36:54 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../../../lofi/core/include/l_thread_pool.hpp"
#include "rx_vocab.h"

#define RX_THREAD_POOL GET_THREAD_POOL(ThreadPool)

#define RX_THREAD_ID GET_HOST_THREAD_ID(ThreadPool)

#define RX_THIS_WORKER GET_HOST_WORKER(ThreadPool)

#define RX_RUN_THREAD_POOL() RX_THREAD_POOL->run()

#define RX_KILL_THREAD_POOL() RX_THREAD_POOL->kill()

#define RX_RUN_REACTOR() RX_THREAD_POOL->reactor_kernel()

#define RX_FIBER_KICK_HIGH_PRIORITY_JOBS(jobs, job_count) GET_HOST_WORKER(ThreadPool)->kick_high_priority_jobs((jobs), (job_count))

#define RX_FIBER_KICK_MID_PRIORITY_JOBS(jobs, job_count) GET_HOST_WORKER(ThreadPool)->kick_mid_priority_jobs((jobs), (job_count))

#define RX_FIBER_KICK_LOW_PRIORITY_JOBS(jobs, job_count) GET_HOST_WORKER(ThreadPool)->kick_high_priority_jobs((jobs), (job_count))

#define RX_FIBER_KICK_HIGH_PRIORITY_JOB(job) GET_HOST_WORKER(ThreadPool)->kick_high_priority_job(job)

#define RX_FIBER_KICK_MID_PRIORITY_JOB(job) GET_HOST_WORKER(ThreadPool)->kick_mid_priority_job(job)

#define RX_FIBER_KICK_LOW_PRIORITY_JOB(job) GET_HOST_WORKER(ThreadPool)->kick_high_priority_job(job)

#define RX_FIBER_WAIT(counter, wait_count) GET_HOST_WORKER(ThreadPool)->fiber_wait((counter), (wait_count))

#define RX_FIBER_YIELD() GET_HOST_WORKER(ThreadPool)->yield()

#define RX_SLEEP_FOR(nanoseconds) std::this_thread::sleep_for(std::chrono_literals::operator""ns((u64)(nanoseconds)));

namespace roxi {

   using ThreadPool = lofi::ThreadPool<RoxiNumThreads, RoxiNumFibers>;

  using Counter = lofi::atomic_counter<>;

  using Job = lofi::Job;

  //template<class TableDescriptorT>
  //using Database = lofi::Database<TableDescriptorT>;


  DEFINE_JOB_SUCCESS(job_success_increment) {
    Counter* a_counter = (Counter*)counter;
    ++(*a_counter);
  }
  
  DEFINE_JOB_FAILURE(job_failed_minimal) {
    PRINT_LINE("roxi job failed!");
  }

  DEFINE_JOB_FAILURE(job_failed_no_op) {
  }

}		// -----  end of namespace roxi  ----- 

extern template class lofi::ThreadPool<roxi::RoxiNumThreads, roxi::RoxiNumFibers>;
