// =====================================================================================
//
//       Filename:  vk_command.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-25 12:21:27 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_context.h"
#include "vk_descriptors.hpp"
#include "vk_pipeline.hpp"
#include "vk_sync.hpp"

#define ROXI_COMMAND_TYPES(X) X(Graphics) X(Compute) X(Transfer) X(RayTracing) X(Max)

namespace roxi {

  namespace vk {

    // just double buffer the pools to limit memory consumption
    static constexpr u64 s_max_primary_command_buffers_per_arena = 1;
    static constexpr u64 s_max_secondary_command_buffers_per_arena = 2;

    static constexpr u64 s_max_command_buffers_per_arena 
      = s_max_primary_command_buffers_per_arena 
      + s_max_secondary_command_buffers_per_arena;

    static constexpr u64 s_max_command_arenas = 16;

    static constexpr u64 s_max_command_buffers_per_pool = s_max_command_buffers_per_arena * s_max_command_arenas;

    enum class CommandBufferLevelType {
      Primary,
      Secondary,
      Max
    };

    enum class CommandType {
#define ENUM(X) X,
      ROXI_COMMAND_TYPES(ENUM)
#undef ENUM
    };

    static const char* get_command_type_string(const CommandType type) {
#define STRING(X) STRINGIFY(X),
      static const char* s_string_array[] = {
        ROXI_COMMAND_TYPES(STRING)
      };
#undef STRING
      return s_string_array[(u8)type];
    }

    template<CommandType Type>
    static constexpr QueueType get_queue_type() {
      if constexpr (Type == CommandType::Graphics) {
        return vk::QueueType::Render;
      } else if constexpr (Type == CommandType::Transfer) {
        return vk::QueueType::Transfer;
      } else if constexpr (Type == CommandType::Compute) {
        return vk::QueueType::Compute;
      }
      return vk::QueueType::Max;
    }

    template<QueueType Type>
    static constexpr CommandType get_command_type() {
      if constexpr (Type == QueueType::Render) {
        return vk::CommandType::Graphics;
      } else if constexpr (Type == QueueType::Transfer) {
        return vk::CommandType::Transfer;
      } else if constexpr (Type == QueueType::Compute) {
        return vk::CommandType::Compute;
      }
      return vk::CommandType::Max;
    }


    const VkCommandBufferLevel get_buffer_level(const CommandBufferLevelType type);
    
    template<CommandType CommandT>
    static constexpr PipelineType Command2PipelineT 
      = CommandT == CommandType::Compute ? 
        PipelineType::Compute 
        : CommandT == CommandType::Graphics ? 
          PipelineType::Graphics 
          : CommandT == CommandType::RayTracing ? 
            PipelineType::RayTracing 
            : PipelineType::Max;

    class CommandBuffer {
    private:
      Context* _context = nullptr;
      Pipeline* _current_pipeline = nullptr;
      DescriptorPool* _current_pool = nullptr;
      VkCommandBuffer _buffer = VK_NULL_HANDLE;
    public:
      CommandBuffer() {}

      b8 init(Context* context, VkCommandBuffer buffer) { 
        _context = context;
        _buffer = buffer;
        return true;
      }

      CommandBuffer& begin_render_pass(const RenderPass& render_pass, const u32 width, const u32 height, const Framebuffer& framebuffer, VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) {
        VkClearValue clear_value {1.f, 0.f, 0.f, 1.f};
        VkRenderPassBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.pNext = nullptr;
        begin_info.renderPass = render_pass.get_render_pass();
        begin_info.renderArea = VkRect2D{VkOffset2D{0, 0}, VkExtent2D{width, height}};
        begin_info.clearValueCount = 1;
        begin_info.pClearValues = &clear_value;
        begin_info.framebuffer = framebuffer.get_framebuffer();
        _context->get_device().get_device_function_table()
          .vkCmdBeginRenderPass(_buffer, &begin_info, subpass_contents);
        return *this;
      }

      CommandBuffer& next_subpass(VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) {
        _context->get_device().get_device_function_table()
          .vkCmdNextSubpass(_buffer, subpass_contents);
        return *this;
      }

