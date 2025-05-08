// =====================================================================================
//
//       Filename:  rx_renderer.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-09-10 9:59:52 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_allocator.hpp"
#include "rx_gpu_device.hpp"
#include "vk_renderpass.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {

  const u8 GPUDevice::Loader::get_buffer_handle_from_ptr(const void* const ptr) {
    if(ptr >= _staging_buffer_mapped_pointers[0] && ptr < (void*)((u8*)_staging_buffer_mapped_pointers[0] + GPUDeviceSettings::StagingBufferSize)) {
      return 0;
    }
    if(ptr >= _staging_buffer_mapped_pointers[1] && ptr < (void*)((u8*)_staging_buffer_mapped_pointers[1] + GPUDeviceSettings::StagingBufferSize)) {
      return 1;
    }
    return MAX_u8;
  }

  b8 GPUDevice::Loader::init(const GPUDevice::QueueHandle queue_handle) {
    _queue_handle = queue_handle;
    u8* buffer = (u8*)ALLOCATE
      ( sizeof(void*) * CommandPoolCount
      + sizeof(VkBufferCopy) * GPUDeviceSettings::BufferCopiesPerFrame
//      + sizeof(VkBufferImageCopy) * GPUDeviceSettings::BufferImageCopiesPerFrame
//      + sizeof(VkImageCopy) * GPUDeviceSettings::ImageCopiesPerFrame
      );

    _buffer_copies = (VkBufferCopy*)buffer;
    buffer += sizeof(VkBufferCopy) * GPUDeviceSettings::BufferCopiesPerFrame;

    _staging_buffer_mapped_pointers.move_ptr(buffer);
    _staging_buffer_mapped_pointers.push(CommandPoolCount);
    buffer += sizeof(void*) * CommandPoolCount;

//    _buffer_image_copies.move_ptr(buffer);
//    buffer += sizeof(VkBufferImageCopy) * GPUDeviceSettings::BufferImageCopiesPerFrame;
//
//    _image_copies.move_ptr(buffer);
    RX_END();
  }

  b8 GPUDevice::init(void* window, const u32 system_count, const vk::QueueType* system_queue_types) {

    RX_TRACE("allocating gpu memory");
    u8* buffer = (u8*)ALLOCATE(
        sizeof(vk::PipelinePool) * (system_count + 1)
      + sizeof(vk::CommandPool) * (system_count + 1)
      + sizeof(vk::ResourcePool) * (system_count + 1)
      + sizeof(vk::DescriptorPool) * (system_count + 1)
      + sizeof(VkQueue) * (system_count + 1)
      + sizeof(vk::TimelineSemaphore) * (system_count + 1)
      + sizeof(vk::Fence) * GPUDeviceSettings::MaxFenceCount
      + sizeof(vk::Semaphore) * GPUDeviceSettings::MaxSemaphoreCount
    );

    WINDOW_PREFIX::Window* window_ptr = (WINDOW_PREFIX::Window*)window;
    const auto extents = window_ptr->get_extents();
    _current_width = extents.width;
    _current_height = extents.height;

    vk::ContextBuilder context_builder;

    RX_TRACE("initializing vulkan context builder");
    RX_CHECK(context_builder.init()
        , "failed to initialize vk::ContextBuilder in GPUDevice init()");

    VkPhysicalDeviceVulkan12Features device_features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    device_features12.timelineSemaphore = true;
    device_features12.descriptorIndexing = true;
    device_features12.bufferDeviceAddress = true;
    device_features12.runtimeDescriptorArray = true;

    //VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    //indexing_features.runtimeDescriptorArray = VK_TRUE;

    //device_features12.pNext = &indexing_features;

    RX_TRACE("adding vulkan extensions");
    context_builder
      .add_instance_extension(VK_KHR_SURFACE_EXTENSION_NAME)
      .add_instance_extension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
      .add_instance_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)
      .add_instance_layer("VK_LAYER_KHRONOS_validation")
      .add_device_extension("VK_KHR_swapchain")
      .add_device_extension("VK_KHR_shader_draw_parameters")
      .add_device_extension("VK_KHR_buffer_device_address")
      .add_device_extension("VK_EXT_descriptor_indexing")
      .add_device_extension("VK_EXT_descriptor_buffer")
      .add_device_extension("VK_KHR_timeline_semaphore")
      .add_device_extension("VK_EXT_pipeline_creation_cache_control")
      .add_device_p_next(&device_features12)
      .set_window(window);

