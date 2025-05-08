// =====================================================================================
//
//       Filename:  l_sync.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-11 11:45:44 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#include <atomic>
#include <thread>
#include "l_base.hpp"
#include "l_vocab.hpp"

namespace lofi {

  template<u32 Attempts>
  class spin_lock {
  public:
    void lock() {
      u32 i = 0;
      while(_flag.test_and_set(std::memory_order_acquire)) {
        if(i++ == Attempts) {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(10ns);
          i = 0;
        }
      }
    }

    void unlock() {
      _flag.clear();
    }

  private:
    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
  };

  template<u64 Max = MAX_u64>
  class atomic_counter {
  public:
    using count_t = meta::choose_index_type_t<Max>;

    atomic_counter() {};
    atomic_counter(count_t initial_count) : count{initial_count} {};

    void operator=(count_t new_count) {
      return count.store(new_count, std::memory_order_release);
    }

    void reset() {
      count.store(0, std::memory_order_release);
    }

    count_t clear() {
      auto current = count.load();
      while(!count.compare_exchange_weak(current, 0, std::memory_order_acq_rel)) {
      }
      return current;
    }

    const count_t get_count() const {
      return count.load(std::memory_order_acquire);
    }

    bool operator==(count_t c) {
      return count == c;
    }

    count_t operator++() {
      count_t val = count.fetch_add(1);
      return val;
    }

    count_t operator--() {
      count_t val = count.fetch_sub(1);
      return val;
    }


    count_t operator++(int) {
      count_t cur = 1 + count.fetch_add(1);
      return cur;
    }

    count_t operator--(int) {
      count_t cur = count.fetch_sub(1) - 1;
      return cur;
    }

    count_t add(count_t amount) {
      count_t cur = count.fetch_add(amount);
      return cur + amount;
    }

    count_t sub(count_t amount) {
      count_t cur = count.fetch_sub(amount);
      return cur - amount;
    }

  private:
    std::atomic<count_t> count{0};
  };

  template<size_t Takes>
  class semaphore {
  private:
    atomic_counter<Takes> counter{0};
    void sleep() {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1ns);
    }
    using count_t = typename atomic_counter<Takes>::count_t;
  public:
    semaphore() {}; 
    bool can_take() {
      if(counter.get_count() >= Takes) {
        return false;
      }
      return true;
    }

    b8 try_take() {
      const b8 result = counter++ < Takes;
      if(result == false) {
        counter--;
      }
      return result;
    }

    count_t spin_take() {
      count_t cur = 0;
      count_t result{};
      while((result = counter.get_count()) >= Takes) {
        if(cur < 8) {
          cur++;
        } else {
          sleep();
          cur = 0;
        }
      }
      return result;
    }

    void give() {
      counter.decrement();
    };

    void reset() {
      counter.reset();
    }
  };

//  based on fedor pikus'
//  never did get it working
//  template <size_t N, typename T>
//  class SyncQueue {
//    using queue_t = mem::ArrayContainerPolicy<T, N, 8, mem::StackAllocPolicy>;
//  public:
//    // ====================  TYPEDEFS      =======================================
//
//    // ====================  LIFECYCLE     =======================================
//    SyncQueue() {
//      _queue.push(N);
//    };                           // constructor      
//    ~SyncQueue() {};                          // destructor       
//
//    // ====================  MUTATORS      ======================================= 
//    T pop_front() {
//      static_assert(!((N - 1) & N));
//      T result{};
//      count_t front = _front.load(std::memory_order_relaxed);
//      if(front.equals(_back)) return FWD(result);
//      while(!front.equals(_front) ||
//        !_front.compare_exchange_weak(front, {(front.front + 1) & (N - 1), front.back}
//        , std::memory_order_acquire, std::memory_order_relaxed)) 
//      {}
//      _counter.decrement();
//      result = _queue[front.front];
//      return result;
//    }
//
//    void enqueue_back(T&& task) {
//      static_assert(!((N - 1) & N));
//      count_t back = _back.load(std::memory_order_relaxed);
//      while(!back.equals(_back) ||
//        !_back.compare_exchange_weak(back, {back.front, (back.back + 1) & (N - 1)}
//        , std::memory_order_acquire, std::memory_order_relaxed)) 
//      {}
//      _counter.increment();
//      while(_counter.get_count() >= N) {};
//      _queue[back.back] = FWD(task);
//    }
//
//    void clear() {
//      _front.store({0, 0});
//      _back.store({0, 0});
//    }
//
//  private:
//    // ====================  TYPEDEFS      =======================================
//    struct count_t {
//      u32 front = 0;
//      u32 back = 0;
//
//      bool equals(std::atomic<count_t>& n) {
//        if(front == back) return true;
//        *this = n.load(std::memory_order_relaxed);
//        return false;
//      }
//    };
//    // ====================  METHODS       ======================================= 
//
//    // ====================  DATA MEMBERS  ======================================= 
//    queue_t _queue;
//    lofi::atomic_counter<N> _counter;
//    std::atomic<count_t> _front;
//    std::atomic<count_t> _back;
//
//  }; // -----  end of template class TaskQueue  ----- 

  template<typename T>
  struct lock_free_stack_node {
    using type = T;
    T data;
    lock_free_stack_node* next = nullptr;
    bool to_delete = false;
  };

  template<typename T>
  struct deletion_node {
    T data;
    bool to_delete = false;
  };

  template<typename T>
  struct deletion_wait_node {
    T data;
    atomic_counter<>* counter{0};
    u64 count_to_wait = 0;
    bool to_delete = false;
    b8 check() {
      return counter->get_count() >= count_to_wait;
    }
  };

  template<typename T>
  struct lock_free_queue_node {
    T data;
    std::atomic<lock_free_queue_node*> next = nullptr;
    b8 to_delete = false;
    lock_free_queue_node() {};
    lock_free_queue_node(T t) : data{t} {}
    lock_free_queue_node(const T& t) : data{t} {}
    lock_free_queue_node(T&& t) : data{std::move(t)} {}
  };

