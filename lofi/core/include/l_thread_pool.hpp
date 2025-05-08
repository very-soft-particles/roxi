// =====================================================================================
//
//       Filename:  l_thread_pool.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-13 9:31:08 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_sync.hpp"
#include "l_allocator.hpp"
#include "l_arena.hpp"
#include "l_tuple.hpp"
#include "l_fiber.hpp"
#include "l_variant.hpp"
#include "l_job.hpp"
#include "inline/preprocessor.hpp"

#if(OS_WINDOWS)
#include <Windows.h>
#endif

#include <thread>

#define GET_NUM_THREADS(desired_number) CLAMP_TOP(desired_number, MAX((std::thread::hardware_concurrency()), 1))

#define GET_THREAD_POOL(pool_t) pool_t::instance()
#define GET_HOST_WORKER(pool_t) pool_t::instance()->get_host_worker()
#define GET_HOST_THREAD_ID(pool_t) pool_t::instance()->get_host_worker()->get_thread_id()

namespace lofi {
  template<size_t NumThreads, size_t NumFibers>
  static void fiber_main(void* data);

  template<size_t NumThreads, size_t NumFibers>
  class ThreadPool {
    static constexpr u64 StackSize = KB(64);
    static constexpr u64 WaitListSize = 64;
    static constexpr u64 JobQueueSize = 64;
    
    using Fiber = Fiber<StackSize>;
    using FiberHandle = FiberHandle<StackSize>;

    struct wait_node {
      FiberHandle handle;
      atomic_counter<>* counter = 0;
      u64 count_to_wait = 0;
    };
    using fiber_pool_t = lock_free_pool<Fiber, NumFibers>;
    using stack_t = mem::Block<StackSize>;
    using stack_pool_t = lock_free_pool<stack_t, NumFibers>;
    using task_queue_t = lock_free_queue<Job, NumThreads>;
    using waiting_fiber_list_t = lock_free_queue<wait_node, NumThreads>;
    using waiting_fiber_list_node_t = typename waiting_fiber_list_t::node;
    using waiting_fiber_list_node_handle_t = waiting_fiber_list_node_t*;
    friend void fiber_main<NumThreads, NumFibers>(void* data);
    static ThreadPool _instance;
  public:

    static ThreadPool* instance() {
      return &_instance;
    }

    using job_node_t = typename task_queue_t::node;

    ThreadPool() {
      num_workers = GET_NUM_THREADS(NumThreads);
      _workers = (Worker**)RuntimeAllocator<0>::allocate(sizeof(Worker*) * num_workers, 8); // NOLINT
      for(size_t i = 0; i < num_workers; i++) {
        _workers[i] = StaticAllocator::allocate<0, Worker>();
      }
      fiber_pool.set_ptr(StaticAllocator::allocate<0, mem::Block<sizeof(Fiber) * NumFibers>>());
      stack_pool.set_ptr(StaticAllocator::allocate<0, mem::Block<sizeof(stack_t) * NumFibers>>());
    }

    b8 run() {
      for(size_t i = 0; i < num_workers; i++) {
        //PRINT("constructing worker %llu\n", i);
        new(_workers[i]) Worker(this, i, StaticAllocator::allocate<0, mem::Block<KB(20)>>());
      }
      return true;
    }

    b8 kill() {
      high_priority_job_queue.clear();
      mid_priority_job_queue.clear();
      low_priority_job_queue.clear();
      thread_local_task_counter.reset();
      for(size_t i = 0; i < NumThreads; i++) {
        _workers[i]->halt();
      }
      return true;
    }

    bool terminate() {
      if(!kill()) {
        return false;
      }
      for(size_t i = 0; i < NumThreads; i++) {
        _workers[i]->join();
      }
      return true;
    }

    void push_reactor_jobs(size_t thread_id, job_node_t* first, job_node_t* last, u32 count) {
      reactor_queue.push(thread_id, first, last, count);
    }

    void push_high_priority_jobs(size_t thread_id, job_node_t* first, job_node_t* last, u32 count) {
      high_priority_job_queue.push(thread_id, first, last, count);
    }

    void push_mid_priority_jobs(size_t thread_id, job_node_t* first, job_node_t* last, u32 count) {
      mid_priority_job_queue.push(thread_id, first, last, count);
    }

    void push_low_priority_jobs(size_t thread_id, job_node_t* first, job_node_t* last, u32 count) {
      low_priority_job_queue.push(thread_id, first, last, count);
    }

    void push_reactor_job(size_t thread_id, job_node_t* new_node) {
      reactor_queue.push(thread_id, new_node);
    }

    void push_high_priority_job(size_t thread_id, job_node_t* new_node) {
      high_priority_job_queue.push(thread_id, new_node);
    }

