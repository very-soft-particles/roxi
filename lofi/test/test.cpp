// =====================================================================================
//
//       Filename:  test.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-02 11:23:33 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vector>
#define LOFI_DEFAULT_BUCKETS_COUNT 4
#include "../core/include/l_tuple.hpp"
#include "../core/include/l_variant.hpp"
#include "../core/include/l_memory.hpp"
#include "../core/include/l_thread_pool.hpp"
#include "../core/include/l_database.hpp"
#include "../core/include/l_map.hpp"
#include "../core/include/ecs/l_ecs.hpp"


//#define DO_BIG
static constexpr u64 NumThreads = 4;

static constexpr u64 NumFibers = 128;

static u64 values[8192];

static u64 big_job_results[4];

DEFINE_JOB_SUCCESS(standard_job_success) {
  //PRINT("entering standard_job_success counter ptr = %llu\n", PTR2INT(counter));
  lofi::atomic_counter<>* a_counter = (lofi::atomic_counter<>*) counter;
  (*a_counter)++;
  //PRINT("success counter = %llu\n", (*a_counter).get_count());
}

DEFINE_JOB_FAILURE(print_job_failure) {
  //PRINT_S("print job failed!\n");
}

DEFINE_JOB(print_index) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = typename pool_t::Worker;

  pool_t* thread_pool = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();

  //PRINT("thread_id: %llu entering print_index job\n", worker->get_thread_id());
  if(start == end)
    return false;
  for(size_t print_job_index = start; print_job_index < end; print_job_index++) {
    values[print_job_index] = print_job_index;
    //EVAL_PRINT_ULL(print_job_index);
  }
  //PRINT("thread_id: %llu exiting print_index job\n", worker->get_thread_id());
  return true;
}

DEFINE_JOB(subinit_job) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = lofi::ThreadPool<NumThreads, NumFibers>::Worker;
  pool_t* thread_pool  = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();
  size_t thread_id = worker->get_thread_id();
  //PRINT("thread %llu entering init_job\n", thread_id);
  lofi::atomic_counter<> counter(0);
  //PRINT("init job counter ptr = %llu\n", PTR2INT(&counter));
  const size_t job_size = 256;
  const size_t num_jobs = 8;
  const size_t start_index = start * num_jobs * job_size;
  for(size_t i = 0; i < num_jobs; i++) {
    lofi::Job new_job;
    new_job.set_job_start(i * job_size + start_index);
    new_job.set_job_end((i * job_size) + job_size + start_index);
    new_job.set_entry_point(print_index);
    new_job.set_job_counter(&counter);
    new_job.set_job_success(standard_job_success);
    new_job.set_job_failure(print_job_failure);
    //PRINT("job entry point = %llu\n", PTR2INT((void*)new_job.entry_point));
    worker->kick_high_priority_job(new_job);
  }
  worker->fiber_wait(&counter, num_jobs);
  //worker = thread_pool->get_host_worker();
  //thread_id = worker->get_thread_id();
  //PRINT("thread %llu exiting after waking in init job\n", thread_id);
  return true;
}

DEFINE_JOB(big_job) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = lofi::ThreadPool<NumThreads, NumFibers>::Worker;
  pool_t* thread_pool  = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();
  size_t thread_id = worker->get_thread_id();

  const size_t Size = 100'000'000;
  size_t result = 0;

  for(size_t i = 0; i < Size; i++) {
    result += i;
  }

  big_job_results[start] = result;
  return true;
}