//  template<typename T>
//  struct wait_list_node {
//    T data;
//    atomic_counter<>* counter = 0;
//    u64 count_to_wait = 0;
//
//    std::atomic<wait_list_node*> next = nullptr;
//    bool to_delete = false;
//    wait_list_node() {};
//    wait_list_node(T t) : data{t} {}
//    wait_list_node(const T& t) : data{t} {}
//    wait_list_node(T&& t) : data{std::move(t)} {}
//    wait_list_node(T t, atomic_counter<>* new_counter, u64 wait_for) : data{t}, counter{new_counter}, count_to_wait{wait_for} {}
//    wait_list_node(const T& t, atomic_counter<>* new_counter, u64 wait_for) : data{t}, counter{new_counter}, count_to_wait{wait_for} {}
//    wait_list_node(T&& t, atomic_counter<>* new_counter, u64 wait_for) : data{std::move(t)}, counter{new_counter}, count_to_wait{wait_for} {}
//
//    const b8 check() {
//      auto cur = counter->get_count();
//      if(cur >= counter->get_count()) {
//        if(!_flag.test_and_set(std::memory_order_acquire)) {
//          return true;
//        }
//        //PRINT("wait list check succeeded, cur = %llu, count_to_wait = %llu\n", cur, count_to_wait);
//        return false;
//      }
//      //PRINT("wait list check failed, cur = %llu, count_to_wait = %llu\n", cur, count_to_wait);
//      return false;
//    }
//
//    void clear() {
//      _flag.clear();
//    }
//
//    std::atomic_flag _flag = ATOMIC_FLAG_INIT;
//  };

  template<typename T>
  struct wait_list_node {
    T data;
    atomic_counter<>* counter{0};
    u64 count_to_wait = 0;
    std::atomic<wait_list_node*> next = nullptr;
    b8 to_delete = false;
    //std::atomic_flag _flag = ATOMIC_FLAG_INIT;

    wait_list_node(T t) : data{t} {}
    wait_list_node(const T& t) : data{t} {}
    wait_list_node(T&& t) : data{std::move(t)} {}
    wait_list_node(T t, atomic_counter<>* new_counter, u64 wait_for) : data{t}, counter{new_counter}, count_to_wait{wait_for} {}
    wait_list_node(const T& t, atomic_counter<>* new_counter, u64 wait_for) : data{t}, counter{new_counter}, count_to_wait{wait_for} {}
    wait_list_node(T&& t, atomic_counter<>* new_counter, u64 wait_for) : data{std::move(t)}, counter{new_counter}, count_to_wait{wait_for} {}

    const b8 check() {
      //PRINT_S("before get count\n");
      //PRINT("current counter %llu\n", PTR2INT(counter));
      //auto cur = counter->get_count();
      if(counter->get_count() >= count_to_wait) {
        //PRINT("wait list check succeeded, cur = %llu, count_to_wait = %llu\n", counter->get_count(), count_to_wait);
        return true;
      }
      //PRINT("wait list check failed, cur = %llu, count_to_wait = %llu\n", counter->get_count(), count_to_wait);
      return false;
    }

  };

