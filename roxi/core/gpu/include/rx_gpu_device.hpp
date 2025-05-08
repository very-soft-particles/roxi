// =====================================================================================
//
//       Filename:  rx_renderer.hpp
//
//    Description:  3D graphics renderer
//
//        Version:  1.0
//        Created:  2024-06-20 6:14:59 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
//#include "rx_rendergraph.hpp"
#include "rx_frame_manager.hpp"
#include "rx_vocab.h"
#include "vk_pipeline.hpp"
#include "vk_renderpass.hpp"
#include "window.h"
#include "vk_command.hpp"
#include "vk_context.h"
#include "vk_device.h"
#include "vk_image.hpp"
#include "vk_instance.h"
#include "vk_resource.hpp"
#include "vk_sync.hpp"

#if (OS_WINDOWS)
#define WINDOW_PREFIX win
#elif (OS_LINUX)
#define WINDOW_PREFIX linux
#elif (OS_MAC)
#define WINDOW_PREFIX mac
#else
#define WINDOW_PREFIX SDL
#endif

#define GPU_SYSTEM_TYPES(X) X(Render) X(Compute) X(Loader)

namespace roxi {

  struct GPUDeviceSettings {
	  static constexpr u32 SliceCountX = 16;
	  static constexpr u32 SliceCountY = 9;
	  static constexpr u32 SliceCountZ = 24;
	  static constexpr u32 ClusterCount = SliceCountX * SliceCountY * SliceCountZ;

	  // Note that this also has to be set inside the compute shader
	  static constexpr u32 MaxLightPerCluster = 150;
	  static constexpr u32 MaxLights = KB(1);

    static constexpr u32 StagingBufferSize = KB(64);

    static constexpr u32 MaxDrawIndirectCount = KB(1);

    static constexpr u32 RenderPassCount = 2;

    static constexpr u32 DefaultScreenWidth = 720;
    static constexpr u32 DefaultScreenHeight = 500;

    static constexpr u32 BufferCopiesPerFrame = 256;
    static constexpr u32 BufferImageCopiesPerFrame = 256;
    static constexpr u32 ImageCopiesPerFrame = 256;

    static constexpr u32 MaxComputeSystems = 128;
    static constexpr u32 MaxGraphicsSystems = 128;
    static constexpr u32 MaxSystems = MaxGraphicsSystems + MaxComputeSystems;

    static constexpr u32 MaxFenceCount = 32;
    static constexpr u32 MaxSemaphoreCount = 32;

    static constexpr u32 RenderQueueHandle = 0;
    static constexpr u32 TransferQueueHandle = 1;
    static constexpr u32 ComputeQueueHandle = 2;
  };

  static VkPipelineCache s_pipeline_cache;

  DECLARE_JOB(init_job);

  class GPUDeviceBuilder;

  class GPUDevice {
  public:
    using ResourcePoolHandle        = Array<vk::ResourcePool>::index_t;
    using PipelinePoolHandle        = Array<vk::PipelinePool>::index_t;
    using DescriptorPoolHandle      = Array<vk::DescriptorPool>::index_t;
    using FramebufferHandle         = Array<vk::Framebuffer>::index_t;
    using QueueHandle               = Array<VkQueue>::index_t;
    using SemaphoreHandle           = Array<vk::Semaphore>::index_t;
    using FenceHandle               = Array<vk::Fence>::index_t;
    using TimelineHandle            = Array<vk::TimelineSemaphore>::index_t;

    using CommandPoolHandle = typename Array<vk::CommandPool>::index_t;

    using CommandArenaHandle = typename vk::CommandPool::ArenaHandle;

//    // compute pipelines
//    static constexpr u8 SkinningPipelineIndex = 0;
//    static constexpr u8 FrustumCullingPipelineIndex = 1;
//    static constexpr u8 AABBGeneratorPipelineIndex = 2;
//    static constexpr u8 LightCullingPipelineIndex = 3;
//    // graphics pipelines
//    static constexpr u8 LightingPipelineIndex = 4;
//    static constexpr u9 PBRClusterForwardPipelineIndex = 5; // with forward transparency ??

  private:
    class Loader {
    public:
      static constexpr u32 CommandPoolCount = 2;
      static constexpr vk::QueueType QueueType = vk::QueueType::Transfer;