DEFINE_JOB(lil_init_job) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = lofi::ThreadPool<NumThreads, NumFibers>::Worker;
  pool_t* thread_pool  = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();
  size_t thread_id = worker->get_thread_id();
  //PRINT("thread %llu entering init_job\n", thread_id);
  lofi::atomic_counter<> counter{0};
  //PRINT("init job counter ptr = %llu\n", PTR2INT(&counter));
  const size_t job_size = 64;
  const size_t num_jobs = 4;
  const size_t start_index = start * num_jobs * job_size;
 
  for(size_t i = 0; i < num_jobs; i++) {
    lofi::Job new_job;
    new_job.set_job_start(i * job_size + start_index);
    new_job.set_job_end((i * job_size) + job_size + start_index);
    new_job.set_entry_point(print_index);
    new_job.set_job_counter(&counter);
    new_job.set_job_success(standard_job_success);
    new_job.set_job_failure(print_job_failure);
    //PRINT("job entry point = %llu\n", PTR2INT((void*)new_job.entry_point));
    worker->kick_high_priority_job(new_job);
  }
  worker->fiber_wait(&counter, num_jobs);
  //worker = thread_pool->get_host_worker();
  //thread_id = worker->get_thread_id();
  //PRINT("thread %llu exiting after waking in init job\n", thread_id);
  return true;
}


DEFINE_JOB(big_init_job) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = lofi::ThreadPool<NumThreads, NumFibers>::Worker;
  pool_t* thread_pool  = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();
  size_t thread_id = worker->get_thread_id();
  //PRINT("thread %llu entering init_job\n", thread_id);
  lofi::atomic_counter<> counter{0};
  //PRINT("init job counter ptr = %llu\n", PTR2INT(&counter));
  const size_t num_jobs = 4;
  for(size_t i = 0; i < num_jobs; i++) {
    lofi::Job new_job;
    new_job.set_job_start(i);
    new_job.set_job_end(i + 1);
    new_job.set_entry_point(subinit_job);
    new_job.set_job_counter(&counter);
    new_job.set_job_success(standard_job_success);
    new_job.set_job_failure(print_job_failure);
    //PRINT("job entry point = %llu\n", PTR2INT((void*)new_job.entry_point));
    worker->kick_high_priority_job(new_job);
  }
  for(size_t i = 0; i < num_jobs; i++) {
    lofi::Job new_job;
    new_job.set_job_start(i);
    new_job.set_job_end(i + 1);
    new_job.set_entry_point(big_job);
    new_job.set_job_counter(&counter);
    new_job.set_job_success(standard_job_success);
    new_job.set_job_failure(print_job_failure);
    //PRINT("job entry point = %llu\n", PTR2INT((void*)new_job.entry_point));
    worker->kick_high_priority_job(new_job);
  }
  worker->fiber_wait(&counter, num_jobs * 2);
  //worker = thread_pool->get_host_worker();
  //thread_id = worker->get_thread_id();
  //PRINT("thread %llu exiting after waking in init job\n", thread_id);
  return true;
}

using ExampleTableTuple = lofi::tuple<u64, u64, double>;
using ExampleTableDescriptor = lofi::TableDescriptor<32, u64, u64, double>;
using ExampleTable = lofi::Table< ExampleTableDescriptor
                                , 8
                                >;

using ExampleTableDescriptorSet = lofi::db::TableDescriptorSet<8, ExampleTableDescriptor, ExampleTableDescriptor, ExampleTableDescriptor>;

struct ExampleCol0Predicate {
  b8 matches(ExampleTable& table, u64 index) {
    if(table.get_column<0>()[index] < 32) {
      return true;
    }
    return false;
  }
};

struct ExampleCol1Predicate {
  b8 matches(ExampleTable& table, u64 index) {
    if(table.get_column<1>()[index] > 32) {
      return true;
    }
    return false;
  }
};

struct ExampleCol2Predicate {
  b8 matches(ExampleTable& table, u64 index) {
    if(table.get_column<2>()[index] >= 1.f) {
      return true;
    }
    return false;
  }
};

struct ExampleComponent0 {
  u32 value;
};

struct ExampleComponent1 {
  u32 value;
};

struct ExampleComponent2 {
  u32 value;
};

struct ExampleTag0 {};