    void push_mid_priority_job(size_t thread_id, job_node_t* new_node) {
      mid_priority_job_queue.push(thread_id, new_node);
    }

    void push_low_priority_job(size_t thread_id, job_node_t* new_node) {
      low_priority_job_queue.push(thread_id, new_node);
    }

    b8 reactor_kernel() {
      Job job;
      while(reactor_queue.pop(0, &job)) {
        b8 result = job.run();
        if(!result) {
          return false;
        }
      }
      return true;
    }

  private:
    FiberHandle pull_fiber(size_t thread_id, waiting_fiber_list_node_handle_t new_wait_node) {
      FiberHandle result = nullptr;
      b8 got_fiber = false;
      if(waiting_fiber_list.pop(thread_id, &(new_wait_node->data))) {
        got_fiber = true;
        if(new_wait_node->data.counter->get_count() >= new_wait_node->data.count_to_wait) {
          result = new_wait_node->data.handle;
          new_wait_node->to_delete = true;
          return result;
        }
        new_wait_node->to_delete = false;
        new_wait_node->next = nullptr;
        waiting_fiber_list.push(thread_id, new_wait_node);
      }
      if(!got_fiber) {
        new_wait_node->to_delete = true;
      }
      result = fiber_pool.get_object();
      new(result) Fiber(fiber_main<NumThreads, NumFibers>, (void*)this, (void*)stack_pool.get_object());
      return result;
    }

    void return_fiber(size_t thread_id, FiberHandle fiber_to_return) {
      stack_pool.return_object((stack_t*)fiber_to_return->get_stack_base());
      fiber_to_return->clear();                       // sets stack to nullptr
      fiber_pool.return_object(fiber_to_return);
    }

    Job pull_job(size_t thread_id) {
      Job result;
      high_priority_job_queue.pop(thread_id, &result);
      if(!result) {
        mid_priority_job_queue.pop(thread_id, &result);
        if(!result) {
          low_priority_job_queue.pop(thread_id, &result);
        }
      }
      return result;
    }

  public:
    class Worker {
    private:
      friend void fiber_main<NumThreads, NumFibers>(void*);
      static constexpr size_t MaxDeadHandles = 32;
      static constexpr size_t MaxDeadJobs = 64;
      using fiber_node_t = waiting_fiber_list_node_t;
      using fiber_node_handle_t = fiber_node_t*;
      using job_node_t = ThreadPool::job_node_t;
      using fiber_handle_pool_t = mem::FreeListContainerPolicy<fiber_node_t, MaxDeadHandles, 8, mem::SubAllocPolicy>;
      using dead_fiber_handles_t = mem::PackedArrayContainerPolicy<fiber_node_handle_t, MaxDeadHandles, 8, mem::SubAllocPolicy>;
      using job_handle_pool_t = mem::FreeListContainerPolicy<job_node_t, MaxDeadJobs, 8, mem::SubAllocPolicy>;
      using dead_job_handles_t = mem::PackedArrayContainerPolicy<job_node_t*, MaxDeadJobs, 8, mem::SubAllocPolicy>;
      using tls_t = Arena<KB(20), 8, mem::SubAllocPolicy>;
    public:
      Worker(ThreadPool* pool, size_t _thread_id, void* thread_local_storage) 
        : pool_ptr(pool)
        , _tls{thread_local_storage}
        , thread_id(_thread_id)
        , handle_pool(_tls.push(sizeof(fiber_node_t)*MaxDeadHandles))
        , dead_handles(_tls.push(sizeof(fiber_node_handle_t)*MaxDeadHandles))
        , job_pool(_tls.push(sizeof(job_node_t)*MaxDeadJobs))
        , dead_job_handles(_tls.push(sizeof(job_node_t*)*MaxDeadJobs))
        , _thread(&Worker::run_kernel, this) 
      {
        //PRINT("inside thread %llu constructor, tls = %llu\n", thread_id, thread_local_storage);
      }

      const size_t get_thread_id() {
        return thread_id;
      }

      void halt() {
        should_halt = true;
      }

      b8 can_join() {
        return _thread.joinable();
      }

      void join() {
        _thread.join();
      }

      const FiberHandle get_current_fiber() const {
        return current_fiber;
      }

      fiber_node_handle_t create_waiter() {
        auto index = dead_handles.add_object(handle_pool.add_object());
        L_ASSERT(dead_handles[index] != nullptr && "ran out of waiter nodes in handle_pool");
        dead_handles[index]->to_delete = false;
        dead_handles[index]->next = nullptr;
        dead_handles[index]->data.handle = nullptr;
        return dead_handles[index];
      }