//    // currently queue handles are static but could be changed so that the implementation
//    // supports a dynamic number of queues
//    // queue_handle = 0
//    context_builder.add_queue_type(vk::QueueType::Render);
//    // queue_handle = 1
//    context_builder.add_queue_type(vk::QueueType::Transfer);
//    // queue_handle = 2
//    context_builder.add_queue_type(vk::QueueType::Compute);

    RX_TRACE("adding vulkan queue types");
    for(u32 i = 0; i < system_count; i++) {
      context_builder.add_queue_type(system_queue_types[i]);
    }

    RX_TRACE("building vulkan context");
    RX_CHECK(context_builder.build(&_context)
        , "failed to build roxi::vk::Context in roxi::GPUDevice::init()");

    _pipeline_pools.move_ptr(buffer);
    buffer += sizeof(vk::PipelinePool) * (system_count + 1);
    _pipeline_pools.clear();

    _command_pools.move_ptr(buffer);
    buffer += sizeof(vk::CommandPool) * (system_count + 1);
    _command_pools.clear();

    _resource_pools.move_ptr(buffer);
    buffer += sizeof(vk::ResourcePool) * (system_count + 1);
    _resource_pools.clear();

    _descriptor_pools.move_ptr(buffer);
    buffer += sizeof(vk::DescriptorPool) * (system_count + 1);
    _descriptor_pools.clear();

    _queue_pool.move_ptr(buffer);
    VkQueue* const queues_begin = _queue_pool.push(system_count + 1);
    _queue_pool.clear();

    RX_TRACE("getting vulkan queues");
    for(u32 i = 0; i < system_count; i++) {
      queues_begin[i] = _context.get_queue(i);
    }

    buffer += sizeof(VkQueue) * system_count;

    // one primary command per queue per frame

    _timelines.move_ptr(buffer);
    _timelines.push(system_count);
    
    buffer += sizeof(vk::TimelineSemaphore) * system_count;
    _fence_pool.move_ptr(buffer);

    buffer += sizeof(vk::Fence) * GPUDeviceSettings::MaxFenceCount;
    _semaphore_pool.move_ptr(buffer);

    RX_END();
  }

  b8 GPUDevice::create_framebuffers(const u32 count, vk::Framebuffer* const framebuffers, const vk::FramebufferCreation* const creations, const ResourcePoolHandle resource_pool_handle) {
    for(u32 i = 0; i < count; i++) {
      RX_TRACEF("creating framebuffer %u", i);
      RX_CHECKF(framebuffers[i].init(&_context, creations[i], obtain_resource_pool(resource_pool_handle))
          , "failed to create framebuffer id %u with resource pool %u", i, resource_pool_handle);
    }
    RX_END();
  }

  const GPUDevice::PipelinePoolHandle GPUDevice::create_pipeline_pool(const DescriptorPoolHandle descriptor_pool_handle, const u32 pipeline_count, const u32 render_pass_count, vk::PipelineInfo* pipeline_infos, vk::RenderPassInfo* render_pass_infos) {
    vk::PipelinePoolBuilder builder;
    builder.init(&_context);

    for(u32 i = 0; i < render_pass_count; i++) {
      vk::RenderPassInfo& info = render_pass_infos[i];
      builder.add_render_pass(info.colour_attachments.get_size(), info.colour_attachments.get_buffer(), info.depth_attachment);
    }

    for(u32 i = 0; i < pipeline_count; i++) {
      builder.add_pipeline(pipeline_infos[i]);
    }

    const PipelinePoolHandle result = _pipeline_pools.get_size();
    RX_TRACEF("building pipeline pool %u", result);
    RX_RETURN(builder.build(_pipeline_pools.push(1))
      , "failed to create pipeline pool"
      , lofi::index_type_max<PipelinePoolHandle>::value);
    builder.terminate();
    RX_TRACE("returning from create pipeline pool");
    RX_END_RESULT(result);
  }

  const GPUDevice::ResourcePoolHandle GPUDevice::create_resource_pool(const u32 buffer_count, const u32 image_count, const gpu::ResourceInfo* buffer_infos, const gpu::ResourceInfo* image_infos) {
    RX_TRACEF("creating resource pool with buffer_count = %u, and image_count = %u", buffer_count, image_count);
    const ResourcePoolHandle result = _resource_pools.get_size();
    vk::ResourcePool& pool = *(_resource_pools.push(1));

    RX_RETURN(pool.init(&_context, buffer_count, buffer_infos, image_count, image_infos)
        , "failed to initialize ResourcePool in GPUDevice::create_resource_pool()", lofi::index_type_max<ResourcePoolHandle>::value);

    RX_END_RESULT(result);
  }


    // not internally synchronized, one pool per system frame
  const GPUDevice::CommandPoolHandle GPUDevice::create_command_pools(const vk::CommandType command_type, const QueueHandle queue_handle, const u32 pool_count, const u32 parallelism, const u32 job_count) {

    CommandPoolHandle result = _command_pools.get_size();
    vk::CommandPool* const command_pools_begin = _command_pools.push(pool_count);
    RX_TRACEF("initializing %u command pools", pool_count);
    for(u32 i = 0; i < pool_count; i++) {
      RX_TRACEF("initializing pool %u", i);
      RX_RETURNF(command_pools_begin[i].init(&_context, queue_handle, command_type, parallelism, job_count)
        , lofi::index_type_max<CommandPoolHandle>::value
        , "failed to initialize command pool %u for queue %u"
        , i
        , queue_handle
        );
    }
    RX_TRACE("initializing timelines");
    RX_RETURNF(_timelines[queue_handle].init(&_context, 0)
      , lofi::index_type_max<CommandPoolHandle>::value
      , "failed to initialize timeline for queue %u"
      , queue_handle
      );
    RX_END_RESULT(result);
  }

  // current implementation works through the use of an internal synchronization mechanism
  // which does not rely on vkWaitForFences and uses the fiber wait functionality to ensure
  // that the function is non-blocking, there is the option of using the standard
  // vkWaitForFences vulkan API call, but it means that this function MUST be called from
  // inside of a reactor job
  b8 GPUDevice::transfer_queue_submit_immediate(vk::CommandBuffer buffer_to_submit, const FenceHandle wait_fence, const vk::Semaphore signal_semaphore) {
    // fiber will wait on this job to finish
    // must be call from a fiber
    wait_for_fence(wait_fence);

//    alternative standard blocking implementation, if used, this function cannot be called
//    from a fiber
//    VK_CHECK(_context.get_device().get_device_function_table()
//      .vkWaitForFences(_context.get_device().get_device(), 1, &wait_fence.get_fence(), VK_TRUE, 1000000)
//      , "wait for vk::Fence in roxi::vk::GPUDevice::transfer_queue_submit_immediate() timed out");
//

    _context.get_device().get_device_function_table()
      .vkResetFences(_context.get_device().get_device(), 1, &obtain_fence(wait_fence).get_fence());

    VkCommandBufferSubmitInfo command_info{};
    command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    command_info.commandBuffer = buffer_to_submit.get_command_buffer();

    VkSemaphoreSubmitInfo signal_info{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    signal_info.semaphore = signal_semaphore.get_semaphore();

    VkSubmitInfo2 submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_info;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    submit_info.signalSemaphoreInfoCount = 1;

    _context.get_device().get_device_function_table()
      .vkQueueSubmit2(_queue_pool[GPUDeviceSettings::TransferQueueHandle], 1, &submit_info, obtain_fence(wait_fence).get_fence());

    RX_END();
  }

  // non blocking, but must be called from a fiber as it will place a job on the wait list
  void GPUDevice::wait_for_fence(const FenceHandle handle) {
    Counter counter = 0;
    vk::wait_on_fence(&_context, &obtain_fence(handle), &counter);
  }

  b8 GPUDevice::queue_submit(const QueueHandle queue, const vk::CommandBuffer buffer_to_submit, const u32 wait_semaphore_count, const SemaphoreHandle* wait_semaphores, const u32 signal_semaphore_count, const SemaphoreHandle* signal_semaphores, FenceHandle signal_fence) {
    //const auto current_frame_count = _frame_counters[frame_id].add(1);

    VkCommandBufferSubmitInfo command_info{};
    command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    command_info.commandBuffer = buffer_to_submit.get_command_buffer();

    VkSemaphoreSubmitInfo wait_infos[16];

    for(u32 i = 0; i < wait_semaphore_count; i++) {
      wait_infos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
      // hacky workaround for the fact that the compute queue is static constexpr = 2 and transfer = 1
      wait_infos[i].semaphore = obtain_semaphore(wait_semaphores[i]).get_semaphore();
      wait_infos[i].value = 0;
    }

    VkSemaphoreSubmitInfo signal_infos[16];

    for(u32 i = 0; i < wait_semaphore_count; i++) {
      signal_infos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
      // hacky workaround for the fact that the compute queue is static constexpr = 2 and transfer = 1
      signal_infos[i].semaphore = obtain_semaphore(signal_semaphores[i]).get_semaphore();
      signal_infos[i].value = 0;
    }     
    VkSubmitInfo2 submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
    // one primary command buffer per queue currently just 2 queues
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_info;
    submit_info.pSignalSemaphoreInfos = signal_infos;
    submit_info.pWaitSemaphoreInfos = wait_infos;
    submit_info.signalSemaphoreInfoCount = wait_semaphore_count;
    submit_info.waitSemaphoreInfoCount = signal_semaphore_count;

    RX_CHECKF(_context.get_device().get_device_function_table()
      .vkQueueSubmit2(_queue_pool[queue], 1, &submit_info, obtain_fence(signal_fence).get_fence())
      , "failed to properly submit primary command buffer with queue handle = %u"
      , queue);

    RX_END();
  }

  void GPUDevice::Loader::create_resources(GPUDevice* device) {

    StackArray<gpu::ResourceInfo> resources;
    _staging_buffer_handles_begin = 0;
    for(u32 i = 0; i < CommandPoolCount; i++) {
      gpu::ResourceInfo& staging_buffer_info = *(resources.push(1));
      staging_buffer_info.type = gpu::ResourceType::StagingBuffer;
      staging_buffer_info.buffer.size = GPUDeviceSettings::StagingBufferSize;
    }

    _transfer_source_handles_begin = CommandPoolCount;
    for(u32 i = 0; i < CommandPoolCount; i++) {
      gpu::ResourceInfo& transfer_source_info = *(resources.push(1));
      transfer_source_info.type = gpu::ResourceType::TransferSource;
      transfer_source_info.buffer.size = GPUDeviceSettings::StagingBufferSize;
    }

    _resource_pool_handle = device->create_resource_pool(2 * CommandPoolCount, 0, resources.get_buffer(), nullptr);

    vk::ResourcePool& resource_pool = device->obtain_resource_pool(_resource_pool_handle);

    for(u32 i = 0; i < CommandPoolCount; i++) {
      _staging_buffer_mapped_pointers[i] = resource_pool.obtain_buffer(_staging_buffer_handles_begin + i).map();
      _transfer_blocking_fences[i] = device->create_fence(true);
      _transfer_submission_semaphores[i] = device->create_semaphore();
    }

  }

  // reactor job, may wait fiber
  const b8 GPUDevice::Loader::transfer(GPUDevice* device) {
    const auto top = _buffer_copies_top.get_count();
    vk::ResourcePool& pool = device->obtain_resource_pool(_resource_pool_handle);
    const vk::Buffer& staging_buffer = pool.obtain_buffer(_staging_buffer_handles_begin + _current_buffer);
    const vk::Buffer& transfer_source = pool.obtain_buffer(_transfer_source_handles_begin + _current_buffer);
    _current_primary_buffer.begin();
    _current_primary_buffer.record_copy_buffer(staging_buffer, transfer_source, top, _buffer_copies);
    _current_primary_buffer.end();
    swap_buffers_and_submit(device);
    RX_END();
  }

  vk::Semaphore& GPUDevice::Loader::get_signal_semaphore(GPUDevice* device) {
    RX_END_RESULT(device->obtain_semaphore(_transfer_submission_semaphores[_current_buffer]));
  }

  const vk::Buffer GPUDevice::Loader::get_transfer_source(GPUDevice* device) {
    RX_END_RESULT(device->obtain_resource_pool(_resource_pool_handle).obtain_buffer(_transfer_source_handles_begin + _current_buffer));
  }

  // reactor job, may wait fiber
  const u8 GPUDevice::Loader::swap_buffers_and_submit(GPUDevice* device) {
    const auto current_buffer_handle = _current_buffer;
    const vk::CommandBuffer current_buffer = _current_primary_buffer;
    _current_buffer ^= 1;
    _current_buffer_top.reset();
    vk::CommandPool& pool = device->obtain_command_pool(_command_pool_handles_begin + _current_buffer);
    pool.reset();
    _current_primary_buffer = pool.obtain_command_buffer(pool.obtain_command_arena());

    device->transfer_queue_submit_immediate(current_buffer, _transfer_blocking_fences[current_buffer_handle],  device->obtain_semaphore(_transfer_submission_semaphores[current_buffer_handle]));
    _buffer_copies_top.reset();
    
    RX_END_RESULT(_current_buffer);
  }

  const GPUDevice::SemaphoreHandle GPUDevice::Loader::get_corresponding_semaphore_for_staging_ptr(const void* const ptr) {
    const u8 handle = get_buffer_handle_from_ptr(ptr);
    RX_RETURNF(handle == MAX_u8
      , lofi::index_type_max<SemaphoreHandle>::value
      , "neither staging buffer owns this pointer %p queried for signal semaphore"
      , ptr);
    return _transfer_submission_semaphores[handle];
  }

  const vk::Buffer& GPUDevice::Loader::get_corresponding_transfer_buffer_for_staging_ptr(GPUDevice* device, const void* const ptr) {
    const u8 handle = get_buffer_handle_from_ptr(ptr);
    RX_ASSERT(handle != MAX_u8, "failed to find corresponding transfer buffer");
    return device->obtain_resource_pool(_resource_pool_handle).obtain_buffer(_transfer_source_handles_begin + handle);
  }

  // maps buffer to buffer
  const b8 GPUDevice::Loader::queue_copy(const u32 offset, const u32 size) {
    const auto idx = _buffer_copies_top++;
    RX_CHECK(idx >= GPUDeviceSettings::BufferCopiesPerFrame
      , "too many buffer copies in GPUDevice::_loader at frame");

    VkBufferCopy& buffer_copy = _buffer_copies[idx];
    buffer_copy.dstOffset = offset;
    buffer_copy.srcOffset = offset;
    buffer_copy.size = size;
    RX_END();
  }

  const u32 GPUDevice::Loader::push_to_staging_buffer(const u32 push_bytes) {
    const auto result = _current_buffer_top.add(push_bytes);
    RX_RETURN(queue_copy(result, push_bytes)
      , "failed to create buffer copy in get staging buffer raw pointer"
      , MAX_u32);
    RX_END_RESULT(result);
  }

  void* GPUDevice::Loader::get_staging_buffer_raw_pointer() {
    RX_END_RESULT(_staging_buffer_mapped_pointers[_current_buffer]);
  }

  // returns to offset
  const u32 GPUDevice::Loader::copy_to_staging_buffer(void* src, const u32 num_bytes_to_copy) {
    const u32 result = _current_buffer_top.add(num_bytes_to_copy); 
    RX_RETURNF(queue_copy(result, num_bytes_to_copy)
        , MAX_u32
        , "tried to copy too many buffer copies into staging buffer in GPUDevice::Loader::copy_to_staging_buffer() with current buffer = %u", _current_buffer, result);
    RX_RETURNF(result + num_bytes_to_copy < GPUDeviceSettings::StagingBufferSize
        , MAX_u32
        , "tried to copy too many bytes into staging buffer in GPUDevice::Loader::copy_to_staging_buffer() with current buffer = %u and size before copy = %u", _current_buffer, result);

    MEM_COPY((u8*)_staging_buffer_mapped_pointers[_current_buffer] + result, src, num_bytes_to_copy);
    RX_END_RESULT(result);
  }

  void GPUDevice::Loader::create_commands(GPUDevice* device) {
    _command_pool_handles_begin = device->create_command_pools(vk::CommandType::Transfer, _queue_handle, CommandPoolCount, 1, 1);
  }

  // currently unused
  b8 GPUDevice::Loader::terminate(GPUDevice* device) {

    RX_CHECK(_buffer_copies != nullptr,
        "tried to terminate GPUDevice::_loader but it was never initialized");

    FREE(_buffer_copies);
    RX_END();
  }


  b8 GPUDevice::queue_frame_submit(const frame::ID frame_id, const QueueHandle queue, const vk::CommandBuffer buffer_to_submit) {
    //const auto current_frame_count = _frame_counters[frame_id].add(1);

    VkCommandBufferSubmitInfo command_info{};
    command_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    command_info.commandBuffer = buffer_to_submit.get_command_buffer();

    VkSemaphoreSubmitInfo wait_info{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    // hacky workaround for the fact that the compute queue is static constexpr = 2 and transfer = 1
    //
    wait_info.semaphore = _timelines[queue].get_semaphore();
    wait_info.value = frame_id;

    VkSemaphoreSubmitInfo signal_info{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    signal_info.semaphore = _timelines[queue].get_semaphore();
    signal_info.value = frame_id + 1;
    
    VkSubmitInfo2 submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
    // one primary command buffer per queue currently just 2 queues
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_info;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    submit_info.pWaitSemaphoreInfos = &wait_info;
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.waitSemaphoreInfoCount = 1;

    RX_CHECKF(_context.get_device().get_device_function_table()
      .vkQueueSubmit2(_queue_pool[queue], 1, &submit_info, VK_NULL_HANDLE)
      , "failed to properly submit primary command buffer with queue handle = %u"
      , queue);

    RX_END();
  }





  b8 GPUDevice::terminate() {

    const u32 timeline_count = _timelines.get_size();
    for(u32 i = 0; i < timeline_count; i++) {
      _timelines[i].terminate(&_context);
    }

    const u32 command_pool_count = _command_pools.get_size();
    for(u32 i = 0; i < command_pool_count; i++) {
      _command_pools[i].terminate();
    }

    const u32 descriptor_pool_count = _descriptor_pools.get_size();
    for(u32 i = 0; i < descriptor_pool_count; i++) {
      RX_CHECK(_descriptor_pools[i].terminate()
        , "failed to terminate DescriptorPool in GPUDevice::terminate()");
    }

    const u32 pipeline_pool_count = _pipeline_pools.get_size();
    for(u32 i = 0; i < pipeline_pool_count; i++) {
      RX_CHECK(_pipeline_pools[i].terminate(&_context)
        , "failed to terminate PipelinePool in GPUDevice::terminate()");
    }

    const u32 resource_pool_count = _resource_pools.get_size();
    for(u32 i = 0; i < resource_pool_count; i++) {
      RX_CHECK(_resource_pools[i].terminate()
        , "failed to terminate ResourcePool in GPUDevice::terminate()");
    }
    RX_CHECK(_context.terminate()
        , "failed to terminate vk::Context in GPUDevice::terminate()");

    FREE(_pipeline_pools.get_buffer());
    return true;
  }

  const vk::PipelinePool& GPUDevice::obtain_pipeline_pool(const PipelinePoolHandle handle) const {
    return _pipeline_pools[handle];
  }

  const GPUDevice::DescriptorPoolHandle GPUDevice::create_descriptor_pool(const u32 ubo_count, const u32 storage_count, const u32 texture_count, const u32 image_count) {
    const DescriptorPoolHandle result = _descriptor_pools.get_size();
    vk::DescriptorPool& pool = *(_descriptor_pools.push(1));
    RX_RETURN(pool.init(&_context, ubo_count, storage_count, texture_count, image_count)
      , "failed to create descriptor pool"
      , lofi::index_type_max<DescriptorPoolHandle>::value
      );
    RX_END_RESULT(result);
  }

  vk::DescriptorPool& GPUDevice::obtain_descriptor_pool(const DescriptorPoolHandle handle) {
    return _descriptor_pools[handle];
  }

  const VkQueue GPUDevice::obtain_queue(const QueueHandle handle) const {
    return _queue_pool[handle];
  }

  const vk::Extent<2> GPUDevice::get_current_extent() const {
    return vk::Extent<2>{{_current_width, _current_height}};
  }

  b8 Renderer::init(GPUDevice* device, const GPUDevice::QueueHandle queue_handle) {
    _device = device;
    _queue_handle = queue_handle;
    _framebuffers.move_ptr(ALLOCATE(sizeof(vk::Framebuffer) * vk::s_max_framebuffers));
    _framebuffers.clear();

    RX_END();
  }


//  b8 GPUDeviceBuilder::init() {
//    _resources.move_ptr(ALLOCATE(sizeof(ResourceData) * gpu::s_max_resources));
//    _descriptors.move_ptr(ALLOCATE(sizeof(DescriptorData) * vk::s_max_descriptor_buffers));
//
//    RX_CHECK
//      ( _pipeline_builder.init()
//      , "failed to initialize pipeline builder in GPUDeviceBuilder"
//      );
//
//    return true;
//  }
//
//  vk::PipelinePool::RenderPassHandle GPUDeviceBuilder::add_render_pass(u32 attachment_count, vk::AttachmentInfo* colour_infos, vk::AttachmentInfo depth_info) {
//    const auto result = _pipeline_builder.add_render_pass(attachment_count, colour_infos, depth_info);
//    return result;
//  }
//
//  vk::PipelinePool::PipelineHandle GPUDeviceBuilder::add_pipeline(vk::DispatchType dispatch_type, const vk::PipelineInfo pipeline_info) {
//    const auto result = _pipeline_builder.add_pipeline(dispatch_type, pipeline_info);
//    return result;
//  }
//
//  // have to create and destroy later which is where you get the handle, this call just for preallocation
//  void GPUDeviceBuilder::add_resource(const gpu::ResourceType type, const gpu::ResourceInfo info) {
//    ResourceData& resource = *(_resources.push(1));
//    resource.info = info;
//    resource.type = type;
//  }
//
//  vk::DescriptorPool::DescriptorBufferHandle GPUDeviceBuilder::add_descriptor_type(const vk::DescriptorBufferType type, const vk::DescriptorBufferLevel level, const u32 count) {
//    const auto result = _descriptors.get_size();
//    DescriptorData& data = *(_descriptors.push(1));
//    data.type = type;
//    data.level = level;
//    data.count = count;
//    return result;
//  }
//
//  GPUDeviceBuilder& GPUDeviceBuilder::set_initial_screen_size(const vk::Extent<2> size) {
//    _screen_size = size.value;
//    return *this;
//  }
//
//  GPUDeviceBuilder& GPUDeviceBuilder::set_name(const char* name) {
//    _name = name;
//    return *this;
//  }
//
//  b8 GPUDeviceBuilder::build(GPUDevice* gpu_device) {
//    gpu_device->_window.init(_screen_size.width, _screen_size.height, _name);
//    gpu_device->_context.init(gpu_device->_window.get_window());
//
//    gpu::ResourceInfo info{};
//    info.buffer.size = GPUDeviceSettings::StagingBufferSize;
//    info.type = gpu::ResourceType::StagingBuffer;
//    resource_builder.add_resource(info);
//
//    RX_CHECK
//      ( resource_builder.init(&gpu_device->_context)
//      , "failed to initialize resource builder in GPUDeviceBuilder"
//      );
//
//    u32 num_iterations = _resources.get_size();
//    for(u32 i = 0; i < num_iterations; i++) {
//      resource_builder.add_resource(_resources[i].type, _resources[i].info);
//    }
//      ( resource_builder.build(&gpu_device->_resource_pool)
//      , "failed to build ResourcePool in GPUDeviceBuilder"
//      );
//
//    resource_builder.terminate();
//
////
////    for(u32 i = 0; i < RoxiNumFrames; i++) {
////      gpu_device->_staging_buffer_handle[i] = gpu_device->create_buffer(gpu::BufferType::StagingBuffer, GPUDeviceSettings::StagingBufferSize);
////      gpu_device->_transfer_command_buffer[i] = gpu_device->create_command_type(vk::CommandType::MainTransfer, vk::CommandBufferLevelType::Primary, RoxiNumFrames);
////    }
////
//    return true;
//  }
//
//  b8 GPUDeviceBuilder::terminate() {
//    _pipeline_builder.terminate();
//    FREE(_descriptors.get_buffer());
//    FREE(_resources.get_buffer());
//    return true;
//  }

}		// -----  end of namespace roxi  ----- 