      CommandBuffer& end_render_pass() {
        _context->get_device().get_device_function_table()
          .vkCmdEndRenderPass(_buffer);
        return *this;
      }

      CommandBuffer& begin(VkCommandBufferInheritanceInfo* inheritance_info = nullptr) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = nullptr;
        if (inheritance_info == nullptr) {
          begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        } else {
          begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        }
        begin_info.pInheritanceInfo = inheritance_info;
        VK_ASSERT(_context->get_device().get_device_function_table()
          .vkBeginCommandBuffer(_buffer, &begin_info), "failed to begin command buffer");
        return *this;
      }

      CommandBuffer& bind_pipeline(const Pipeline& pipeline) {
        if(_current_pipeline == &pipeline) {
          return *this;
        }
        _current_pipeline = &(Pipeline&)pipeline;
        _context->get_device().get_device_function_table()
          .vkCmdBindPipeline(_buffer, pipeline.get_pipeline_bind_point(), pipeline.get_pipeline());
        return *this;
      }

      CommandBuffer& bind_descriptor_pool(const DescriptorPool& pool) {
        if(_current_pool == &pool) {
          return *this;
        }
        _current_pool = &(DescriptorPool&)pool;
        const u32 buffer_count = 4;
        VkDescriptorBufferBindingInfoEXT binding_infos[4];

        u32 indices[] = {0, 1, 2, 3};
        VkDeviceSize offsets[] = {0, 0, 0, 0};

        binding_infos[0].address = pool.obtain_arena<DescriptorBufferType::Uniform>().get_descriptor_buffer().get_device_address();
        binding_infos[0].usage   = pool.obtain_arena<DescriptorBufferType::Uniform>().get_usage_flags();

        binding_infos[1].address = pool.obtain_arena<DescriptorBufferType::Storage>().get_descriptor_buffer().get_device_address();
        binding_infos[1].usage   = pool.obtain_arena<DescriptorBufferType::Storage>().get_usage_flags();

        binding_infos[2].address = pool.obtain_arena<DescriptorBufferType::CombinedImageSampler>().get_descriptor_buffer().get_device_address();
        binding_infos[2].usage   = pool.obtain_arena<DescriptorBufferType::CombinedImageSampler>().get_usage_flags();

        binding_infos[3].address = pool.obtain_arena<DescriptorBufferType::StorageImage>().get_descriptor_buffer().get_device_address();
        binding_infos[3].usage   = pool.obtain_arena<DescriptorBufferType::StorageImage>().get_usage_flags();

        for(u32 i = 0; i < buffer_count; i++) {
          binding_infos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
          binding_infos[i].pNext = nullptr;
          // offset is zero for each buffer
        }
        _context->get_device().get_device_function_table()
          .vkCmdBindDescriptorBuffersEXT(_buffer, buffer_count, binding_infos);

        _context->get_device().get_device_function_table()
          .vkCmdSetDescriptorBufferOffsetsEXT(_buffer, _current_pipeline->get_pipeline_bind_point(), _current_pipeline->get_pipeline_layout(), 0, buffer_count, indices, offsets);
        return *this;
      }

      CommandBuffer& bind_meshes(VkBuffer* vertex_buffers, VkDeviceSize* offsets, u32 count) {
        _context->get_device().get_device_function_table()
          .vkCmdBindVertexBuffers(_buffer, 0, count, vertex_buffers, offsets);
        return *this;
      }

      CommandBuffer& bind_indices(VkBuffer index_buffer, VkDeviceSize size) {
        _context->get_device().get_device_function_table()
          .vkCmdBindIndexBuffer(_buffer, index_buffer, size, VK_INDEX_TYPE_UINT32);
        return *this;
      }