      void fiber_wait(lofi::atomic_counter<>* counter, const u64 count_to_wait) {
        fiber_node_handle_t wait_handle = create_waiter();
        wait_handle->to_delete = false;
        wait_handle->next = nullptr;
        wait_handle->data.counter = counter;
        wait_handle->data.count_to_wait = count_to_wait;
        wait_handle->data.handle = current_fiber;
        pool_ptr->waiting_fiber_list.push(thread_id, wait_handle);
        current_fiber->wait();
      }

      void kick_high_priority_jobs(Job* jobs, const u32 job_count) {
        auto first_index = MAX_u64;
        auto last_index = MAX_u64;
        for(u32 i = 0; i < job_count; i++) {
          auto index = dead_job_handles.add_object(job_pool.add_object());
          L_ASSERT(dead_job_handles[index] != nullptr && "ran out of job nodes in high priority job_pool");
          dead_job_handles[index]->data = jobs[i];
          dead_job_handles[index]->clear();
          if(last_index != MAX_u64) {
            dead_job_handles[last_index]->next = dead_job_handles[index];
          } else {
            first_index = index;
          }
          last_index = index;
        }
        pool_ptr->push_high_priority_jobs(thread_id, dead_job_handles[first_index], dead_job_handles[last_index], job_count);
      }

      void kick_mid_priority_jobs(Job* jobs, const u32 job_count) {
        auto first_index = MAX_u64;
        auto last_index = MAX_u64;
        for(u32 i = 0; i < job_count; i++) {
          auto index = dead_job_handles.add_object(job_pool.add_object());
          L_ASSERT(dead_job_handles[index] != nullptr && "ran out of job nodes in high priority job_pool");
          dead_job_handles[index]->data = jobs[i];
          dead_job_handles[index]->clear();
          if(last_index != MAX_u64) {
            dead_job_handles[last_index]->next = dead_job_handles[index];
          } else {
            first_index = index;
          }
          last_index = index;
        }
        pool_ptr->push_mid_priority_jobs(thread_id, dead_job_handles[first_index], dead_job_handles[last_index], job_count);
      }


      void kick_low_priority_jobs(Job* jobs, const u32 job_count) {
        auto first_index = MAX_u64;
        auto last_index = MAX_u64;
        for(u32 i = 0; i < job_count; i++) {
          auto index = dead_job_handles.add_object(job_pool.add_object());
          L_ASSERT(dead_job_handles[index] != nullptr && "ran out of job nodes in high priority job_pool");
          dead_job_handles[index]->data = jobs[i];
          dead_job_handles[index]->clear();
          if(last_index != MAX_u64) {
            dead_job_handles[last_index]->next = dead_job_handles[index];
          } else {
            first_index = index;
          }
          last_index = index;
        }
        pool_ptr->push_mid_priority_jobs(thread_id, dead_job_handles[first_index], dead_job_handles[last_index], job_count);
      }

      void kick_high_priority_job(Job job) {
        auto index = dead_job_handles.add_object(job_pool.add_object());
        L_ASSERT(dead_job_handles[index] != nullptr && "ran out of job nodes in high priority job_pool");
        dead_job_handles[index]->data = job;
        dead_job_handles[index]->clear();
        pool_ptr->push_high_priority_job(thread_id, dead_job_handles[index]);
      }

      void kick_mid_priority_job(Job job) {
        auto index = dead_job_handles.add_object(job_pool.add_object());
        L_ASSERT(dead_job_handles[index] != nullptr && "ran out of job nodes in high priority job_pool");
        dead_job_handles[index]->data = job;
        dead_job_handles[index]->clear();
        pool_ptr->push_mid_priority_job(thread_id, dead_job_handles[index]);
      }

      void kick_low_priority_job(Job job) {
        auto index = dead_job_handles.add_object(job_pool.add_object());
        L_ASSERT(dead_job_handles[index] != nullptr && "ran out of job nodes in low priority job_pool");
        dead_job_handles[index]->data = job;
        dead_job_handles[index]->clear();
        pool_ptr->push_low_priority_job(thread_id,dead_job_handles[index]);
      }

      b8 owns(FiberHandle fiber) {
        //PRINT("thread %llu checking ownership\n", thread_id);
        if(!current_fiber) {
          return false;
        }
        return (*current_fiber) == fiber;
      }

      // an initialization function for doing thread dependent initialization work
//      b8 execute_initialization_tasks() {
//        auto tasks = pool_ptr->get_thread_init_tasks(thread_id);
//        const size_t num_tasks = tasks.get_size();
//        for(size_t i = 0; i < num_tasks; i++) {
//          if(!run_local_job(tasks[i])) {
//            return false;
//          }
//        }
//        return true;
//      }