    private:
      GPUDevice::QueueHandle _queue_handle;
      GPUDevice::ResourcePoolHandle      _resource_pool_handle;
      GPUDevice::CommandPoolHandle _command_pool_handles_begin;
      vk::CommandBuffer                _current_primary_buffer;
      vk::ResourcePool::BufferHandle                     _staging_buffer_handles_begin;
      vk::ResourcePool::BufferHandle                    _transfer_source_handles_begin;
      Array<void*>                                     _staging_buffer_mapped_pointers;
      // lock free array of copies
      VkBufferCopy*                                               _buffer_copies;
      Counter                                                 _buffer_copies_top = 0;
      //Array<VkBufferImageCopy>                                    _buffer_image_copies;
      //Array<VkImageCopy>                                                 _image_copies;

      Counter _current_buffer_top = 0;
      u8 _current_buffer = 0;

      SemaphoreHandle                   _transfer_submission_semaphores[Loader::CommandPoolCount];
      FenceHandle                             _transfer_blocking_fences[Loader::CommandPoolCount];

      const u8 get_buffer_handle_from_ptr(const void* const ptr);

    public:
      b8 init(const GPUDevice::QueueHandle queue_handle);

      void create_resources(GPUDevice* device);

      // reactor job, may wait fiber
      const b8 transfer(GPUDevice* device);

      vk::Semaphore& get_signal_semaphore(GPUDevice* device);

      const vk::Buffer get_transfer_source(GPUDevice* device);

      // reactor job, may wait fiber
      const u8 swap_buffers_and_submit(GPUDevice* device);

      const SemaphoreHandle get_corresponding_semaphore_for_staging_ptr(const void* const ptr);

      const vk::Buffer& get_corresponding_transfer_buffer_for_staging_ptr(GPUDevice* device, const void* const ptr);

      // maps buffer to buffer
      const b8 queue_copy(const u32 offset, const u32 size);

      const u32 push_to_staging_buffer(const u32 push_bytes);

      void* get_staging_buffer_raw_pointer();

      // returns to offset
      const u32 copy_to_staging_buffer(void* src, const u32 num_bytes_to_copy);

      void create_commands(GPUDevice* device);

      b8 terminate(GPUDevice* device = nullptr);

    } _loader;

  public:

    b8 init(void* window, const u32 system_count, const vk::QueueType* system_queue_types);

    b8 create_framebuffers(const u32 count, vk::Framebuffer* const framebuffers, const vk::FramebufferCreation* const creations, const ResourcePoolHandle resource_pool_handle);

    const PipelinePoolHandle create_pipeline_pool(const DescriptorPoolHandle descriptor_pool_handle, const u32 pipeline_count, const u32 render_pass_count, vk::PipelineInfo* pipeline_infos, vk::RenderPassInfo* render_pass_infos);

    const ResourcePoolHandle create_resource_pool(const u32 buffer_count, const u32 image_count, const gpu::ResourceInfo* buffer_infos, const gpu::ResourceInfo* image_infos);

    const FenceHandle create_fence(const b8 init_signaled = true) {
      const FenceHandle result = _fence_pool.get_size();
      RX_RETURN((_fence_pool.push(1))->init(&_context, init_signaled)
        , "failed to create fence"
        , lofi::index_type_max<FenceHandle>::value);
      RX_END_RESULT(result);
    }

    const b8 check_fence(const FenceHandle handle) {
      RX_END_RESULT(_fence_pool[handle].is_signaled(&_context));
    }

    const void reset_fence(const FenceHandle handle) {
      RX_END_RESULT(_fence_pool[handle].reset(&_context));
    }


    vk::Fence& obtain_fence(const FenceHandle handle) {
      RX_END_RESULT(_fence_pool[handle]);
    }

    vk::Semaphore& obtain_semaphore(const SemaphoreHandle handle) {
      RX_END_RESULT(_semaphore_pool[handle]);
    }

    const SemaphoreHandle create_semaphore() {
      const SemaphoreHandle result = _semaphore_pool.get_size();
      (_semaphore_pool.push(1))->init(&_context);
      RX_END_RESULT(result);
    }

    vk::ResourcePool& obtain_resource_pool(const ResourcePoolHandle handle) {
      RX_END_RESULT(_resource_pools[handle]);
    }

    const vk::PipelinePool& obtain_pipeline_pool(const PipelinePoolHandle handle) const;

    const VkQueue obtain_queue(const QueueHandle handle) const;

    // not internally synchronized, one pool per system frame
    const CommandPoolHandle create_command_pools(const vk::CommandType command_type, const QueueHandle queue_handle, const u32 pool_count, const u32 parallelism, const u32 job_count);

    vk::CommandPool& obtain_command_pool(const CommandPoolHandle handle) {
      RX_END_RESULT(_command_pools[handle]);
    }

    const vk::Extent<2> get_current_extent() const;

    const DescriptorPoolHandle create_descriptor_pool(const u32 ubo_count, const u32 storage_count, const u32 texture_count, const u32 image_count);