      CommandBuffer& pipeline_barriers(const VkDependencyFlags dependency_flags = 0, const u32 buffer_count = 0, const u32 image_count = 0, const u32 barrier_count = 0, VkBufferMemoryBarrier2* buffer_barriers = nullptr, VkImageMemoryBarrier2* image_barriers = nullptr, VkMemoryBarrier2* memory_barriers = nullptr) {
        VkDependencyInfo dependency_info{};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.pNext = nullptr;
        dependency_info.dependencyFlags = dependency_flags;
        dependency_info.bufferMemoryBarrierCount = buffer_count;
        dependency_info.imageMemoryBarrierCount = image_count;
        dependency_info.memoryBarrierCount = barrier_count;
        dependency_info.pBufferMemoryBarriers = buffer_barriers;
        dependency_info.pImageMemoryBarriers = image_barriers;
        dependency_info.pMemoryBarriers = memory_barriers;
        _context->get_device().get_device_function_table()
          .vkCmdPipelineBarrier2(_buffer, &dependency_info);
        return *this;
      }

      CommandBuffer& record_copy_buffer(const Buffer& src, const Buffer& dst, u32 region_count, VkBufferCopy* region_infos) {
        _context->get_device().get_device_function_table()
          .vkCmdCopyBuffer(_buffer, src.get_buffer(), dst.get_buffer(), region_count, region_infos);
        return *this;
      }

      CommandBuffer& record_copy_image(const Image& src, const Image&& dst, u32 region_count, VkImageCopy* region_infos) {
        _context->get_device().get_device_function_table()
          .vkCmdCopyImage(_buffer, src.get_image(), src.get_image_layout(), dst.get_image(), dst.get_image_layout(), region_count, region_infos);
        return *this;
      }

      CommandBuffer& record_copy_image_to_buffer(const Image& src, const Buffer& dst, u32 region_count, VkBufferImageCopy* region_infos) {
        _context->get_device().get_device_function_table()
          .vkCmdCopyImageToBuffer(_buffer, src.get_image(), src.get_image_layout(), dst.get_buffer(), region_count, region_infos);
        return *this;
      }


      CommandBuffer& record_copy_buffer_to_image(const Buffer& src, const Image& dst, u32 region_count, VkBufferImageCopy* region_infos) {
        _context->get_device().get_device_function_table()
          .vkCmdCopyBufferToImage(_buffer, src.get_buffer(), dst.get_image(), dst.get_image_layout(), region_count, region_infos);
        return *this;
      }

      CommandBuffer& record_draw(u32 vertex_count, u32 instance_count, u32 first_vertex = 0, u32 first_instance = 0) {
        _context->get_device().get_device_function_table()
          .vkCmdDraw(_buffer, vertex_count, instance_count, first_vertex, first_instance);
        return *this;
      }

      CommandBuffer& record_indexed_draw(u32 index_count, u32 instance_count, u32 vertex_offset, u32 first_index = 0, u32 first_instance = 0) {
        _context->get_device().get_device_function_table()
          .vkCmdDrawIndexed(_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
        return *this;
      }

      CommandBuffer& record_indirect_draw(Buffer* const indirect_buffer, VkDeviceSize offset, u32 count, u32 stride) {
        _context->get_device().get_device_function_table()
          .vkCmdDrawIndirect(_buffer, indirect_buffer->get_buffer(), offset, count, stride);
        return *this;
      }

      CommandBuffer& record_indexed_indirect_draw(Buffer* const indirect_buffer, VkDeviceSize offset, u32 count, u32 stride) {
        _context->get_device().get_device_function_table()
          .vkCmdDrawIndexedIndirect(_buffer, indirect_buffer->get_buffer(), offset, count, stride);
        return *this;
      }

      CommandBuffer& record_dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z) {
        _context->get_device().get_device_function_table()
          .vkCmdDispatch(_buffer, group_count_x, group_count_y, group_count_z);
        return *this;
      }

      CommandBuffer& record_dispatch_indirect(Buffer* const indirect_buffer, const VkDeviceSize offset) {
        _context->get_device().get_device_function_table()
          .vkCmdDispatchIndirect(_buffer, indirect_buffer->get_buffer(), offset);
        return *this;
      }

      CommandBuffer& record_execute_commands(u32 num_commands, VkCommandBuffer* _buffers) {
        _context->get_device().get_device_function_table()
          .vkCmdExecuteCommands(_buffer, num_commands, _buffers);
        return *this;
      }