//  template<typename T>
//  const b8 wait_list_node_check(wait_list_node<T>* node) {
//    if(!node) {
//      return false;
//    }
//      auto cur = node->counter->get_count();
//      if(cur >= node->counter->get_count()) {
//        if(!node->_flag.test_and_set(std::memory_order_acquire)) {
//          return true;
//        }
//        //PRINT("wait list check succeeded, cur = %llu, count_to_wait = %llu\n", cur, count_to_wait);
//        return false;
//      }
//      //PRINT("wait list check failed, cur = %llu, count_to_wait = %llu\n", cur, count_to_wait);
//      return false;
//  }

  template<typename T, size_t PoolSize>
  class lock_free_pool {
  private:
  public:
    using node = T;
    struct node_handle {
      union {
        node* handle = nullptr;
        node** next;
      };
    };
    lock_free_pool() {}

    lock_free_pool(void* ptr) {
      _pool = (T*)ptr;
      reset_elements();
    }

    node* get_ptr() {
      return _pool;
    }

    b8 set_ptr(void* ptr) {
      if(!_pool) {
        _pool = (T*)ptr;
        reset_elements();
        return true;
      }
      return false;
    }

    node* get_object() {
      node* old = top.load(std::memory_order_acquire);
      //PRINT("old top: %llu\n", old);
      if(!old) {
        return nullptr;
      }
      node** old_top = (node**)old;
      node* temp = *old_top;
      //PRINT("new top: %llu\n", temp);
      while(!top.compare_exchange_weak(old, temp, std::memory_order_release, std::memory_order_acquire)) {
        old_top = (node**)old;
        temp = *old_top;
      }
      if(old) {
        return (node*)old;
      }
      return nullptr;
    }

    bool return_object(node* ptr) {
      if(ptr >= &_pool[0] && ptr < &_pool[PoolSize]) {
        node* temp = top.load(std::memory_order_acquire);
        *((node**)ptr) = temp;
        while(!top.compare_exchange_weak(temp, ptr, std::memory_order_release, std::memory_order_acquire)) {
          *((node**)ptr) = temp;
        }
        return true;
      }
      return false;
    }

    bool return_object(void* ptr) {
      return return_object((node*)ptr);
    }


    b8 object_exists(node* ptr) {
      node* begin = top.load();
      for(; begin != nullptr; begin = *((node**)begin)) {
        if(begin == ptr) {
          return true;
        }
      }
      return false;
    }

    static constexpr size_t get_object_size() {
      return sizeof(T);
    }

    const b8 belongs(void* ptr) const {
      return ptr >= (void*)&_pool[0] && ptr < (void*)(&_pool[PoolSize]);
    }

  private:
    void reset_elements() {
      for(size_t i = 0; i < PoolSize; i++)
      {
        size_t next = i + 1;
        node** temp = (node**)(&_pool[i]);
        if(next == PoolSize) {
          *temp = nullptr;
          break;
        }
        *temp = &_pool[next];
      }
      top.store(&_pool[0], std::memory_order_release);
    }

    node* _pool = nullptr;
    std::atomic<node*> top = nullptr;
  };
 
  template<typename T>
  class lock_free_stack {
  private:
  public:
    using node = lock_free_stack_node<T>;
    void push(node* new_node) {
      new_node->next = top.load(std::memory_order_acquire);
      while(!top.compare_exchange_weak(new_node->next, new_node, std::memory_order_release)) {}
    }

    b8 pop(T* memory) {
      node* old_top = top.load(std::memory_order_acquire);
      while(old_top
            && !top.compare_exchange_weak(old_top, top.next)
            ) {}
      if(old_top) {
        *memory = old_top->data;
        old_top->to_delete = true;
        return true;
      }
      return false;
    }

    void clear() {
      top.store(nullptr, std::memory_order_release);
    }

  private:
    std::atomic<node*> top = nullptr;
  };
  
  template<typename T, size_t NumThreads = 4>
  class lock_free_queue {
  public:
    static_assert((NumThreads & (NumThreads - 1)) == 0);
    static constexpr size_t _ArrayMask = NumThreads - 1;
    struct node {
      T data;
      node* next = nullptr;
      b8 to_delete = false;
      node() : next{nullptr}, to_delete{false} {}
      void clear() {
        to_delete = false;
        next = nullptr;
      }
    };
  private:
    class guarded_list {
    private:
      node* first = nullptr;
      node* last = nullptr;
      std::atomic_flag _semaphore = ATOMIC_FLAG_INIT;
      atomic_counter<> counter{0};
    public:
      bool grab() {
        return !_semaphore.test_and_set(std::memory_order_acq_rel);
      }
    
      void give() {
        _semaphore.clear(std::memory_order_release);
      }
    
      b8 push(const node* const t) {
        ++counter;
        if(last) {
          last->next = (node*)t;
        } else {
          first = (node*)t;
        }
        last = (node*)t;
        return true;
      }

      b8 push(const node* const first_node, const node* const last_node, const size_t count) {
        counter.add(count);
        if(last) {
          last->next = (node*)first_node;
        } else {
          first = (node*)first_node;
        }
        last = (node*)last_node;
        return true;
      }

      void pop(node** new_node) {
        pop(new_node, new_node, 1);
      }

      void pop(node** first_ptr, node** last_ptr, const u64 count) {
        size_t i = 0;
        *first_ptr = first;
        while(first) {
          i++;
          *last_ptr = first;
          first = first->next;
          --counter;
          if(i == count) {
            break;
          }
        } if (!first) {
          last = nullptr;
        }
        return;
      }
  
      u32 get_size() {
        return counter.get_count();
      }
  
      b8 is_empty() {
        return counter.get_count() == 0;
      }

      void clear() {
        counter.clear();
        first = nullptr;
        last = nullptr;
      }
  
    };
  
    guarded_list thread_local_pools[NumThreads];
  public:

    lock_free_queue() {}

    void push(size_t thread_id, const node* new_node) {            // returns pointer to newly created node
      //PRINT("thread id = %llu in push\n", thread_id);
      for(size_t i = thread_id; i < NumThreads + thread_id; i++) {
        const size_t index = i & _ArrayMask;
        if(thread_local_pools[index].grab()) {
          //PRINT("thread id = %llu got pool %llu\n", thread_id, index);
          thread_local_pools[index].push((node*)new_node);
          thread_local_pools[index].give();
          break;
        }
        if(i == NumThreads + thread_id - 1) {
          //PRINT("thread %llu looping again, no pools found\n", thread_id);
          i = thread_id;
        }
      }
    }
  
    void push(size_t thread_id, const node* first, const node* last, const u32 count) {            // returns pointer to newly created node
      //PRINT("thread id = %llu in push\n", thread_id);
      for(size_t i = thread_id; i < NumThreads + thread_id; i++) {
        const size_t index = i & _ArrayMask;
        if(thread_local_pools[index].grab()) {
          //PRINT("thread id = %llu got pool %llu\n", thread_id, index);
          thread_local_pools[index].push(first, last, count);
          thread_local_pools[index].give();
          break;
        }
        if(i == NumThreads + thread_id - 1) {
          //PRINT("thread %llu looping again, no pools found\n", thread_id);
          i = thread_id;
        }
      }
    }
  
    b8 pop(size_t thread_id, T* memory) {
      // returns pointer to the newly
      node* new_node = nullptr;
      for(size_t i = thread_id; i < NumThreads + thread_id; i++) {
        const size_t index = i & _ArrayMask;
        if(thread_local_pools[index].is_empty()) {
          continue;
        }
        if(thread_local_pools[index].grab()) {
          //PRINT("thread id = %llu got pool %llu in pop\n", thread_id, index);
          thread_local_pools[index].pop(&new_node);
          thread_local_pools[index].give();
          break;
        }
      }
      if(new_node) {
        MEM_COPY(memory, &new_node->data, sizeof(T));
        new_node->to_delete = true;
        return true;
      }
      return false;
    }
  
  private:

  public:
    void clear() {
      for(size_t i = 0; i < NumThreads; i++) {
        while(!thread_local_pools[i].grab()) {}
        thread_local_pools[i].clear();
        thread_local_pools[i].give();
      }
    }
  };