    vk::DescriptorPool& obtain_descriptor_pool(const DescriptorPoolHandle handle);

    const u32 push_to_staging_buffer(const u32 byte_count) {
      return _loader.push_to_staging_buffer(byte_count);
    }

    void* get_staging_buffer_raw_pointer() {
      return _loader.get_staging_buffer_raw_pointer();
    }

    const SemaphoreHandle get_transfer_signal_semaphore(const void* const buffer_ptr) {
      return _loader.get_corresponding_semaphore_for_staging_ptr(buffer_ptr);
    }

    const vk::Buffer& get_transfer_buffer(const void* const buffer_ptr) {
      return _loader.get_corresponding_transfer_buffer_for_staging_ptr(this, buffer_ptr);
    }
    // current implementation works through the use of an internal synchronization mechanism
    // which does not rely on vkWaitForFences and uses the fiber wait functionality to ensure
    // that the function is non-blocking, there is the option of using the standard
    // vkWaitForFences vulkan API call, but it means that this function MUST be called from
    // inside of a reactor job
    b8 transfer_queue_submit_immediate(vk::CommandBuffer buffer_to_submit, const FenceHandle wait_fence, const vk::Semaphore signal_semaphore);

    // non blocking, but must be called from a fiber as it will place a job on the wait list
    void wait_for_fence(const FenceHandle handle);

    b8 queue_submit(const QueueHandle queue, const vk::CommandBuffer buffer_to_submit, const u32 wait_semaphore_count, const SemaphoreHandle* wait_semaphores, const u32 signal_semaphore_count, const SemaphoreHandle* signal_semaphores, FenceHandle signal_fence = {});

    b8 queue_frame_submit(const frame::ID frame_id, const QueueHandle queue, const vk::CommandBuffer buffer_to_submit);

    b8 terminate();

  private:
    // helper functions
    WINDOW_PREFIX::Window                                                              _window;

    vk::Context                                                                       _context;

    Array<vk::ResourcePool>                                                     _resource_pools;
   // contains renderpasses and layouts
    Array<vk::PipelinePool>                                                     _pipeline_pools;

    Array<vk::DescriptorPool>                                                 _descriptor_pools;

    Array<vk::CommandPool>                                                       _command_pools;
    
    Array<VkQueue>                                                                  _queue_pool;

    Array<QueueHandle>                                                           _update_queues;

    Array<Counter>                                                              _frame_counters;
    //  always 1 more than the last frame submitted
    u64                                                                  _current_working_frame;

    // one per queue
    Array<vk::TimelineSemaphore>                                                     _timelines;

    // fences
    Array<vk::Fence>                                                                _fence_pool;

    // semaphores
    Array<vk::Semaphore>                                                        _semaphore_pool;


    u32 _current_width = 0;
    u32 _current_height = 0;
  };

  class Renderer {
  private:
    GPUDevice* _device = nullptr;
    GPUDevice::QueueHandle _queue_handle;
    GPUDevice::FenceHandle _transfer_fence;
    GPUDevice::ResourcePoolHandle _resource_pool;
    GPUDevice::CommandPoolHandle _command_pool;
    GPUDevice::PipelinePoolHandle _pipeline_pool;
    GPUDevice::DescriptorPoolHandle _descriptor_pool;

  public:
    static constexpr vk::QueueType QueueType = vk::QueueType::Render;

  protected:
    using framebuffer_pool_t = Array<vk::Framebuffer>;
    using FramebufferHandle = typename framebuffer_pool_t::index_t;

    framebuffer_pool_t _framebuffers;

    b8 init(GPUDevice* device, const GPUDevice::QueueHandle queue_handle);

    b8 create_resources(const u32 buffer_count, const u32 image_count, const gpu::ResourceInfo *buffer_infos, const gpu::ResourceInfo *image_infos) {
      _resource_pool = _device->create_resource_pool(buffer_count, image_count, buffer_infos, image_infos);
      RX_CHECK(_resource_pool != lofi::index_type_max<GPUDevice::ResourcePoolHandle>::value
        , "failed to create resource pool");
      _transfer_fence = _device->create_fence(false);
      RX_CHECK(_transfer_fence != lofi::index_type_max<GPUDevice::FenceHandle>::value
        , "failed to create transfer fence");
      RX_END();
    }

    b8 create_descriptors(const u32 ubo_count, const u32 storage_buffer_count, const u32 texture_count, const u32 storage_image_count) {
      _descriptor_pool = _device->create_descriptor_pool(ubo_count, storage_buffer_count, texture_count, storage_image_count);
      RX_CHECK(_descriptor_pool != lofi::index_type_max<GPUDevice::DescriptorPoolHandle>::value
        , "failed to create descriptor pool");
      RX_END();
    }