      CommandBuffer& end() {
        VK_ASSERT(_context->get_device().get_device_function_table()
          .vkEndCommandBuffer(_buffer), "failed to end command buffer");
        return *this;
      }

      const VkCommandBuffer get_command_buffer() const {
        return _buffer;
      }
    };

//    struct DispatchInfo {
//      union {
//        struct {
//          Buffer* vertex_buffer = nullptr;
//          u32 vertex_count;
//          u32 instance_count;
//          u32 first_vertex;
//          u32 first_instance;
//        } draw;
//
//        struct {
//          Buffer* vertex_buffer = nullptr;
//          Buffer* index_buffer = nullptr;
//          u32 index_count;
//          u32 instance_count;
//          u32 vertex_offset;
//          u32 first_index;
//          u32 first_instance;
//        } draw_indexed;
//
//        struct { 
//          Buffer* indirect_buffer = nullptr;
//          VkDeviceSize offset = 0;
//          u32 count = 0;
//        } draw_indirect;
//
//        struct { 
//          Buffer* indirect_buffer = nullptr;
//          VkDeviceSize offset = 0;
//          u32 count = 0;
//        } draw_indexed_indirect;
//
//        struct {
//          u32 uint_x;
//          u32 uint_y;
//          u32 uint_z;
//        } compute;
//
//        struct {
//          Buffer* indirect_buffer = nullptr;
//          VkDeviceSize offset = 0;
//       } compute_indirect;
//
//      };
//    };

//    class DispatchSequence;
// 
//    class DispatchSequence {
//    private:
//      DispatchType _type = DispatchType::Max;
//      DispatchInfo _info{};
//    public:
//      DispatchSequence(const DispatchInfo info, const DispatchType type) : _type{type}, _info{info} {}
//
//      b8 operator()(Pipeline* pipeline, DescriptorPool* descriptor_pool, CommandBuffer& cmd) {
//        cmd.bind_pipeline(pipeline);
//        cmd.bind_descriptor_pool(descriptor_pool);
//        switch (_type) {
//          case DispatchType::Compute:
//            cmd.record_dispatch(_info.compute.uint_x, _info.compute.uint_y, _info.compute.uint_z);
//            break;
//          case DispatchType::ComputeIndirect:
//            cmd.record_dispatch_indirect(_info.compute_indirect.indirect_buffer, _info.compute_indirect.offset);
//            break;
//          case DispatchType::Draw:
//            cmd.record_draw(_info.draw.vertex_count, _info.draw.instance_count, _info.draw.first_vertex, _info.draw.first_instance);
//            break;
//          case DispatchType::DrawIndexed:
//            cmd.record_indexed_draw(_info.draw_indexed.index_count, _info.draw_indexed.instance_count, _info.draw_indexed.first_index, _info.draw_indexed.vertex_offset, _info.draw_indexed.first_instance);
//            break;
//          case DispatchType::DrawIndirect:
//            cmd.record_indirect_draw(_info.draw_indirect.indirect_buffer, _info.draw_indirect.offset, _info.draw_indirect.count, sizeof(VkDrawIndirectCommand));
//            break;
//          case DispatchType::DrawIndexedIndirect:
//            cmd.record_indexed_indirect_draw(_info.draw_indexed_indirect.indirect_buffer, _info.draw_indexed_indirect.offset, _info.draw_indexed_indirect.count, sizeof(VkDrawIndexedIndirectCommand));
//            break;
//          case DispatchType::Max:
//          default:
//            return false;
//        }
//        return true;
//      }
//    };   

    class CommandArena {
    private:
      Context* _context = nullptr;
      VkCommandPool _pool = VK_NULL_HANDLE;
      //
      Array<VkCommandBuffer> _command_buffers;

      u32 current_top = 0;
      // only allocates linearly, no freeing except all
      std::atomic_flag _flag = ATOMIC_FLAG_INIT;
    public:
      using CommandBufferHandle = u32;

      b8 init(Context* context, const CommandType command_type, const CommandBufferLevelType level_type, const u32 job_count, const u32 queue_handle);

