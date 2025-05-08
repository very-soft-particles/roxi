// =====================================================================================
//
//       Filename:  vk_command.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-10 3:21:23 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_command.hpp"
#include "rx_allocator.hpp"

namespace roxi {
  namespace vk {

    b8 CommandArena::init(Context* context, const CommandType command_type, const CommandBufferLevelType level_type, const u32 job_count, const u32 queue_handle) {
      _context = context;
      VkCommandPoolCreateInfo create_info{};

      if(level_type == CommandBufferLevelType::Secondary && (job_count != 0)) {

        _command_buffers.move_ptr(ALLOCATE(sizeof(VkCommandBuffer) * job_count));
        _command_buffers.push(job_count);

        RX_TRACE("filling allocate info for secondary command arena");
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;
        alloc_info.commandPool = _pool;
        alloc_info.commandBufferCount = job_count;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        RX_TRACE("allocating secondary command buffer");
        VK_CHECK(_context->get_device()
          .get_device_function_table()
          .vkAllocateCommandBuffers(_context->get_device().get_device()
            , &alloc_info
            , &_command_buffers[0])
          , "failed to allocate secondary command buffer"
          );
      } else if(level_type == CommandBufferLevelType::Primary) {
        _command_buffers.move_ptr(ALLOCATE(sizeof(VkCommandBuffer) * (1 + job_count)));
        _command_buffers.push(1 + job_count);

        RX_TRACE("filling create info for primary command arena");
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        create_info.queueFamilyIndex = static_cast<u32>(context->get_device().get_queue_family_index(queue_handle).queue_family_index);


        RX_TRACE("vkCreateCommandPool");
        VK_CHECK(context->get_device()
          .get_device_function_table()
          .vkCreateCommandPool(context->get_device().get_device()
            , &create_info
            , CALLBACKS()
            , &_pool)
          , "failed to create secondary command pool");

        RX_TRACE("allocating primary command buffer");
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;
        alloc_info.commandPool = _pool;
        alloc_info.commandBufferCount = 1;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VK_CHECK(_context->get_device()
          .get_device_function_table()
          .vkAllocateCommandBuffers(_context->get_device().get_device()
            , &alloc_info
            , &_command_buffers[0])
          , "failed to allocate primary command buffer"
          );

        if(job_count) {
          alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
          alloc_info.pNext = nullptr;
          alloc_info.commandPool = _pool;
          alloc_info.commandBufferCount = job_count;
          alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

          VK_CHECK(_context->get_device()
            .get_device_function_table()
            .vkAllocateCommandBuffers(_context->get_device().get_device()
              , &alloc_info
              , &_command_buffers[1])
            , "failed to allocate secondary command buffer"
            );
        }
      } else {
        LOG("attempted to create secondary command buffers without a job count in roxi::vk::CommandArena", Error);
        return false;
      }

      return true;
    }

    const VkCommandPool CommandArena::get_pool() const {
      return _pool;
    }

    CommandBuffer CommandArena::obtain_command_buffer() {
      CommandBuffer result;
      result.init(_context, _command_buffers[current_top++]);
      return result;
    }

    b8 CommandPool::init(Context* context, const u32 queue_handle, const CommandType command_type, const u32 parallelism, const u32 job_count) {
      _context = context;

      RX_TRACE("allocating command arena buffer");
      u8* buffer = (u8*)ALLOCATE
        ( sizeof(CommandArena) * parallelism
        + sizeof(u32) * parallelism);

      _command_arenas.move_ptr(buffer);
      _command_arenas.push(parallelism);
      buffer += sizeof(CommandArena) * parallelism;

      _free_command_arena_handles.move_ptr(buffer);
      _free_command_arena_handles.push(parallelism);

      RX_TRACEF("initializing primary command arena with command type = %u, and job count = %u", command_type, job_count);
      RX_CHECKF(_command_arenas[0].init(context, command_type, vk::CommandBufferLevelType::Primary, job_count, queue_handle), "failed to initialize primary command arena with roxi::vk::CommandType::%s", get_command_type_string(command_type));
      _free_command_arena_handles[0] = 0;
      if(parallelism > 1) {
        RX_TRACEF("initializing %u secondary command arenas", parallelism);
        for(u32 i = 1; i < parallelism; i++) {
          _free_command_arena_handles[i] = i;
          RX_TRACEF("initializing secondary command arena %u", i);
          RX_CHECKF(_command_arenas[i].init(context, command_type, vk::CommandBufferLevelType::Secondary, job_count, queue_handle)
            , "roxi::vk::CommandPool with type %s failed to initialize roxi::vk::SecondaryCommandArena"
            , get_command_type_string(command_type));
        }
      }
      return true;
    }

    b8 CommandPool::reset() {
      const auto size = _command_arenas.get_size();
      top_arena = 0;
      for(u32 i = 0; i < size; i++) {
        _free_command_arena_handles[i] = i;
        RX_CHECK(_command_arenas[i].reset()
          , "failed to reset secondary command arena"
        );
      }
      return true;
    }

    b8 CommandPool::terminate() {
      const auto size = _command_arenas.get_size();
      for(u32 i = 0; i < size; i++) {
        RX_CHECK
        ( _command_arenas[i].terminate()
        , "CommandArena failed to terminate properly");
      }
      FREE(_command_arenas.get_buffer());
      return true;
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