using ArchetypeDescriptor0 = lofi::ecs::ArchetypeDescriptor<128, ExampleComponent0, ExampleComponent1>;
using ArchetypeDescriptor1 = lofi::ecs::ArchetypeDescriptor<128, ExampleComponent1, ExampleComponent2>;
using ArchetypeDescriptor2 = lofi::ecs::ArchetypeDescriptor<128, ExampleComponent0, ExampleComponent2, ExampleTag0>;
using ArchetypeDescriptor3 = lofi::ecs::ArchetypeDescriptor<128, ExampleComponent0, ExampleComponent1, ExampleComponent2>;
using ArchetypeDescriptor4 = lofi::ecs::ArchetypeDescriptor<128, ExampleComponent0, ExampleComponent2>;
using ArchetypeDescriptor5 = lofi::ecs::ArchetypeDescriptor<128, ExampleComponent0, ExampleComponent1, ExampleComponent2, ExampleTag0>;

using comp0 = ExampleComponent0;
using comp1 = ExampleComponent1;
using comp2 = ExampleComponent2;

using component_list_t = lofi::List<comp0, comp1, comp2>;

using tag0 = ExampleTag0;

using tag_list_t = lofi::List<tag0>;

using arch0 = ArchetypeDescriptor0;
using arch1 = ArchetypeDescriptor1;
using arch2 = ArchetypeDescriptor2;
using arch3 = ArchetypeDescriptor3;
using arch4 = ArchetypeDescriptor4;
using arch5 = ArchetypeDescriptor5;

using archetype_list_t = lofi::List<arch0, arch1, arch2, arch3, arch4, arch5>;

using descriptor_t = lofi::ecs::ECSDescriptor<component_list_t, tag_list_t, archetype_list_t>;

template<>
struct lofi::ecs::ECSSettings<0> {
  using type = descriptor_t;
};