      const VkCommandPool get_pool() const;

      b8 reset() {
        VK_CHECK(_context->get_device()
          .get_device_function_table()
          .vkResetCommandPool(_context->get_device().get_device()
            , _pool
            , 0)
          , "failed to reset secondary command pool");
        return true;
      }

      CommandBuffer obtain_command_buffer();

      b8 terminate() {
        if(_pool == VK_NULL_HANDLE) {
          return false;
        }
        _context->get_device()
          .get_device_function_table()
          .vkDestroyCommandPool(_context->get_device().get_device()
            , _pool
            , CALLBACKS());

        return true;
      }

    };

    // keep one per frame and reset before using
    class CommandPool {
    public:
      
    private:
      using command_pool_t        = Array<CommandArena>;
      using handle_pool_t         =          Array<u32>;

      Context*                       _context = nullptr;
      command_pool_t                    _command_arenas;
      handle_pool_t         _free_command_arena_handles;

      u32 top_arena = 0;

      //struct CommandInfo {
      //  CommandType _command_type = CommandType::Max;
      //  CommandBufferLevelType _level_type = CommandBufferLevelType::Max;
      //};
      //Array<CommandInfo> _command_infos;

    public:
      using ArenaHandle = typename command_pool_t::index_t;

      b8 init(Context* context, const u32 queue_handle, const CommandType command_type, const u32 parallelism = RoxiNumThreads, const u32 job_count = 1);

      // not thread safe, for preallocating, CommandArena is
      // combo of VkCommandPool and an Array of VkCommandBuffers
      CommandBuffer obtain_command_buffer(const ArenaHandle handle) {
        return _command_arenas[handle].obtain_command_buffer();
      }

      const ArenaHandle obtain_command_arena() {
        return (const ArenaHandle)_free_command_arena_handles[top_arena++];
      }

      void return_command_arena(const ArenaHandle handle) {
        _free_command_arena_handles[top_arena--] = handle;
      }

      b8 reset();

      b8 terminate();
    };

//    template<CommandType CommandT>
//    class CommandPoolBuilder {
//    private:
//      struct CommandArenaInfo {
//        CommandType type;
//        CommandBufferLevelType level;
//        u32 count;
//      };
//
//      Array<CommandArenaInfo> _arena_infos;
//
//    public:
//      using CommandPool = CommandPool<CommandT>;
//      using CommandHandle = typename CommandPool::ArenaHandle;
//
//      b8 init(const u32 arena_count = s_max_command_arenas) {
//
//        _arena_infos.move_ptr(ALLOCATE(sizeof(CommandArenaInfo) * arena_count));
//
//        return true;
//      }
//
//      const CommandHandle create_command_arena(const CommandType type, const CommandBufferLevelType level_type, const u32 count) {
//        const CommandHandle result = _arena_infos.get_size();
//        CommandArenaInfo& info = *(_arena_infos.push(1));
//        info.type = type;
//        info.level = level_type;
//        info.count = count;
//        return result;
//      }
//
//      b8 build(Context* context, CommandPool* pool) {
//        const auto size = _arena_infos.get_size();
//
//        pool->init(context);
//
//        for(u32 i = 0; i < size; i++) {
//          if(pool->create_command_type(_arena_infos[i].count, _arena_infos[i].type, _arena_infos[i].level) != i) {
//            LOGF(Error, "failed to create CommandArena %u in CommandPoolBuilder::build()", i);
//            return false;
//          }
//        }
//
//        return true;
//      }
//
//      b8 terminate() {
//        FREE(_arena_infos.get_buffer());
//        return true;
//      }
//
//    };