      // an initialization function for doing thread dependent termination work
//      b8 execute_termination_tasks() {
//        auto tasks = pool_ptr->get_thread_term_tasks(thread_id);
//        const size_t num_tasks = tasks.get_size();
//        for(size_t i = 0; i < num_tasks; i++) {
//          if(!run_local_job(tasks[i])) {
//            return false;
//          }
//        }
//        return true;
//      }

    private:

      void yield() {
        current_fiber->yield();
      }

      b8 try_delete_fiber(u32 index) {
        if(dead_handles[index]->to_delete) {
          handle_pool.remove_object((void*)dead_handles[index]);
          dead_handles[index]->to_delete = false;
          dead_handles.remove_object(index);
          return true;
        }
        return false;
      }

      void prune_dead_fiber_handles() {
        for(size_t i = 0; i < dead_handles.get_size(); i++) {
          try_delete_fiber(i);
        }
      }

      void prune_dead_job_handles() {
        for(size_t i = 0; i < dead_job_handles.get_size(); i++) {
          if(dead_job_handles[i]->to_delete) {
            job_pool.remove_object((void*)dead_job_handles[i]);
            dead_job_handles[i]->to_delete = false;
            dead_job_handles.remove_object(i);
          }
        }
      }

      void run_kernel() {

        //PRINT("thread %llu running kernel\n", thread_id);
#if(OS_WINDOWS)
        //SetThreadAffinityMask((HANDLE)_thread.native_handle(), BIT(thread_id));
#elif (OS_LINUX)
#elif (OS_MAC)
#endif
        //const size_t num_tasks = pool_ptr->get_num_init_tasks();
        //execute_initialization_tasks();
        //while(pool_ptr->get_current_thread_local_task_count() <= num_tasks) {
        //    using namespace std::chrono_literals;
        //    std::this_thread::sleep_for(5ms);
        //}
        while(!should_halt) {
          if(current_fiber) [[likely]] {
            //using namespace std::chrono_literals;
            //std::this_thread::sleep_for(5ms);
            prune_dead_job_handles();
            if(!current_fiber->is_waiting()) {
              pool_ptr->return_fiber(thread_id, current_fiber);
            }
            prune_dead_fiber_handles();
          }
          Fiber here = Fiber();
          fiber_node_handle_t wait_handle = create_waiter();
          current_fiber = pool_ptr->pull_fiber(thread_id, wait_handle);
          current_fiber->swap(&here);
        }
        //execute_termination_tasks();
      }

      ThreadPool *pool_ptr;
      tls_t _tls{nullptr};
      size_t thread_id;
      volatile FiberHandle current_fiber = nullptr;
      fiber_handle_pool_t handle_pool{nullptr};
      dead_fiber_handles_t dead_handles{nullptr};
      job_handle_pool_t job_pool{nullptr};
      dead_job_handles_t dead_job_handles{nullptr};
      std::thread _thread;
      volatile b8 should_halt = false;
    };

    Worker* get_host_worker() {
      Fiber here = Fiber();
      for(size_t i = 0; i < num_workers; i++) {
        Worker* worker = _workers[i];
        //PRINT("checking WORKER ID = %llu\n", worker->get_thread_id());
        if(worker->owns(&here)) {
          return worker;
        }
      }
      //PRINT_S("failed to find host worker, returning worker 0\n");
      return _workers[0];
    }

  private:
    Worker** _workers;

    task_queue_t high_priority_job_queue;
    task_queue_t mid_priority_job_queue;
    task_queue_t low_priority_job_queue;

    task_queue_t reactor_queue;
    waiting_fiber_list_t waiting_fiber_list;
    stack_pool_t stack_pool;
    fiber_pool_t fiber_pool;
    atomic_counter<> thread_local_task_counter{0};
    //HeapAllocator<0> local_memory_pool;
    u8 num_workers = 0;
  };

  template<size_t T, size_t F>
  static void fiber_main(void* data) {
    using pool_t = ThreadPool<T, F>;
    using worker_t = typename pool_t::Worker;
    worker_t* worker = GET_HOST_WORKER(pool_t);
    //PRINT("worker %llu entered fiber main\n", worker->get_thread_id());
    Job job = GET_THREAD_POOL(pool_t)->pull_job(worker->get_thread_id());
    job.set_obj((void*)GET_THREAD_POOL(pool_t));
    if(!job.run()) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1ms);
    }
    worker = GET_HOST_WORKER(pool_t);
    worker->yield();
  }

  template<size_t NumThreads, size_t NumFibers>
  ThreadPool<NumThreads, NumFibers> ThreadPool<NumThreads, NumFibers>::_instance{};
}		// -----  end of namespace lofi  ----- 