    b8 create_commands(const u32 frame_count = RoxiNumFrames, const u32 job_count = 1, const u32 concurrent_thread_count = 1) {
      _command_pool = _device->create_command_pools(vk::CommandType::Graphics, _queue_handle, frame_count, concurrent_thread_count, job_count);
      RX_CHECK(_command_pool != lofi::index_type_max<GPUDevice::CommandPoolHandle>::value
          , "failed to create command pool");
      RX_END();
    }

    b8 create_pipelines(const u32 pipeline_count, const u32 render_pass_count, vk::PipelineInfo* pipeline_infos, vk::RenderPassInfo* render_pass_infos) {
      RX_TRACEF("create pipeline pool with device at %llu", PTR2INT(_device));
      _pipeline_pool = _device->create_pipeline_pool(_descriptor_pool, pipeline_count, render_pass_count, pipeline_infos, render_pass_infos);
      RX_CHECK(_pipeline_pool != lofi::index_type_max<GPUDevice::PipelinePoolHandle>::value
        , "failed to create pipeline pool");
      RX_END();
    }

    const FramebufferHandle create_framebuffers(const u32 count, const vk::FramebufferCreation* const creations) {
      const FramebufferHandle result = _framebuffers.get_size();
      RX_CHECKF(_device->create_framebuffers(count, _framebuffers.push(count), creations, _resource_pool)
        , "Renderer failed to create %u framebuffers", count);
      RX_END_RESULT(result);
    }

    void* transfer_allocate(const u32 size) {
      const auto offset = _device->push_to_staging_buffer(size);
      RX_RETURN(offset == MAX_u32, "failed to allocate to transfer buffer in Renderer", nullptr);
      u8* result = (u8*)_device->get_staging_buffer_raw_pointer() + offset;
      return (void*)result;
    }

    const GPUDevice::SemaphoreHandle get_transfer_semaphore(const void* const ptr) {
      return _device->get_transfer_signal_semaphore(ptr);
    }

    const u32 get_transfer_source_offset(const void* const buffer_ptr) {
      return (u8*)buffer_ptr - (u8*)_device->get_staging_buffer_raw_pointer();
    }

    const vk::Buffer& get_transfer_source_buffer(const void* const ptr) {
      return _device->get_transfer_buffer(ptr);
    }

    const vk::Framebuffer& obtain_framebuffer(const FramebufferHandle handle) {
      RX_END_RESULT(_framebuffers[handle]);
    }

    vk::ResourcePool& get_resource_pool() {
      RX_END_RESULT(_device->obtain_resource_pool(_resource_pool));
    }

    const vk::PipelinePool& get_pipeline_pool() {
      RX_END_RESULT(_device->obtain_pipeline_pool(_pipeline_pool));
    }

    vk::CommandPool& get_command_pool(const u32 frame_id) {
      RX_END_RESULT(_device->obtain_command_pool(_command_pool + frame_id));
    }

    vk::DescriptorPool& get_descriptor_pool() {
      RX_END_RESULT(_device->obtain_descriptor_pool(_descriptor_pool));
    }

    // returns staging buffer offset to descriptor
    const u32 copy_descriptor_to_staging_buffer(const vk::ResourcePool::BufferHandle buffer_handle, const vk::DescriptorAllocation descriptor_allocation) {
      const auto offset = _device->push_to_staging_buffer(descriptor_allocation.size);

      vk::Buffer& buffer = get_resource_pool().obtain_buffer(buffer_handle);
      RX_RETURNF(get_descriptor_pool().get_descriptor(buffer, buffer.get_buffer_type(), descriptor_allocation, (void*)((u8*)_device->get_staging_buffer_raw_pointer() + offset))
          , MAX_u32
          , "failed to get descriptor for buffer handle %u"
          , buffer_handle);
      return offset;
    }

    void wait_for_transfer_completion() {
      return _device->wait_for_fence(_transfer_fence);
    }

    b8 check_transfer_for_completion() {
      return _device->check_fence(_transfer_fence);
    }

    void reset_transfer_fence() {
      return _device->reset_fence(_transfer_fence);
    }

    b8 submit_transfer(const vk::CommandBuffer buffer, const GPUDevice::SemaphoreHandle transfer_wait_semaphore) {
      return _device->queue_submit(_queue_handle, buffer, 1, &transfer_wait_semaphore, 0, nullptr,_transfer_fence);
    }