//  template<typename T>
//  class lock_free_queue {
//  private:
//  public:
//    using node = lock_free_queue_node<T>;
//    void push(node* new_node) {            // returns pointer to newly created node
//      //PRINT_S("pushing to lock_free_queue\n");
//      node* old_tail = tail.load(std::memory_order_acquire);
//      node* null_node = nullptr;
//      if(!old_tail) {
//        if(tail.compare_exchange_strong(null_node, new_node, std::memory_order_acq_rel, std::memory_order_relaxed)) {
//          head.store(new_node, std::memory_order_release);
//          return;
//        }
//      }
//      null_node = nullptr;
//      while(
//          old_tail 
//          && !old_tail->next.compare_exchange_weak(null_node, new_node, std::memory_order_release)
//          ) {
//        null_node = nullptr;
//        old_tail = tail.load(std::memory_order_acquire);
//      }
//      tail.store(new_node, std::memory_order_release);
//    }
//  
//    b8 pop(T* memory) {                                   // returns pointer to the newly
//      //PRINT("incrementing threads in pop, current count = %llu\n", threads_in_pop.get_count());
//      threads_in_pop++;
//      //PRINT_S("entering pop\n");
//      node* old_head = head.load(std::memory_order_acquire);  // popped node
//      while(
//        old_head 
//        && !head.compare_exchange_weak(old_head, old_head->next, std::memory_order_release)
//        ) {}
//      if(old_head) {
//        node* dummy = old_head;
//        tail.compare_exchange_strong(dummy, nullptr, std::memory_order_release);
//        //PRINT_S("found T to pop\n");
//        MEM_COPY(memory, &old_head->data, sizeof(T));
//        try_reclaim(old_head);
//        return true;
//      }
//      //PRINT_S("did not find T to pop\n");
//      memory = nullptr;
//      threads_in_pop--;
//      return false;
//    }
//
//    void clear() {
//      head.store(nullptr, std::memory_order_release);
//      tail.store(nullptr, std::memory_order_release);
//    }
//  
//  private:
//    static void delete_nodes(node* nodes) {
//      while(nodes) {
//        node* next = nodes->next;
//        nodes->to_delete = true;
//        nodes = next;
//      }
//    }
//
//    void try_reclaim(node* old_head) {
//      //PRINT("in try reclaim, thread count = %llu\n", threads_in_pop.get_count());
//      if(threads_in_pop.get_count() == 1) {
//        //PRINT_S("1 thread found\n");
//        node* nodes_to_delete = to_be_deleted.exchange(nullptr);
//        //PRINT_S("decrementing threads_in_pop\n");
//        //PRINT("threads_in_pop decremented = %llu\n", cur);
//        if(threads_in_pop-- == 1) {
//          //PRINT_S("deleting nodes\n");
//          delete_nodes(nodes_to_delete);
//        } else if (nodes_to_delete) {
//          //PRINT_S("chaining nodes\n");
//          chain_pending_nodes(nodes_to_delete);
//        }
//        //PRINT_S("setting old_head to_delete\n");
//      } else {
//        //PRINT_S("more than one thread found, chaining pending nodes\n");
//        chain_pending_node(old_head);
//        //PRINT_S("decrementing threads_in_pop\n");
//        threads_in_pop--;
//        //PRINT("threads_in_pop decremented = %llu\n", cur);
//      }
//    }
//
//    void chain_pending_nodes(node* nodes) {
//      node* last = nodes;
//      while(node* const next = last->next)
//        last = next;
//      chain_pending_nodes(nodes, last);
//    }
//
//    void chain_pending_nodes(node* first, node* last) {
//      last->next.store(to_be_deleted);
//      auto tbd = last->next.load(std::memory_order_acquire);
//      while(!to_be_deleted.compare_exchange_weak(tbd, first, std::memory_order_release, std::memory_order_acquire));
//    }
//
//    void chain_pending_node(node* n) {
//      chain_pending_nodes(n, n);
//    }
//
//    atomic_counter<> threads_in_pop = 0;
//    std::atomic<node*> head = nullptr;
//    std::atomic<node*> tail = nullptr;
//    std::atomic<node*> to_be_deleted = nullptr;
//  };
//  template<typename T, size_t Capacity>
//  class lock_free_queue {
//    static_assert((Capacity & (Capacity - 1)) == 0);
//  public:
//    using node = deletion_node<T>;
//  private:
//    static constexpr size_t _Mask = Capacity - 1;
//    struct local_node {
//      std::atomic<i64> stat_index;
//      std::atomic<node*> data;
//    };
//    atomic_counter<> _num_written = 0;
//    atomic_counter<> _num_read = 0;
//    local_node _nodes[Capacity];
//  public:
//    lock_free_queue() {
//      reset_array();
//    }
//
//    void clear() {
//      _num_written = 0;
//      _num_read = 0;
//      reset_array();
//    }
//
//    void push(node* memory) {
//      i64 index;
//      i64 nindex;
//      do{
//        index = _num_written++;
//        nindex = ~index;
//      }
//      while(!_nodes[index & _Mask].stat_index.compare_exchange_strong(index, nindex, std::memory_order_relaxed));
//      node* old_data = _nodes[index & _Mask].data.exchange(memory);
//      if(old_data) {
//        old_data->to_delete = true;
//      }
//    }
//
//    b8 pop(T* memory) {
//      i64 index;
//      i64 nindex;
//      index = _num_read++;
//      if(index >= _num_written.get_count()) {
//        _num_read--;
//        return false;
//      }
//      nindex = ~index;
//      if(!_nodes[index & _Mask].stat_index.load(std::memory_order_acquire) == nindex) {
//        return false;
//      }
//      node* popped_node = _nodes[index & _Mask].data.exchange(nullptr);
//      if(popped_node) {
//        MEM_COPY(memory, &popped_node->data, sizeof(T));
//        popped_node->to_delete = true;
//        _nodes[index & _Mask].stat_index.store(index);
//        return true;
//      }
//      return false;
//    }
//
//  private:
//    void reset_array() {
//      for(size_t i = 0; i < Capacity; i++) {
//        _nodes[i].stat_index.store(i);
//        _nodes[i].data.store(nullptr);
//      }
//    }
//
//  };

