// =====================================================================================
//
//       Filename:  vk_batch.hpp
//
//    Description:  abstraction for a batch of commands to be batched
//
//        Version:  1.0
//        Created:  2024-10-06 12:33:41 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#include "rx_frame_manager.hpp"

#include "rx_vocab.h"
#include "vk_allocator.hpp"
#include "vk_descriptors.hpp"
#include "vk_pipeline.hpp"
#include "vk_command.hpp"

namespace roxi {
 
  namespace vk {

    template<DispatchType Type>
    class BindGroup {
    private:
      using DescriptorBinding = u32;
      Pipeline                                                        _pipeline;

      // per layout binding ?? (rather than descriptor set)
      Array<DescriptorBinding>               _descriptor_buffer_bindings;
      Array<u32>                              _descriptor_buffer_offsets;
      VkExtent3D dispatch_dimensions{};

    public:
      static constexpr CommandType command_type = CommandT;
      static constexpr CommandBufferLevelType command_level = LevelT;
      b8 init(Context* context, const u32 descriptor_set_count, DescriptorSetLayout* layouts) {
        _descriptor_buffer_bindings.move_ptr(ALLOCATE(descriptor_set_count * 16 * sizeof(DescriptorBinding)));
        _descriptor_buffer_offsets.move_ptr( ALLOCATE(descriptor_set_count * 16 * sizeof(u32)));
        for(u64 i = 0; i < descriptor_set_count; i++) {
          const u32 binding_count = layouts[i].get_num_bindings();
          for(u64 j = 0; j < binding_count; j++) {
            *(_descriptor_buffer_offsets.push(1)) = layouts[i].get_layout_binding_offset(context, j);
            *(_descriptor_buffer_bindings.push(1)) = layouts[i].get_layout_binding_id(j);
          }
        }
      }

      b8 bind(Context* context, CommandBuffer<, LevelT>& command) {
        command.bind_pipeline(context, _pipeline);
        command.bind_descriptor_offsets(context, _pipeline.get_pipeline_layout(), 0, _descriptor_buffer_bindings.get_buffer(), _descriptor_buffer_bindings.get_buffer(), _descriptor_buffer_offsets.get_buffer());
      }

      b8 terminate() {
        FREE(_descriptor_buffer_bindings.get_buffer());
        FREE(_descriptor_buffer_offsets.get_buffer());
      }

    };

    template
      < QueueIndices QueueIndex
      , u64 NumFrames
      , u64 NumThreads
      , u64 NumSecondaryCommandBuffers
      , typename DescriptorBufferTypeListT
      , typename DescriptorBufferLevelListT
      , typename CommandTypeListT
      >
    class Batch;