int main(int argc, char** argv) {
//--------------------------------------------------------------------------------------------
  PRINT_TITLE("OS AND COMPILER");
//--------------------------------------------------------------------------------------------
  if(OS_WINDOWS) {
    PRINT_S("WINDOWS\n");
  } else if (OS_LINUX) {
    PRINT_S("LINUX\n");
  } else if (OS_MAC) {
    PRINT_S("MAC\n");
  } else {
    PRINT_S("OS NOT FOUND\n");
  }
  if(COMPILER_CL) {
    PRINT_S("MSVC\n");
  } else if(COMPILER_CLANG) {
    PRINT_S("CLANG\n");
  } else if(COMPILER_GCC) {
    PRINT_S("GCC\n");
  } else {
    PRINT_S("COMPILER NOT FOUND\n");
  }
 
//--------------------------------------------------------------------------------------------
  PRINT_TITLE("meta::map");
//--------------------------------------------------------------------------------------------

  using Key0 = lofi::IdxT<1>;
  using Key1 = lofi::IdxT<3>;
  using Key2 = lofi::IdxT<4>;

  using Value0 = lofi::IdxT<8>;
  using Value1 = lofi::IdxT<2>;
  using Value2 = lofi::IdxT<3>;
  
  using KeyList = lofi::List<Key0, Key1, Key2>;
  using ValueList = lofi::List<lofi::IdxT<8>, lofi::IdxT<2>, lofi::IdxT<3>>;

  using my_map_t = typename lofi::meta::lift_t<lofi::meta::map_t<KeyList>::template type>::template type<ValueList>::apply;

  my_map_t my_map;

  auto map_0 = my_map.template get<Key0>();
  auto map_1 = my_map.template get<Key1>();
  auto map_2 = my_map.template get<Key2>();

  PRINT("meta::map value = %llu at key = %llu\n", map_0, Key0::value);
  PRINT("meta::map value = %llu at key = %llu\n", map_1, Key1::value);
  PRINT("meta::map value = %llu at key = %llu\n", map_2, Key2::value);

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("TUPLES");
//--------------------------------------------------------------------------------------------

  lofi::tuple<int, float, double, int> t0;
  lofi::tuple<float, double, int> t1;

  lofi::meta::get<0>(t0).elem = 4;
  lofi::meta::get<1>(t0).elem = 4.20f;
  lofi::meta::get<2>(t0).elem = 4.24;
  lofi::meta::get<3>(t0).elem = 420;

  PRINT("t0[0] = %d\n", lofi::meta::get<0>(t0).elem);
  PRINT("t0[1] = %f\n", lofi::meta::get<1>(t0).elem);
  PRINT("t0[2] = %f\n", lofi::meta::get<2>(t0).elem);
  PRINT("t0[3] = %d\n", lofi::meta::get<3>(t0).elem);

  t0.get<0>() = 69;
  t0.get<1>() = 6.9;
  t0.get<2>() = 6.969;
  t0.get<3>() = 6969;

  PRINT("t0[0] = %d\n", t0.get<0>());
  PRINT("t0[1] = %f\n", t0.get<1>());
  PRINT("t0[2] = %f\n", t0.get<2>());
  PRINT("t0[3] = %d\n", t0.get<3>());
  
  lofi::meta::get<0>(t1).elem = 44.20f;
  lofi::meta::get<1>(t1).elem = 4.20f;
  lofi::meta::get<2>(t1).elem = 424;

  PRINT("t1[0] = %f\n", lofi::meta::get<0>(t1).elem);
  PRINT("t1[1] = %f\n", lofi::meta::get<1>(t1).elem);
  PRINT("t1[2] = %d\n", lofi::meta::get<2>(t1).elem);

  t1.get<0>() = 69.69;
  t1.get<1>() = 6.9;
  t1.get<2>() = 6969;

  PRINT("t1[0] = %f\n", t1.get<0>());
  PRINT("t1[1] = %f\n", t1.get<1>());
  PRINT("t1[2] = %d\n", t1.get<2>());

  using Var0 = lofi::variant<f32, i32, u8, u16>;

  struct custom_type0 {
    u8 data[64];
    custom_type0() {
      MEM_ZERO_ARRAY(data);
    }

    custom_type0(const custom_type0& other) {
      MEM_COPY_ARRAY(data, other.data);
    }

    custom_type0(custom_type0&& other) {}

    //~custom_type0() {
    //  PRINT_S("destructor called for custom_type");
    //}
  };

  struct custom_type1 {};

  using Var1 = lofi::variant<f32, custom_type0, i32>;
  using Var2 = lofi::variant<f32, custom_type1, i32>;


  Var0 var00(2.5f);
  Var0 var01((i32)420);
  Var0 var02((u16)420);
  Var0 var03((u8)69);

  Var1 var10((f32)420.420f);
  Var1 var11((i32)6969);
  Var1 var12(custom_type0{});

  Var2 var20((i32)6969);
  Var2 var21(custom_type1{});
  Var2 var22((f32)6969.6969f);

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("VARIANTS");
//--------------------------------------------------------------------------------------------

  struct Visitor {
    using result_t = void;
    void operator()(f32&& f) {
      PRINT_S("dispatching visitor f32: ");
      EVAL_PRINT_F(f);
    }
    void operator()(i32&& i) {
      PRINT_S("dispatching visitor i32: ");
      EVAL_PRINT_I(i);
    }
    void operator()(u8&& i) {
      PRINT_S("dispatching visitor u8: ");
      EVAL_PRINT_I(i);
    }
    void operator()(u16&& i) {
      PRINT_S("dispatching visitor u16: ");
      EVAL_PRINT_I(i);
    }
    void operator()(custom_type0&& i) {
      PRINT_S("dispatching visitor ct0: ");
      PRINT("%s\n", "custom_type0{}");
    }
    void operator()(custom_type1&& i) {
      PRINT_S("dispatching visitor ct1: ");
      PRINT("%s\n", "custom_type1{}");
    }
  } vis;

  var00.visit(vis);
  var01.visit(vis);
  var02.visit(vis);
  var03.visit(vis);

  var10.visit(vis);
  var11.visit(vis);
  var12.visit(vis);

  var20.visit(vis);
  var21.visit(vis);
  var22.visit(vis);

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("MEMORY POOL");
//--------------------------------------------------------------------------------------------

  struct DataStruct0 {
    u8 data[64];
    u64 data2[64];
  };

  struct DataStruct1 {
    u64 data[64];
    u64 data2[64];
    u64 data3[64];
  };

  size_t DataSize0 = sizeof(DataStruct0);
  size_t DataSize1 = sizeof(DataStruct1);

  EVAL_PRINT_ULL(DataSize0);
  EVAL_PRINT_ULL(DataSize1);

  PRINT_S("Allocating with static allocator\n");
  DataStruct0* static_pointers0[64];
  DataStruct0* runtime_pointers0[64];
  DataStruct1* static_pointers1[64];
  DataStruct1* runtime_pointers1[64];
  for(size_t i = 0; i < 64; i++) {
    static_pointers0[i] = lofi::StaticAllocator::allocate<0, DataStruct0>();
    new((void*)static_pointers0[i]) DataStruct0{};
    static_pointers1[i] = lofi::StaticAllocator::allocate<0, DataStruct1>();
    new((void*)static_pointers1[i]) DataStruct1{};
    EVAL_PRINT_ULL(static_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }

  PRINT_S("Allocating with runtime allocator\n"); 
  for(size_t i = 0; i < 64; i++) {
    runtime_pointers0[i] = (DataStruct0*)lofi::RuntimeAllocator<0>::allocate(sizeof(DataStruct0), 8);
    new((void*)runtime_pointers0[i]) DataStruct0{};
    runtime_pointers1[i] = (DataStruct1*)lofi::RuntimeAllocator<0>::allocate(sizeof(DataStruct1), 8);
    new((void*)runtime_pointers1[i]) DataStruct1{};
    EVAL_PRINT_ULL(runtime_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }

  PRINT_S("Deallocating with static allocator\n"); 
  for(size_t i = 0; i < 64; i++) {
    lofi::StaticAllocator::deallocate<0>(static_pointers0[i]);
    lofi::StaticAllocator::deallocate<0>(static_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }
  PRINT_S("Deallocating with runtime allocator\n"); 
  for(size_t i = 0; i < 64; i++) {
    lofi::RuntimeAllocator<0>::free(runtime_pointers0[i]);
    lofi::RuntimeAllocator<0>::free(runtime_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }

////--------------------------------------------------------------------------------------------
//  PRINT_TITLE("SYNC");
////--------------------------------------------------------------------------------------------
//
//  lofi::lock_free_queue<size_t, 128> g_queue;
//  using node = lofi::lock_free_queue<size_t, 128>::node;
//  node nodes[128];
//  std::vector<std::thread> worker_pool;
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    PRINT("constructing push thread %llu\n", i);
//    worker_pool.emplace_back(std::thread{[&](){
//      for(size_t j = i * 16; j < ((i * 16) + 16); j++) {
//        nodes[j].data = j;
//        PRINT("thread %llu pushing %llu\n", i, j);
//        g_queue.push(&nodes[j]);  
//      }
//    }});
//  }
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    worker_pool[i].join();
//  }
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    PRINT("constructing pop thread %llu\n", i);
//    worker_pool.emplace_back(std::thread{[&](){
//        size_t index = MAX_u64;
//      while(g_queue.pop(&index)) {
//        EVAL_PRINT_ULL(index);
//      }
//    }});
//  }
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    worker_pool[i + NumThreads].join();
//  }

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("THREAD POOL");
//--------------------------------------------------------------------------------------------

  using thread_pool_t = lofi::ThreadPool<NumThreads, NumFibers>;

  thread_pool_t::job_node_t node{};
  lofi::atomic_counter<> terminate_gate{0};

  lofi::Job job_to_push;

#if defined(DO_BIG)
  job_to_push.set_entry_point(big_init_job);
#else
  job_to_push.set_entry_point(lil_init_job);
#endif

  job_to_push.set_job_success(standard_job_success);
  job_to_push.set_job_failure(print_job_failure);
  job_to_push.set_job_start(0);
  job_to_push.set_job_end(1);
  job_to_push.set_job_counter(&terminate_gate);
  node.data = job_to_push;

  GET_THREAD_POOL(thread_pool_t)->push_high_priority_job(0, &node);
  GET_THREAD_POOL(thread_pool_t)->run();
//  u32 spins = 0;
  while(terminate_gate.get_count() < 1) { 
//    spins++;
//    PRINT("sleeping 100ms, total = %u\n", spins * 100);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms); 
  }
  GET_THREAD_POOL(thread_pool_t)->terminate();

#if defined(DO_BIG)
  const size_t job_size = 8192;
#else
  const size_t job_size = 256;
#endif

  for(size_t i = 0; i < job_size; i++) {
    PRINT("values[%llu] = %llu\n", i, values[i]);
  }

#if defined(DO_BIG)
  for(size_t i = 0; i < 4; i++) {
    PRINT("big_job_results[%llu] = %llu\n", i, big_job_results[i]);
  }
#endif

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("Database");
//--------------------------------------------------------------------------------------------


  using db_t = lofi::Database<ExampleTableDescriptorSet>;
  using arg_t = ExampleTableTuple;
  using col0_pred_t = ExampleCol0Predicate;
  using col1_pred_t = ExampleCol1Predicate;
  using col2_pred_t = ExampleCol2Predicate;
  db_t db;

  col0_pred_t pred0;
  col1_pred_t pred1;
  col2_pred_t pred2;

  lofi::Query<db_t, 0, col0_pred_t> query0{&db, FWD(pred0)};
  lofi::Query<db_t, 0, col1_pred_t> query1{&db, FWD(pred1)};
  lofi::Query<db_t, 0, col2_pred_t> query2{&db, FWD(pred2)};

  for(size_t i = 0; i < 8; i++) {
    arg_t arg;
    arg.get<0>() = i * 10;
    arg.get<1>() = i * 32;
    arg.get<2>() = 4.2f * i;

    db.template get_table<0>().emplace(FWD(arg));
  }

  for(size_t i = 0; i < 8; i++) {
    if(query0.matches(i)) {
      PRINT("compiler constructed query 0 matches %llu, column 0 < 32\n", i);
    }
    if(query1.matches(i)) {
      PRINT("compiler constructed query 1 matches %llu, column 1 > 32\n", i);
    }
    if(query2.matches(i)) {
      PRINT("compiler constructed query 2 matches %llu, column 2 >= 1.f\n", i);
    }
    PRINT("columns at %llu = {%llu, %llu, %f}\n", i, db.template get_table<0>().template get_column<0>()[i], db.template get_table<0>().template get_column<1>()[i], db.template get_table<0>().template get_column<2>()[i]);
  }

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("ECS");
//--------------------------------------------------------------------------------------------

  using ecs_t = lofi::ecs::Manager<lofi::ecs::Config<0>>;

  ecs_t ecs;

  ecs.print_test_results();

  lofi::ecs::Entity entities[128];
  for(size_t i = 0; i < 128; i++) {
    PRINT("creating entity %llu ", i);
    if(i < 32) {
      entities[i] = ecs.create_entity<arch0>();
      PRINT("with arch 0 row index = %llu\n", ecs.get_entity_row_index(entities[i]));
    } else if (i < 64) {
      entities[i] = ecs.create_entity<arch1>();
      PRINT("with arch 1 row index = %llu\n", ecs.get_entity_row_index(entities[i]));
    } else {
      entities[i] = ecs.create_entity<arch2>();
      PRINT("with arch 2 row index = %llu\n", ecs.get_entity_row_index(entities[i]));
    }
  }
  for(size_t i = 0; i < 16; i++) {
    PRINT("entity %llu adding component 2 and tag0\n", i);
    lofi::tuple<comp2, tag0> add_arg;
    add_arg.template get<0>().value = i;
    ecs.add_components(entities[i], FWD(add_arg));
  }
  for(size_t i = 32; i < 48; i++) {
    PRINT("entity %llu adding component 0\n", i);
    lofi::tuple<comp0> add_arg;
    add_arg.template get<0>().value = i;
    ecs.add_components(entities[i], FWD(add_arg));
  }
  for(size_t i = 0; i < 8; i++) {
    PRINT("entity %llu removing component 1\n", i);
    ecs.remove_components<comp1, tag0>(entities[i]);
  }
  for(size_t i = 32; i < 40; i++) {
    PRINT("entity %llu removing component 1\n", i);
    ecs.remove_component<comp1>(entities[i]);
    PRINT("entity %llu adding tag 0\n", i);
    ecs.add_components<tag0>(entities[i]);
  }
  for(size_t i = 0; i < 128; i++) {
    PRINT("printing entity at index %llu ", i);
    auto arch_id = ecs.get_entity_arch_id(entities[i]);
    auto row_index = ecs.get_entity_row_index(entities[i]);
    PRINT("entity id = %u has archetype %llu at row index = %llu\n", entities[i].index, arch_id, row_index);
  }
  auto num_arch_2 = ecs.get_num_entities<arch2>();
  auto num_arch_4 = ecs.get_num_entities<arch4>();
  auto num_arch_5 = ecs.get_num_entities<arch5>();

  PRINT("number of archetype 2 entities = %u\n", num_arch_2);
  PRINT("number of archetype 4 entities = %u\n", num_arch_4);
  PRINT("number of archetype 5 entities = %u\n", num_arch_5);

  for(size_t i = 0; i < num_arch_2; i++) {
    auto component_ptr = ecs.get_component<arch2, comp2>();
    PRINT("arch 2 row index = %llu, component value = %u\n", i, component_ptr[i].value);
  }

  for(size_t i = 0; i < num_arch_4; i++) {
    auto component_ptr = ecs.get_component<arch4, comp2>();
    PRINT("arch 4 row index = %llu, component value = %u\n", i, component_ptr[i].value);
  }

  for(size_t i = 0; i < num_arch_5; i++) {
    auto component_ptr = ecs.get_component<arch5, comp2>();
    PRINT("arch 5 row index = %llu, component value = %u\n", i, component_ptr[i].value);
  }
  for(size_t i = 0; i < 16; i++) {
    PRINT("entity index = %llu, component value = %u\n", i, ecs.get_component<comp2>(entities[i]).value);
  }
  auto archetypes01 = ecs.find_matching_archetypes<comp0, comp1>();
  auto& archetypes01_array = archetypes01.template get<0>();
  auto& archetypes01_size_array = archetypes01.template get<1>();

  const auto num_archetypes01 = archetypes01_array.get_size();
  for(size_t i = 0; i < num_archetypes01; i++) {
    const auto table_size = archetypes01_size_array[i];
    for(size_t j = 0; j < table_size; j++) {
      PRINT("archetypes 01 table in tuple index = %llu, row_index = %llu, component 0 value = %u, component 1 value = %u\n", i, j, archetypes01.template get<0>()[i].template get<0>()[j].value, archetypes01.template get<0>()[i].template get<1>()[j].value);
    }
  }

  auto archetypes12 = ecs.find_matching_archetypes<comp1, comp2>();
  auto& archetypes12_array = archetypes12.template get<0>();
  auto& archetypes12_size_array = archetypes12.template get<1>();

  const auto num_archetypes12 = archetypes12_array.get_size();
  for(size_t i = 0; i < num_archetypes01; i++) {
    const auto table_size = archetypes12_size_array[i];
    for(size_t j = 0; j < table_size; j++) {
      PRINT("archetypes 12 table in tuple index = %llu, row_index = %llu, component 1 value = %u, component 2 value = %u\n", i, j, archetypes12.template get<0>()[i].template get<0>()[j].value, archetypes12.template get<0>()[i].template get<1>()[j].value);
    }
  }

  auto archetypes02 = ecs.find_matching_archetypes<comp0, comp2>();
  auto& archetypes02_array = archetypes02.template get<0>();
  auto& archetypes02_size_array = archetypes02.template get<1>();

  const auto num_archetypes02 = archetypes02_array.get_size();
  for(size_t i = 0; i < num_archetypes02; i++) {
    const auto table_size = archetypes02_size_array[i];
    for(size_t j = 0; j < table_size; j++) {
      PRINT("archetypes 02 table in tuple index = %llu, row_index = %llu, component 0 value = %u, component 2 value = %u\n", i, j, archetypes02.template get<0>()[i].template get<0>()[j].value, archetypes02.template get<0>()[i].template get<1>()[j].value);
    }
  }


  PRINT_TITLE("EXITING TESTS");
  return 0;
}