//  template<typename T>
//  class lock_free_queue {
//  public:
//    using node = lock_free_queue_node<T>;
//  private:
//    std::atomic<node*> to_be_pushed = nullptr;
//    std::atomic<node*> to_be_deleted = nullptr;
//    std::atomic<node*> prod_head = nullptr;
//    std::atomic<node*> prod_tail = nullptr;
//    std::atomic<node*> cons_head = nullptr;
//    std::atomic<node*> cons_tail = nullptr;
//    atomic_counter<> pop_counter = 0;
//    atomic_counter<> push_counter = 0;
//    std::atomic_flag is_empty = ATOMIC_FLAG_INIT;
//  public:
//    void push(node* new_node) {
//
//      push_counter++;
//      node* old_prod_head = prod_head.load(std::memory_order_relaxed);
//      node* new_prod_head = new_node;
//      new_prod_head->next = nullptr;
//      while(!prod_head.compare_exchange_weak(old_prod_head, new_prod_head, std::memory_order_relaxed, std::memory_order_relaxed)) {}
//      old_prod_head->next = new_prod_head;
//      if(prod_tail.compare_exchange_strong(old_prod_head, new_node, std::memory_order_release, std::memory_order_relaxed)) {
//        if(!is_empty.test_and_set()) {
//          cons_head.store(new_node, std::memory_order_release);
//        }
//        push_counter--;
//        return;
//      }
//      try_append(new_node);
//      return;
//    }
//
//    b8 pop(T* memory) {
//      pop_counter++;
//      node* old_cons_head = cons_head.load(std::memory_order_relaxed);
//      node* old_prod_tail = prod_tail.load(std::memory_order_relaxed);
//      node* new_cons_head;
//      do {
//        if(!old_cons_head) {
//          return false;
//        }
//        new_cons_head = old_cons_head->next;
//      }
//      while(!cons_head.compare_exchange_weak(old_cons_head, new_cons_head, std::memory_order_relaxed, std::memory_order_relaxed));
//      MEM_COPY(memory, &old_cons_head->data, sizeof(T));
//      if(old_prod_tail == old_cons_head) {
//        new_cons_head = nullptr;
//      }
//      if(cons_tail.compare_exchange_strong(old_cons_head, new_cons_head, std::memory_order_release)) {
//      }
//      try_reclaim(old_cons_head);
//      return true;
//    }
//  private:
//    void delete_nodes(node* nodes) {
//      node* last = nodes;
//      while(const node* next = last->next) {
//        last->to_delete = true;
//        last = next;
//      }
//      while(!cons_tail.compare_exchange_weak(nodes, last, std::memory_order_release, std::memory_order_relaxed)) 
//      {}
//    }
//
//    void try_reclaim(node* old_head) {
//      if(pop_counter.get_count() == 1) {
//        node* nodes_to_delete = to_be_deleted.exchange(nullptr);
//        if(pop_counter-- == 1) {
//          delete_nodes(nodes_to_delete);
//        } else if (nodes_to_delete) {
//          chain_pending_nodes(nodes_to_delete);
//        }
//        cons_tail.store(old_head, std::memory_order_release);
//        old_head->to_delete = true;
//      } else {
//        chain_pending_node(old_head);
//        pop_counter--;
//      }
//    }
//
//    void chain_pending_nodes(node* nodes) {
//      node* last = nodes;
//      while(node* const next = last->next)
//        last = next;
//      chain_pending_nodes(nodes, last);
//    }
//
//    void chain_pending_nodes(node* first, node* last) {
//      last->next.store(to_be_deleted);
//      auto tbd = last->next.load();
//      while(!to_be_deleted.compare_exchange_weak(tbd, first));
//    }
//
//    void chain_pending_node(node* n) {
//      chain_pending_nodes(n, n);
//    }
//
//
//    void try_append(node* new_node) {
//      if(push_counter.get_count() == 1) {
//        node* nodes_to_append = to_be_pushed.exchange(nullptr);
//        if(push_counter-- == 1) {
//          nodes_to_append->next = new_node;
//          prod_tail.store(new_node, std::memory_order_release);
//          return;
//        } else if(nodes_to_append) {
//          chain_append_nodes(nodes_to_append);
//        }
//      } 
//      chain_append_node(new_node);
//      push_counter--;
//    }
//
//    void chain_append_nodes(node* first, node* last) {
//      auto nodes = to_be_pushed.load(std::memory_order_relaxed);
//      do {
//        nodes->next = first;
//      }
//      while(!to_be_pushed.compare_exchange_weak(nodes, last, std::memory_order_release, std::memory_order_acquire));
//    }
//
//    void chain_append_nodes(node* nodes) {
//      node* last = nodes;
//      while(node* const next = last->next)
//        last = next;
//      chain_append_nodes(nodes, last);
//    }
//
//    void chain_append_node(node* first) {
//      chain_append_nodes(first, first);
//    }
//
//    
//
//  };
//
//  template<typename T>
//  class wait_list {
//  private:
//  public:
//    using node = wait_list_node<T>;
//
//    void push(node* new_node) {            // returns pointer to newly created node
//      node* old_head = head.load(std::memory_order_acquire);
//      new_node->next = old_head;
//      while(
//          !head.compare_exchange_weak(old_head, new_node, std::memory_order_release)) {
//        new_node->next = old_head;
//      }
//    }
//  
//    b8 check_and_pop(T* memory) {                            // returns pointer to the newly
//      //PRINT("incrementing threads in wait list pop, current count = %llu\n", threads_in_pop.get_count());
//      threads_in_pop++;
//      //PRINT_S("loading head\n");
//      node* node_to_check = head.load(std::memory_order_acquire);  // popped node
//      node* prev = nullptr;
//      while(node_to_check) {
//        if(prev) {
//          while(!prev->next.compare_exchange_weak(node_to_check, node_to_check->next, std::memory_order_release)) {
//          }
//        } else {
//          prev = node_to_check;
//        }
//        while(!head.compare_exchange_weak(node_to_check, node_to_check->next, std::memory_order_release)) {
//        }
//        if(wait_list_node_check(node_to_check)) {
//          MEM_COPY(memory, &node_to_check->data, sizeof(T));
//          try_reclaim(node_to_check);
//          return true;
//        }
//        node_to_check = node_to_check->next.load(std::memory_order_acquire);
//      }
//      threads_in_pop--;
//      return false;
//    }
//
//    void clear() {
//      head.store(nullptr, std::memory_order_release);
//    }
//  
//  private:
//    static void delete_nodes(node* nodes) {
//      while(nodes) {
//        node* next = nodes->next;
//        nodes->to_delete = true;
//        nodes = next;
//      }
//    }
//
//    void try_reclaim(node* old_head) {
//      //PRINT("in wait_list try reclaim, thread count = %llu\n", threads_in_pop.get_count());
//      if(threads_in_pop.get_count() == 1) {
// //       PRINT_S("1 thread found\n");
//        node* nodes_to_delete = to_be_deleted.exchange(nullptr);
// //       PRINT_S("decrementing threads_in_pop\n");
//        threads_in_pop--;
// //       PRINT("threads_in_pop decremented = %llu\n", cur);
//        if(!threads_in_pop.get_count()) {
// //         PRINT_S("deleting nodes\n");
//          delete_nodes(nodes_to_delete);
//        } else if (nodes_to_delete) {
// //         PRINT_S("chaining nodes\n");
//          chain_pending_nodes(nodes_to_delete);
//        }
// //       PRINT_S("setting old_head to_delete\n");
//        old_head->to_delete = true;
//      } else {
// //       PRINT_S("more than one thread found, chaining pending nodes\n");
//        chain_pending_node(old_head);
// //       PRINT_S("decrementing threads_in_pop\n");
//        threads_in_pop--;
// //       PRINT("threads_in_pop decremented = %llu\n", cur);
//      }
//    }
//
//    void chain_pending_nodes(node* nodes) {
//      node* last = nodes;
//      while(node* const next = last->next)
//        last = next;
//      chain_pending_nodes(nodes, last);
//    }
//
//    void chain_pending_nodes(node* first, node* last) {
//      last->next.store(to_be_deleted);
//      auto tbd = last->next.load();
//      while(!to_be_deleted.compare_exchange_weak(tbd, first));
//    }
//
//    void chain_pending_node(node* n) {
//      chain_pending_nodes(n, n);
//    }
//
//    atomic_counter<> threads_in_pop = 0;
//    std::atomic<node*> head = nullptr;
//    std::atomic<node*> to_be_deleted = nullptr;
//  };

  template<typename T, size_t NumThreads = 4>
  class wait_list {
  public:
    static_assert((NumThreads & (NumThreads - 1)) == 0);
    static constexpr size_t _ArrayMask = NumThreads - 1;
    struct node {
      T data;
      std::atomic<node*> next = nullptr;
      atomic_counter<>* counter{0};
      u64 count_to_wait = 0;
      b8 to_delete = false;
      node() : next{nullptr}, to_delete{false} {}
    };
  private:
    class guarded_list {
    private:
      node* first = nullptr;
      node* last = nullptr;
      std::atomic_flag _semaphore = ATOMIC_FLAG_INIT;
      atomic_counter<> counter{0};
    public:
      bool grab() {
        return !_semaphore.test_and_set(std::memory_order_acq_rel);
      }
    
      void give() {
        _semaphore.clear(std::memory_order_release);
      }
    
      b8 push(node* t) {
        counter++;
        if(last) {
          last->next = t;
        } else {
          first = t;
        }
        last = t;
        return true;
      }

      b8 push(node* first_node, node* last_node, size_t count) {
        counter.add(count);
        if(last) {
          last->next.store(first_node);
        } else {
          first = first_node;
        }
        last = last_node;
        return true;
      }

      size_t exchange(node** first_ptr, node** last_ptr) {
        *first_ptr = first;
        first = nullptr;
        if(*first_ptr) {
          *last_ptr = last;
          last = nullptr;
          return counter.clear();
        }
        return 0;
      }
  
      u32 get_size() {
        return counter.get_count();
      }
  
      b8 is_empty() {
        return counter.get_count() == 0;
      }
  
    };
  
    guarded_list thread_local_pools[2][NumThreads];
  public:
    wait_list() {
      //threads_in_push = 0;
      //threads_in_pop = 0;
    }
    void push(size_t thread_id, node* new_node) {
      push(thread_id, new_node, new_node, 1);
    }
    void push(size_t thread_id, node* first_node, node* last_node, size_t count) {            // returns pointer to newly created node
      if(!first_node) {
        return;
      }
      threads_in_push.fetch_add(1, std::memory_order_release);
      //PRINT("thread id = %llu pushing node in wait list\n", thread_id);
      for(size_t i = thread_id; i < NumThreads + thread_id; i++) {
        const size_t index = i & _ArrayMask;
        u8 current_write_buffer = get_write_buffer();
        if(thread_local_pools[current_write_buffer][index].grab()) {
          thread_local_pools[current_write_buffer][index].push(first_node, last_node, count);
          thread_local_pools[current_write_buffer][index].give();
          break;
        }
        if(i == NumThreads + thread_id - 1) {
          //PRINT("thread %llu looping again, no pools found\n", thread_id);
          i = thread_id;
        }
      }
      
      try_append(thread_id);
    }

    b8 check_and_pop(size_t thread_id, T* memory) {
      size_t count = 0;
      node* node_to_check = nullptr;
      //PRINT("thread %llu in check and pop\n", thread_id);
      if(pop(thread_id, &node_to_check)) {
        //PRINT("thread %llu found node to check\n", thread_id);
        if(node_to_check->counter->get_count() >= node_to_check->count_to_wait) {
          //PRINT("thread %llu check succeeded\n", thread_id);
          MEM_COPY(memory, &node_to_check->data, sizeof(T));
          try_reclaim(thread_id, node_to_check);
          return true;
        }
        threads_in_pop.fetch_sub(1);
        push(thread_id, node_to_check);
        return false;
      } 
      threads_in_pop.fetch_sub(1);
      //PRINT("thread %llu exiting check and pop\n", thread_id);
      return false;
    }
 
  private:
    b8 pop(size_t thread_id, node** memory) {                                   // returns pointer to the newly
      threads_in_pop.fetch_add(1);
      //PRINT("thread id = %llu entered lock free pop\n", thread_id);
      //PRINT("thread id = %llu, %llu current threads in pop\n", thread_id, tip);
      node* old_head = head.load(std::memory_order_acquire);  // popped node
      if(!old_head) {
        threads_in_pop.fetch_sub(1);
        return false;
      }
      node* next_node = old_head->next.load(std::memory_order_acquire);
      while(
        old_head 
        && !head.compare_exchange_weak(old_head, next_node, std::memory_order_release, std::memory_order_acquire)
        ) {
        if(old_head) {
          next_node = old_head->next.load(std::memory_order_acquire);
        }
      }
      if(old_head) {
        //PRINT("thread id = %llu found node to pop\n", thread_id);
        *memory = old_head;
        //PRINT("thread id = %llu trying reclaim\n", thread_id);
        return true;
      }
      threads_in_pop.fetch_sub(1);
      //PRINT("thread id = %llu did not find node to pop\n", thread_id);
      return false; 
    }
 
    void try_append(size_t thread_id) {
      if(threads_in_push.load(std::memory_order_acquire) == 1) {
        switch_buffer();
        u8 current_read_buffer = get_read_buffer();
        size_t count = 0;
        node* first_to_append = nullptr; 
        node* last_to_append = nullptr;
        for(size_t i = 0; i < NumThreads; i++) {
          node* first = nullptr;
          node* last = nullptr;
          if(thread_local_pools[current_read_buffer][i].is_empty()) {
            continue;
          }
          while(!thread_local_pools[current_read_buffer][i].grab()) {}
          size_t local_count = 0;
          if((local_count = thread_local_pools[current_read_buffer][i].exchange(&first, &last))) {
            if(!first_to_append) {
              first_to_append = first;
            } if(last_to_append) {
              last_to_append->next = first;
            }
            last_to_append = last;
            count += local_count;
          }
          thread_local_pools[current_read_buffer][i].give();
        }
        node* old_head = head.load();
        if(!old_head && head.compare_exchange_strong(old_head, first_to_append, std::memory_order_release)) {
        }
        node* old_tail = tail.load();
        tail.store(last_to_append);
        if(old_tail) {
          old_tail->next.store(first_to_append);
        }
      }
      threads_in_push.fetch_sub(1, std::memory_order_release);
    }

    static void delete_nodes(node* nodes) {
      while(nodes) {
        node* next = nodes->next;
        nodes->to_delete = true;
        nodes = next;
      }
    }
  
    void try_reclaim(size_t thread_id, node* old_head) {
      //PRINT("thread id = %llu in try reclaim, thread count = %llu\n", thread_id, cur);
      if(threads_in_pop.load() == 1) {
        //PRINT("thread id = %llu passed check, thread count = %llu\n", thread_id, cur);
        node* nodes_to_delete = to_be_deleted.exchange(nullptr, std::memory_order_release);
        if(threads_in_pop.fetch_sub(1) == 1) {
          //PRINT("thread id = %llu, count %llu, deleting nodes\n", thread_id, cur);
          delete_nodes(nodes_to_delete);
        } else if (nodes_to_delete) {
          //PRINT("thread id = %llu, count %llu, chaining nodes\n", thread_id, cur);
          chain_pending_nodes(nodes_to_delete);
        }
          //PRINT("thread id = %llu deleting node, thread count = %llu\n", thread_id, cur);
        old_head->to_delete = true;
      } else { 
      //PRINT("thread id = %llu, count %llu, chaining 1 node\n", thread_id, cur);
        chain_pending_node(old_head);
        threads_in_pop.fetch_sub(1);
      //PRINT("thread id = %llu exiting try reclaim, thread count = %llu\n", thread_id, cur);
      }
    }
  
    void chain_pending_nodes(node* nodes) {
      node* last = nodes;
      while(node* const next = last->next)
        last = next;
      chain_pending_nodes(nodes, last);
    }
  
    void chain_pending_nodes(node* first, node* last) {
      last->next.store(to_be_deleted, std::memory_order_release);
      auto tbd = last->next.load(std::memory_order_acquire);
      while(!to_be_deleted.compare_exchange_weak(tbd, first, std::memory_order_release, std::memory_order_acquire));
    }
  
    void chain_pending_node(node* n) {
      chain_pending_nodes(n, n);
    }

    void switch_buffer() {
      u8 old_buffer = buffer_index.load();
      u8 new_buffer = !old_buffer;
      buffer_index.compare_exchange_strong(old_buffer, new_buffer);
    }

    u8 get_read_buffer() {
      return buffer_index.load(std::memory_order_acquire);
    }
 
    u8 get_write_buffer() {
      return !buffer_index.load(std::memory_order_acquire);
    }
    std::atomic<u64> threads_in_pop = 0;
    std::atomic<u64> threads_in_push = 0;
    std::atomic<node*> to_be_deleted = nullptr;
    std::atomic<node*> head = nullptr;
    std::atomic<node*> tail = nullptr;
    std::atomic<u8> buffer_index = 0;
  public:
    void clear() {
      head.store(nullptr, std::memory_order_release);
      tail = nullptr;
    }
  };

