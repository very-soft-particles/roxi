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

    b8 CommandArena::init(Context* context, const CommandType command_type, const CommandBufferLevelType level_type, const u32 queue_handle) {
      _context = context;
      VkCommandPoolCreateInfo create_info{};
      _current_top = 0;

      if(level_type == CommandBufferLevelType::Secondary) {

        RX_TRACE("filling allocate info for secondary command arena");
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

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;
        alloc_info.commandPool = _pool;
        alloc_info.commandBufferCount = 1;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        RX_TRACE("allocating secondary command buffer");
        VK_CHECK(_context->get_device()
          .get_device_function_table()
          .vkAllocateCommandBuffers(_context->get_device().get_device()
            , &alloc_info
            , &_command_buffer)
          , "failed to allocate secondary command buffer"
          );
      } else if(level_type == CommandBufferLevelType::Primary) {
        // there can be only one to preserve the linearity of a command buffer
        // if necessary will spawn coroutines to fill secondary command buffers and 
        // record them into the primary one afterwards

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
            , &_command_buffer)
          , "failed to allocate primary command buffer"
          );
      }
      return true;
    }

    const VkCommandPool CommandArena::get_pool() const {
      return _pool;
    }

    CommandBuffer CommandArena::obtain_command_buffer() {
      CommandBuffer result;
      result.init(_context, _command_buffer);
      return result;
    }

    b8 CommandPool::init(Context* context, const u32 queue_handle, const CommandType command_type, const u32 parallelism) {
      _context = context;
      _secondary_command_arenas.clear();

      RX_ASSERT(parallelism > 0, "command pool must have greater than 0 parallelism");
      RX_TRACE("allocating command arena buffer");

      RX_TRACEF("initializing type = %u", command_type);
      RX_CHECKF(_primary_command_arena.init(context, command_type, vk::CommandBufferLevelType::Primary, queue_handle), "failed to initialize primary command arena with roxi::vk::CommandType::%s", get_command_type_string(command_type));
      if(parallelism > 1) {
        u8* buffer = (u8*)ALLOCATE
          ( sizeof(CommandArena) * (parallelism - 1));
  
        _secondary_command_arenas.move_ptr(buffer);
        _secondary_command_arenas.push(parallelism - 1);
 
        RX_TRACEF("initializing %u secondary command arenas", parallelism);
        for(u32 i = 1; i < parallelism; i++) {
          RX_TRACEF("initializing secondary command arena %u", i);
          RX_CHECKF(_secondary_command_arenas[i].init(context, command_type, vk::CommandBufferLevelType::Secondary, queue_handle)
            , "roxi::vk::CommandPool with type %s failed to initialize roxi::vk::SecondaryCommandArena"
            , get_command_type_string(command_type));
        }
      }
      return true;
    }

    b8 CommandPool::reset() {
      _primary_command_arena.reset();
      const auto size = _secondary_command_arenas.get_size();
      for(u32 i = 0; i < size; i++) {
        RX_TRACEF("resetting command arena %u", i);
        RX_CHECK(_secondary_command_arenas[i].reset()
          , "failed to reset secondary command arena"
        );
      }
      return true;
    }

    b8 CommandPool::terminate() {
      const auto size = _secondary_command_arenas.get_size();
      for(u32 i = 0; i < size; i++) {
        RX_CHECK
        ( _secondary_command_arenas[i].terminate()
        , "CommandArena failed to terminate properly");
      }
      if(size) {
        FREE(_secondary_command_arenas.get_buffer());
      }
      return true;
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
