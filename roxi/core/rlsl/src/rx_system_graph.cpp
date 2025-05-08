// =====================================================================================
//
//       Filename:  rx_system_graph.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-31 2:06:52 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/rx_system_graph.hpp"


namespace roxi {
 
  namespace rlsl {

    TaskNode create_task_node(const char* name, Job entry, const u32 input_count, const u32 output_count, const u32* input_ids, const u32* output_ids, const TaskPriority priority) {

      TaskNode result{};
      result.priority = priority;

      u8* buffer = (u8*)ALLOCATE(sizeof(u32) * (input_count + output_count + MaxTaskResourcesPerSystem));
      result.name = lofi::nt_str_cstring(name);
      result.entry = entry;

      result.input_ids.move_ptr(buffer);
      u32* const input_begin = result.input_ids.push(input_count);
      buffer += sizeof(u32) * input_count;

      result.output_ids.move_ptr(buffer);
      u32* const output_begin = result.output_ids.push(output_count);
      buffer += sizeof(u32) * output_count;

      result.edge_ids.move_ptr(buffer);

      for(u32 i = 0; i < input_count; i++) {
        input_begin[i] = input_ids[i];
      }
      for(u32 i = 0; i < output_count; i++) {
        output_begin[i] = output_ids[i];
      }

      return result;
    }

    b8 System::update() {
      const u32 task_count = _tasks.get_size();
      StackArray<u32> ext_component_ids;
      Counter job_counter;
      u32 job_count = 0;
      u32 task_offset = 0;
      u32 current_level = 0;
      for(u32 t = 0; t < task_count; t++) {
        TaskNode& task = _tasks[t];
        if(task.level > current_level) {
          FIBER_WAIT(&job_counter, job_count);
          job_counter.reset();
          job_count = 0;
          current_level++;
          _arena.clear();
        }
        const u32 input_count = task.input_ids.get_size();
        for(u32 i = 0; i < input_count; i++) {
          TaskResource& resource = _resources[task.input_ids[i]];
          if(resource.info.type == TaskResourceType::External) {
            *(ext_component_ids.push(1)) = resource.info.external.component_id;
          }
        }

        const u32 output_count = task.output_ids.get_size();
        for(u32 i = 0; i < output_count; i++) {
          TaskResource& resource = _resources[task.output_ids[i]];
          if(resource.info.type == TaskResourceType::External) {
            *(ext_component_ids.push(1)) = resource.info.external.component_id;
          }
        }

        u32 archetype_count = 0;
        _ecs->select(ext_component_ids.get_buffer(), ext_component_ids.get_size(), &archetype_count, nullptr);

        job_count += archetype_count;
        Array<ECS::ArchetypeID> archetypes{_arena.push(archetype_count * sizeof(ECS::ArchetypeID))};

        _ecs->select(ext_component_ids.get_buffer(), ext_component_ids.get_size(), &archetype_count, archetypes.push(archetype_count));

        Array<Job> jobs{_arena.push(sizeof(Job) * archetype_count)};
        Job* const jobs_begin = jobs.push(archetype_count);

        for(u32 i = 0; i < archetype_count; i++) {
          const u32 entity_count = _ecs->get_table_entity_count(archetypes[i]);
          jobs_begin[i] = task.entry;
          jobs_begin[i].set_job_counter(&job_counter);
          jobs_begin[i].set_job_start(archetypes[i]);
          jobs_begin[i].set_job_end(entity_count);
          jobs_begin[i].set_obj(this);
        }

        switch(task.priority) {
          case TaskPriority::High:
            FIBER_KICK_HIGH_PRIORITY_JOBS(jobs.get_buffer(), archetype_count);
            break;
          case TaskPriority::Medium:
            FIBER_KICK_MID_PRIORITY_JOBS(jobs.get_buffer(), archetype_count);
            break;
          case TaskPriority::Low:
            FIBER_KICK_LOW_PRIORITY_JOBS(jobs.get_buffer(), archetype_count);
            break;
        }
      }
      return true;
    }

    ECS& System::get_ecs() {
      return *_ecs;
    }

    b8 System::terminate() {
      const u32 task_count = _tasks.get_size();
      for(u32 i = 0; i < task_count; i++) {
        void* buffer = _tasks[i].input_ids.get_buffer();
        if(!buffer) {
          return false;
        }
        FREE(buffer);
      }
      FREE(_tasks.get_buffer());
      return true;
    }

  }		// -----  end of namespace rlsl  ----- 

}		// -----  end of namespace roxi  ----- 