      //b8 submit
      //  ( const Semaphore* signal_semaphores
      //  , const Semaphore* wait_semaphores
      //  , const u32 signal_semaphore_count
      //  , const u32 wait_semaphore_count
      //  , const TimelineSemaphore* const timeline_semaphore
      //  , const u64 count_to_wait
      //  , const u64 count_to_signal
      //  ) 
      //{
      //  VkCommandBufferSubmitInfo command_buffer_info;
      //  command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
      //  command_buffer_info.pNext = nullptr;
      //  command_buffer_info.commandBuffer = _primary_buffer;
      //  command_buffer_info.deviceMask = 0;
      //  const u32 semaphore_count = signal_semaphore_count + wait_semaphore_count;
      //  Array<VkSemaphoreSubmitInfo, 256> semaphore_submits{ALLOCATE(sizeof(VkSemaphoreSubmitInfo) * (semaphore_count))};
      //  VkSemaphoreSubmitInfo* const submit_begin = semaphore_submits.push(semaphore_count);
      //  for(u64 i = 0; i < semaphore_count; i++) {
      //    if(i < signal_semaphore_count) {
      //      submit_begin[i].semaphore = signal_semaphores[i].get_semaphore();
      //    }
      //    const u32 wait_index = i - signal_semaphore_count;
      //    semaphore_submits[i].semaphore = wait_semaphores[wait_index].get_semaphore();
      //  }

      //  VkSubmitInfo2 info;
      //  info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
      //  info.pNext = nullptr;
      //  info.flags = 0;
      //  info.commandBufferInfoCount = 1;
      //  info.pCommandBufferInfos = &command_buffer_info;
      //  info.signalSemaphoreInfoCount = signal_semaphore_count;
      //  info.pSignalSemaphoreInfos = signal_semaphore_count ? semaphore_submits.get_buffer() : nullptr;
      //  info.waitSemaphoreInfoCount = wait_semaphore_count;
      //  info.pWaitSemaphoreInfos = wait_semaphore_count ? &semaphore_submits[signal_semaphore_count] : nullptr;

      //  VK_CHECK(_context->get_device().get_device_function_table()
      //    .vkQueueSubmit2
      //      ( _context->get_device().get_queue(get_queue_index<CommandT>())
      //      , 1                   // always submit 1 primary command buffer
      //      , &info
      //      , _fence.get_fence() /* signal fence when finished or signal timeline semaphore*/
      //      )
      //    , "failed to submit VkCommandBuffers in CommandSet::submit(Sempahore*, Semaphore*, u32, u32)"
      //  );
      //  return true;
      //}

      //const u64 get_timeline_count() const {
      //  return _timeline_semaphore.get_count(_context);
      //}

      //const TimelineSemaphore get_timeline_semaphore() const {
      //  return _timeline_semaphore;
      //}

      //const b8 is_fence_pending() const {
      //  return !_fence.is_signaled(_context);
      //}

      //const Fence get_fence() const {
      //  return _fence;
      //}
    //};