//  template<typename T, size_t Capacity>
//  class wait_list {
//    static_assert((Capacity & (Capacity - 1)) == 0);
//  public:
//    using node = deletion_wait_node<T>;
//  private:
//    static constexpr size_t _Mask = Capacity - 1;
//    struct local_node {
//      std::atomic<i64> stat_index;
//      std::atomic<node*> data;
//    };
//    atomic_counter<> _num_written = 0;
//    atomic_counter<> _num_read = 0;
//    local_node _nodes[Capacity];
//  public:
//    wait_list() {
//      reset_array();
//    }
//
//    void clear() {
//      _num_written = 0;
//      _num_read = 0;
//      reset_array();
//    }
//
//    void push(node* memory) {
//      i64 index;
//      i64 nindex;
//      do{
//        index = _num_written++;
//        nindex = ~index;
//      }
//      while(!_nodes[index & _Mask].stat_index.compare_exchange_strong(index, nindex, std::memory_order_relaxed));
//      node* old_data = _nodes[index & _Mask].data.exchange(memory);
//      if(old_data) {
//        old_data->to_delete = true;
//      }
//    }
//
//    b8 check_and_pop(T* memory) {
//      node* node_to_check = nullptr;
//      i64 index = MAX_i64;
//      index = pop(node_to_check); 
//      i64 first = MAX_i64;
//      if(index != MAX_i64) {
//        if(first == MAX_i64) {
//          first = index &_Mask;
//        }
//        if(node_to_check->check()) {
//          MEM_COPY(memory, &node_to_check->data, sizeof(T));
//          node_to_check->to_delete = true;
//          _nodes[index & _Mask].stat_index.store(index);
//          return true;
//        }
//        push(node_to_check);
//      }
//      return false;
//    }
//
//  private:
//    b8 pop(node* memory) {
//      i64 index;
//      i64 nindex;
//      if((index = _num_read.get_count()) >= _num_written.get_count()) {
//        return index;
//      }
//      index = index == _num_read++ ? index : MAX_i64;
//      nindex = ~index;
//      if(!_nodes[index & _Mask].stat_index.load(std::memory_order_acquire) == nindex) {
//        _num_read--;
//        return MAX_i64;
//      }
//      memory = _nodes[index & _Mask].data.exchange(nullptr);
//      if(memory) {
//        return index;
//      }
//      return MAX_i64;
//    }
//
//  private:
//    void reset_array() {
//      for(size_t i = 0; i < Capacity; i++) {
//        _nodes[i].stat_index.store(i);
//        _nodes[i].data.store(nullptr);
//      }
//    }
//
//  };