    b8 submit(const frame::ID id, const vk::CommandBuffer command_buffer) {
      RX_CHECKF(_device->queue_frame_submit(id, _queue_handle, command_buffer)
        , "Renderer failed to submit frame::ID id = %u"
        , id
        );
      RX_END();
    }

  protected:
    const vk::Extent<2> get_current_extent() const {
      RX_END_RESULT(_device->get_current_extent());
    }
  };

  class GPUCompute {
  private:
    GPUDevice* _device;
    GPUDevice::QueueHandle _queue_handle;
    GPUDevice::FenceHandle _transfer_fence;
    GPUDevice::ResourcePoolHandle _resource_pool_handle;
    GPUDevice::CommandPoolHandle   _command_pool_handles_begin;
    GPUDevice::PipelinePoolHandle _pipeline_pool_handle;
    GPUDevice::DescriptorPoolHandle _descriptor_pool_handle;
  public:
    static constexpr vk::QueueType QueueType = vk::QueueType::Compute;

  protected:
    b8 init(GPUDevice* device, const GPUDevice::QueueHandle queue_handle) {
      _device = device;
      _queue_handle = queue_handle;

      RX_END();
    }

    void create_resources(const u32 buffer_count, const u32 image_count, const gpu::ResourceInfo *buffer_infos, const gpu::ResourceInfo *image_infos) {
      _resource_pool_handle = _device->create_resource_pool(buffer_count, image_count, buffer_infos, image_infos);
    }

    void create_descriptors(const u32 ubo_count, const u32 storage_buffer_count, const u32 texture_count, const u32 storage_image_count) {
      _descriptor_pool_handle = _device->create_descriptor_pool(ubo_count, storage_buffer_count, texture_count, storage_image_count);
    }

    void create_commands(const u32 frame_count = RoxiNumFrames, const u32 concurrent_thread_count = 1, const u32 job_count = 1) {
      _command_pool_handles_begin = _device->create_command_pools(vk::CommandType::Compute, _queue_handle, frame_count, concurrent_thread_count, job_count);
    }

    void create_pipelines(const u32 pipeline_count, const u32 render_pass_count, vk::PipelineInfo* pipeline_infos, vk::RenderPassInfo* render_pass_infos) {
      _pipeline_pool_handle = _device->create_pipeline_pool(_descriptor_pool_handle, pipeline_count, render_pass_count, pipeline_infos, render_pass_infos);
    }

    void* transfer_allocate(const u32 size) {
      const auto offset = _device->push_to_staging_buffer(size);
      RX_RETURN(offset == MAX_u32, "failed to allocate to transfer buffer in Renderer", nullptr);
      u8* result = (u8*)_device->get_staging_buffer_raw_pointer() + offset;
      return (void*)result;
    }

    const GPUDevice::SemaphoreHandle get_transfer_semaphore(const void* const ptr) {
      return _device->get_transfer_signal_semaphore(ptr);
    }

    const u32 get_transfer_source_offset(const void* const buffer_ptr) {
      return (u8*)buffer_ptr - (u8*)_device->get_staging_buffer_raw_pointer();
    }

    const vk::Buffer& get_transfer_source_buffer(const void* const ptr) {
      return _device->get_transfer_buffer(ptr);
    }

    vk::ResourcePool& get_resource_pool() {
      RX_END_RESULT( _device->obtain_resource_pool(_resource_pool_handle));
    }

    const vk::PipelinePool& get_pipeline_pool() {
      RX_END_RESULT(_device->obtain_pipeline_pool(_pipeline_pool_handle));
    }

    vk::CommandPool& get_command_pool(const frame::ID frame_id) {
      RX_END_RESULT(_device->obtain_command_pool(_command_pool_handles_begin + frame_id));
    }

    vk::DescriptorPool& get_descriptor_pool() {
      RX_END_RESULT(_device->obtain_descriptor_pool(_descriptor_pool_handle));
    }

    b8 get_descriptor(const vk::ResourcePool::BufferHandle buffer_handle, const vk::DescriptorAllocation descriptor_allocation, void* ptr) {
      vk::Buffer& buffer = get_resource_pool().obtain_buffer(buffer_handle);
      RX_END_RESULT(get_descriptor_pool().get_descriptor(buffer, buffer.get_buffer_type(), descriptor_allocation, ptr));
    }

    b8 submit(const frame::ID id, const vk::CommandBuffer buffer) {
      RX_END_RESULT(_device->queue_frame_submit(id, _queue_handle, buffer));
    }

    const vk::Extent<2> get_current_extent() const {
      RX_END_RESULT(_device->get_current_extent());
    }
  };
}		// -----  end of namespace roxi  ----- 