    template
      < QueueIndices QueueIndex
      , u64 NumFrames
      , u64 NumThreads
      , u64 NumSecondaryCommandBuffers
      , DescriptorBufferType... DescriptorTypes
      , DescriptorBufferLevel... DescriptorLevels
      , CommandType... CommandTypes
      >
    class Batch
      < QueueIndex
      , NumFrames
      , NumThreads
      , NumSecondaryCommandBuffers
      , List<lofi::IdxT<DescriptorTypes>...>
      , List<lofi::IdxT<DescriptorLevels>...>
      , List<lofi::IdxT<CommandTypes>...>
      > 
    {
    private:
      static constexpr u64 command_buffer_count = MIN(NumSecondaryCommandBuffers + 1, NumThreads);

      using bind_groups_t
        = Tuple
          < BindGroup
            < CommandTypes
            , command_buffer_count == 1 ?
              CommandBufferLevelType::Primary 
              : CommandBufferLevelType::Secondary
            >...
          >;

      using descriptor_buffers_t 
        = Tuple
          < DescriptorBufferArena
            < DescriptorTypes
            , DescriptorLevels
            >...
          >;

      using command_pools_t 
        = Tuple
          < CommandPool
            < CommandTypes
            >[command_buffer_count]...
          >;

      using primary_command_buffers_t 
        = Tuple
          < CommandBuffer
            < CommandTypes
            , CommandBufferLevelType::Primary
            >...
          >;

      using secondary_command_buffers_t 
        = Tuple
          < CommandBuffer
            < CommandTypes
            , CommandBufferLevelType::Secondary
            >[command_buffer_count]...
          >;

      Context* _context = nullptr;

      mem::MemoryPool _memory_pool;

      descriptor_buffers_t               _descriptor_buffers[NumFrames];
      command_pools_t                         _command_pools[NumFrames];
      primary_command_buffers_t     _primary_command_buffers[NumFrames];
      secondary_command_buffers_t _secondary_command_buffers[NumFrames];

    public:
      b8 init(Context* context) {
        _context = context;
        mem::MemoryPoolBuilder memory_pool_builder{};
        memory_pool_builder.init(context);

        b8 result = true;
        for(u64 i = 0; i < NumFrames; i++) {
          lofi::meta::static_for
            ( _descriptor_buffers[i]
            , [&]<u64 I>
              ( lofi::IdxT<I> index
              , typename descriptor_buffers_t::template type_at_index<I>& type
              ) 
            {
              if(!type.init(context, &memory_pool_builder, s_max_descriptor_sets)) {
                LOG("could not create descriptor buffer in vk::Batch::init(Context*)", Warn);
                result = false;
              }
            });

          lofi::meta::static_for
            ( _command_pools[i]
            , [&]<u64 I>
              ( lofi::IdxT<I> index
              , typename command_pools_t::template type_at_index<I>& type
              ) 
            {
              if(!create_command_pool(context, &type)) {
                LOG("could not create command pool in vk::Batch::init(Context*)", Warn);
                result = false;
              }
            });

          VkCommandBuffer temp_buffer;

          lofi::meta::static_for
            ( _primary_command_buffers[i]
            , [&]<u64 I>
              ( lofi::IdxT<I> index
              , typename primary_command_buffers_t::template type_at_index<I>& type
              ) 
            {
              if(!_command_pools[i].template get<I>().template allocate<CommandBufferLevelType::Primary>(context, 1, &temp_buffer)) {
                LOG("could not allocate primary command buffers in vk::Batch::init(Context*)", Warn);
                result = false;
              }
              type.init(context, temp_buffer);
            });

          lofi::meta::static_for
            ( _secondary_command_buffers[i]
            , [&]<u64 I>
              ( lofi::IdxT<I> index
              , typename secondary_command_buffers_t::template type_at_index<I>& type
              ) 
            {
              if(!_command_pools[i].template get<I>().template allocate<CommandBufferLevelType::Secondary>(context, 1, &temp_buffer)) {
                LOG("could not allocate secondary command buffers in vk::Batch::init(Context*)", Warn);
                result = false;
              }
              type.init(context, temp_buffer);
            });
        }
        return result;
      }

      b8 record(u32 thread_id, frame::ID frame_id) {
        
      }

      b8 reset_commands(frame::ID frame_id) {
        b8 result = true;

        lofi::meta::static_for
          ( _command_pools[frame_id]
          , [&]<u64 I>
            ( lofi::IdxT<I> index
            , typename command_pools_t::template type_at_index<I>& type
            ) 
          {
            if(!_command_pools[frame_id].template get<I>().reset()) {
              LOG("failed to reset command pools in vk::Batch::reset(frame::ID)", Warn);
              result = false;
            }
          });

        return result;
      }

      b8 reset_descriptors(frame::ID frame_id) {
        b8 result = true;

        lofi::meta::static_for
          ( _descriptor_buffers[frame_id]
          , [&]<u64 I>
            (lofi::IdxT<I> index
            , typename descriptor_buffers_t::template type_at_index<I>& type
            ) 
          {
            if(!_descriptor_buffers[frame_id].template get<I>().reset()) {
              LOG("failed to reset descriptor buffers in vk::Batch::reset(frame::ID)", Warn);
              result = false;
            }
          });

        return result;
      }

      b8 reset(frame::ID frame_id) {
        return reset_descriptors(frame_id) && reset_commands(frame_id);
      }

      b8 terminate() {
        for(u64 i = 0; i < NumFrames; i++) {

          b8 result = true;
          lofi::meta::static_for
            ( _descriptor_buffers[i]
            , [&]<u64 I>
              ( lofi::IdxT<I> index
              , typename descriptor_buffers_t::template type_at_index<I>& type 
              ) 
            {
              if(!type.terminate()) {
                LOG("failed to terminate descriptor buffer in vk::Batch::terminate()", Warn);
                result = false;
              }
            });

          lofi::meta::static_for
            ( _command_pools[i]
            , [&]<u64 I>
              ( lofi::IdxT<I> index
              , typename command_pools_t::template type_at_index<I>& type
              )
            {
              if(!type.terminate(_context)) {
                LOG("failed to erminate command pool in vk::Batch::terminate()", Warn);
                result = false;
              }
            });

          return result;
        }
      }

    };
    

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