//  template<typename T, size_t NumThreads = 4, size_t GuardedThreadSize = 64>
//  class wait_list {
//  public:
//    using node = wait_list_node<T>;
//    b8 check_and_pop(size_t thread_id, T* memory) {
//      //PRINT("thread %llu entered check and pop\n", thread_id);
//      node* node_to_check = nullptr;
//      // ERROR IS HERE!!****
//      if(pop(&node_to_check)) {
//        //PRINT("thread %llu found node, checking...\n", thread_id);
//        if(node_to_check->check()) {
//          //PRINT("thread %llu check and pop succeeded!\n", thread_id);
//          MEM_COPY(memory, &node_to_check->data, sizeof(T));
//          try_reclaim(node_to_check);
//          return true;
//        }
//
//        push(thread_id, node_to_check);
//        threads_in_pop--;
//      }
//      //PRINT("thread %llu no nodes in wait_list\n", thread_id);
//      return false;
//    }
//
//  private:
//    class guarded_array {
//    private:
//      node* _array[GuardedThreadSize];
//      std::atomic_flag _semaphore = ATOMIC_FLAG_INIT;
//      atomic_counter<> counter = 0;
//    public:
//      bool grab() {
//        return !_semaphore.test_and_set(std::memory_order_acq_rel);
//      }
//    
//      void give() {
//        _semaphore.clear(std::memory_order_release);
//      }
//    
//      b8 push(node* t) {
//        if(counter == GuardedThreadSize) {
//          return false;
//        }
//        _array[counter++] = t;
//        return true;
//      }
//    
//      node* pop() {
//        return _array[--counter];
//      }
//  
//      u32 get_size() {
//        return counter.get_count();
//      }
//  
//      b8 is_empty() {
//        return counter.get_count() == 0;
//      }
//  
//      b8 is_full() {
//        return counter.get_count() >= GuardedThreadSize;
//      }
//    };
//  
//    guarded_array thread_local_pools[NumThreads];
//  public:
//    void push(size_t thread_id, node* new_node) {            // returns pointer to newly created node
//      if(!_push_semaphore.test_and_set(std::memory_order_acq_rel)) {
//        //PRINT("thread id = %llu got semaphore\n", thread_id);
//        node* last_node = new_node;
//        for(size_t i = 0; i < NumThreads; i++) {
//          if(thread_local_pools[i].is_empty()) {
//              //PRINT("thread %llu pool %llu is empty in main push\n", thread_id, i);
//              continue;
//          }
//          if(thread_local_pools[i].grab()) {
//            //PRINT("thread %llu grabbing pool %llu in main push\n", thread_id, i);
//            const size_t pool_size = thread_local_pools[i].get_size();
//            for(size_t j = 0; j < pool_size; j++) {
//              //PRINT("thread %llu popping %llu from pool %llu\n", thread_id, j, i);
//              node* next_node = thread_local_pools[i].pop();
//              last_node->next = next_node;
//              //PRINT("thread %llu setting last %llu from pool %llu\n", thread_id, j, i);
//              last_node = next_node;
//            }
//            //PRINT("thread %llu from pool %llu\n", thread_id, i);
//            thread_local_pools[i].give();
//          }
//        }
//        //PRINT("thread %llu proceeding to push in main\n", thread_id);
//        node* null_node = nullptr;
//        //PRINT("thread %llu tail copied\n", thread_id);
//        //PRINT("thread %llu comparing head with null\n", thread_id);
//        if(head.compare_exchange_strong(null_node, new_node, std::memory_order_release, std::memory_order_acquire)) {
//          //PRINT("thread %llu no old head = nullptr\n", thread_id);
//        } else {
//          //PRINT("thread %llu tail next storing\n", thread_id);
//          tail->next.store(new_node, std::memory_order_release);
//          //PRINT("thread %llu tail next stored\n", thread_id);
//        }
//        tail = last_node;
//        //PRINT("thread %llu returning semaphore\n", thread_id);
//        _push_semaphore.clear(std::memory_order_release);
//        return;
//      }
//  
//      for(size_t i = 0; i < NumThreads; i++) {
//        if(thread_local_pools[i].is_full()) {
//          //PRINT("thread id = %llu pool %llu is full\n", thread_id, i);
//          continue;
//        }
//        //PRINT("thread id = %llu pool %llu has room\n", thread_id, i);
//        if(thread_local_pools[i].grab()) {
//          //PRINT("thread id = %llu pool %llu is open\n", thread_id, i);
//          if(thread_local_pools[i].push(new_node)) {
//            //PRINT("thread id = %llu pool %llu returning semaphore after push\n", thread_id, i);
//            thread_local_pools[i].give();
//            return;
//          }
//          //PRINT("thread id = %llu pool %llu returning semaphore without push\n", thread_id, i);
//          thread_local_pools[i].give();
//        }
//        if(i == NumThreads - 1) {
//          //PRINT("thread id = %llu looping again, no pools found\n", thread_id);
//          i = 0;
//        }
//      }
//      //PRINT("thread id = %llu hitting assert...\n", thread_id);
//      L_ASSERT(false && "code should always complete");
//    }
//  private: 
//    b8 pop(node** memory) {                                   // returns pointer to the newly
//      auto tip = threads_in_pop++;
//      *memory = head.load(std::memory_order_acquire);  // popped node
//      while(
//        *memory 
//        && !head.compare_exchange_weak((*memory), (*memory)->next, std::memory_order_release)
//        ) {}
//      if((*memory)) {
//        return true;
//      }
//      tip = threads_in_pop--;
//      return false; 
//    }
//  
//    static void delete_nodes(node* nodes) {
//      while(nodes) {
//        node* next = nodes->next;
//        nodes->to_delete = true;
//        nodes = next;
//      }
//    }
//  
//    void try_reclaim(node* old_head) {
//      //PRINT("in try reclaim, thread count = %llu\n", threads_in_pop.get_count());
//      auto cur = threads_in_pop.get_count();
//      if(cur == 1) {
//        node* nodes_to_delete = to_be_deleted.exchange(nullptr, std::memory_order_release);
//        if((cur = threads_in_pop--) == 1) {
//          delete_nodes(nodes_to_delete);
//        } else if (nodes_to_delete) {
//          chain_pending_nodes(nodes_to_delete);
//        }
//        if(!_push_semaphore.test(std::memory_order_acquire)) {
//          old_head->to_delete = true;
//          return;
//        }
//      }
//      chain_pending_node(old_head);
//      cur = threads_in_pop--;
//    }
//  
//    void chain_pending_nodes(node* nodes) {
//      node* last = nodes;
//      while(node* const next = last->next)
//        last = next;
//      chain_pending_nodes(nodes, last);
//    }
//  
//    void chain_pending_nodes(node* first, node* last) {
//      last->next.store(to_be_deleted, std::memory_order_release);
//      auto tbd = last->next.load(std::memory_order_acquire);
//      while(!to_be_deleted.compare_exchange_weak(tbd, first, std::memory_order_release, std::memory_order_acquire));
//    }
//  
//    void chain_pending_node(node* n) {
//      chain_pending_nodes(n, n);
//    }
//  
//    std::atomic_flag _push_semaphore = ATOMIC_FLAG_INIT;
//    atomic_counter<> threads_in_pop = 0;
//    std::atomic<node*> to_be_deleted = nullptr;
//    std::atomic<node*> head = nullptr;
//    node* tail = nullptr;
//
//    void clear() {
//      head.store(nullptr, std::memory_order_release);
//      tail = nullptr;
//    }
//  };
}		// -----  end of namespace lofi  ----- 