    // also maybe not
//    template<CommandType Type, u64 SecondaryCommandBufferCount>
//    class CommandLane {
//    private:
//      using BufferPoolT = CommandBufferPool<Type, SecondaryCommandBufferCount>;
//      Context* _context;
//      BufferPoolT _pool;
//      StackArray<VkCommandBuffer, SecondaryCommandBufferCount> _pending_secondary_buffers;
//      struct range_t {
//        u32 begin = MAX_u32;
//        u32 end = MAX_u32;
//      };
//      StackArray<range_t, s_max_pipelines_per_lane> _pending_secondary_ranges;
//      StackArray<u32, s_max_pipelines_per_lane> _dispatch_counts;
//      StackArray<Pipeline*, s_max_pipelines_per_lane> _pipeline_binding_lane;
//      StackArray<DescriptorPool*, s_max_pipelines_per_lane> _descriptor_binding_lane;
//
//    public:
//      b8 init(Context* context) {
//        _context = context;
//        RX_CHECK
//        ( _pool.init(context)
//        , "failed to create BufferPoolT"
//        );
//        return true;
//      }
//
//      u32 add_pipeline(Pipeline* new_pipeline, DescriptorPool* new_descriptors, const u32 dispatch_count) {
//        const u32 result = _pipeline_binding_lane.get_size();
//        *(_pipeline_binding_lane.push(1)) = new_pipeline;
//        *(_descriptor_binding_lane.push(1)) = new_descriptors;
//        *(_dispatch_counts.push(1)) = dispatch_count;
//        return result;
//      }
//
//      b8 reset() {
//        RX_CHECK
//        ( _pool.reset(_context)
//        , "failed to reset BufferPoolT"
//        );
//        return true;
//      }
//
//    };
//
//    template<u64 SecondaryCommandBufferCount>
//    class CommandLane<CommandType::Graphics, SecondaryCommandBufferCount> {
//    private:
//      using BufferPoolT = CommandBufferPool<CommandType::Graphics, SecondaryCommandBufferCount>;
//      Context* _context;
//      BufferPoolT _pool;
//      StackArray<VkCommandBuffer, SecondaryCommandBufferCount> _pending_secondary_buffers;
//      struct range_t {
//        u32 begin = MAX_u32;
//        u32 end = MAX_u32;
//      };
//      StackArray<range_t, s_max_pipelines_per_lane> _pending_secondary_ranges;
//      StackArray<u32, s_max_pipelines_per_lane> _dispatch_counts;
//      StackArray<Pipeline*, s_max_pipelines_per_lane> _pipeline_binding_lane;
//      StackArray<DescriptorPool*, s_max_pipelines_per_lane> _descriptor_binding_lane;
//      Buffer* _indirect_buffer;
//
//    public:
//      b8 init(Context* context) {
//        _context = context;
//        RX_CHECK
//        ( _pool.init(context, SecondaryCommandBufferCount)
//        , "failed to create BufferPoolT"
//        );
//        return true;
//      }
//
//      u32 add_pipeline(Pipeline* new_pipeline, DescriptorPool* new_descriptors, const u32 dispatch_count) {
//        const u32 result = _pipeline_binding_lane.get_size();
//        *(_pipeline_binding_lane.push(1)) = new_pipeline;
//        *(_descriptor_binding_lane.push(1)) = new_descriptors;
//        *(_dispatch_counts.push(1)) = dispatch_count;
//        return result;
//      }
//
//      b8 set_indirect_buffer(Buffer* const new_indirect_buffer) {
//        _indirect_buffer = new_indirect_buffer;
//        return true;
//      }
//
//      b8 reset() {
//        RX_CHECK
//        ( _pool.reset(_context)
//        , "failed to reset BufferPoolT"
//        );
//        return true;
//      }
//
//    };

//    template<CommandType CommandT, u64 SecondaryCommandCount>
//    b8 record(CommandLane<CommandT, SecondaryCommandCount>* lane, const u64 start, const u64 end) {
//      CommandBuffer<CommandT, CommandBufferLevelType::Primary> primary_commands{lane->_primary_buffer};
//      const u32 pipeline_count = lane->_pipeline_binding_lane.get_size();
//      u32 current_dispatch_count = 0;
//      for(u32 i = 0; i < pipeline_count; i++) {
//        current_dispatch_count = lane->_dispatch_counts[i];
//        if(current_dispatch_count < 128) {
//          Pipeline* pipeline_to_bind = lane->_pipeline_binding_lane[i];
//          VkPipelineBindPoint bind_point = pipeline_to_bind->get_pipeline_bind_point();
//          DescriptorPool* descriptors_to_bind = lane->_descriptor_binding_lane[i];
//          primary_commands
//            .bind_pipeline(lane->_context, lane->_pipeline_binding_lane[i])
//            .bind_descriptor_pool(lane->_context, lane->_descriptor_binding_lane[i]);
//          if(bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
//            primary_commands.record_dispatch(lane->_context, current_dispatch_count, 0, 0);
//          } else if(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
//            primary_commands.record_indexed_indirect_draw(lane->_context, *lane->_indirect_buffer, 0, current_dispatch_count, sizeof(VkDrawIndexedIndirectCommand));
//        } else {
//          const u32 sub_dispatches = current_dispatch_count / 128;
//          const u32 final_count = current_dispatch_count & (128 - 1);
//          for(u32 j = 0; j < sub_dispatches; j++) {
//            const u32 current_count = j == sub_dispatches - 1 ? final_count : 128;
//
//          }
//        }
//    }
//
//        }
//      }
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
