// =====================================================================================
//
//       Filename:  rx_renderer.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-11 9:51:58 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_renderer.hpp"
#include "rx_resource_manager.hpp"



namespace roxi {
 
//  const u32 ClusteredForwardRenderer::VerticesSize = sizeof(Vertex) * resource::vertex_count();
//  const u32 ClusteredForwardRenderer::IndicesSize = sizeof(u32) * resource::vertex_index_count();
//  const u32 ClusteredForwardRenderer::MeshesSize = sizeof(MeshData) * resource::mesh_count();
//
//  b8 ClusteredForwardRenderer::init() {
//
//    create_commands();
//    create_descriptors(RoxiNumFrames * 2, RoxiNumFrames * 5 + 3, resource::texture_count(), 0);
//
//
//    static const u32 buffer_count = RoxiNumFrames * 8 + 3;
//    static const u32 image_count = resource::texture_count() + (RoxiNumFrames * 2);
//
//    gpu::ResourceInfo* buffer = (gpu::ResourceInfo*)ALLOCATE((buffer_count + image_count) * sizeof(gpu::ResourceInfo));
//
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      buffer[i].type = gpu::ResourceType::HostUniformBuffer;
//      buffer[i].buffer.size = CameraSize;
//
//      buffer[RoxiNumFrames + i].type = gpu::ResourceType::DeviceUniformBuffer;
//      buffer[RoxiNumFrames + i].buffer.size = DrawParamsSize;
//
//      buffer[(2 * RoxiNumFrames) + i].type = gpu::ResourceType::DeviceStorageBuffer;
//      buffer[(2 * RoxiNumFrames) + i].buffer.size = AABBsSize;
//
//      buffer[(3 * RoxiNumFrames) + i].type = gpu::ResourceType::DeviceStorageBuffer;
//      buffer[(3 * RoxiNumFrames) + i].buffer.size = LightCellsSize;
//
//      buffer[(4 * RoxiNumFrames) + i].type = gpu::ResourceType::DeviceStorageBuffer;
//      buffer[(4 * RoxiNumFrames) + i].buffer.size = LightIndicesSize;
//
//      buffer[(5 * RoxiNumFrames) + i].type = gpu::ResourceType::DeviceStorageBuffer;
//      buffer[(5 * RoxiNumFrames) + i].buffer.size = LightDataSize;
// 
//      buffer[(6 * RoxiNumFrames) + i].type = gpu::ResourceType::DeviceStorageBuffer;
//      buffer[(6 * RoxiNumFrames) + i].buffer.size = InstanceDataSize;   
//
//      buffer[(7 * RoxiNumFrames) + i].type = gpu::ResourceType::DeviceStorageBuffer;
//      buffer[(7 * RoxiNumFrames) + i].buffer.size = IndirectCommandsSize;
//    }
//
//    buffer[(8 * RoxiNumFrames)].type = gpu::ResourceType::DeviceStorageBuffer;
//    buffer[(8 * RoxiNumFrames)].buffer.size = MeshesSize;
//
//    buffer[(8 * RoxiNumFrames) + 1].type = gpu::ResourceType::DeviceStorageBuffer;
//    buffer[(8 * RoxiNumFrames) + 1].buffer.size = VerticesSize;
//
//    buffer[(8 * RoxiNumFrames) + 2].type = gpu::ResourceType::DeviceStorageBuffer;
//    buffer[(8 * RoxiNumFrames) + 2].buffer.size = IndicesSize;
//
//    create_resources(buffer_count, image_count, buffer, buffer + buffer_count);
//
//    vk::PipelinePoolBuilder pipeline_builder{};
//    pipeline_builder.init();
//
//    vk::PipelineInfo pipeline_info{};
//
//    pipeline_info.type = vk::DispatchType::ComputeIndirect;
//    pipeline_info.compute.shader_name = "skinning_indirect.comp";
//    _skinning_pipeline_handle = pipeline_builder.add_pipeline(pipeline_info);
//
//    pipeline_info.type = vk::DispatchType::ComputeIndirect;
//    pipeline_info.compute.shader_name = "frustum_.comp";
//    pipeline_builder.add_pipeline(pipeline_info);
//
//    ubo::DrawParams draw_params_begin;
//  
//    const u32 ubo_count = 2 * RoxiNumFrames;
//    const u32 storage_buffer_count = 6 * RoxiNumFrames + 3;
//    const u32 texture_count = resource::texture_count();
//    const u32 storage_image_count = 0;
//
//    create_descriptors(ubo_count, storage_buffer_count, texture_count, storage_image_count);
//
//      _draw_param_ubo_handles_begin = get_resource_pool().create_buffers(gpu::BufferType::DeviceUniformBuffer, sizeof(ubo::DrawParams), RoxiNumFrames);
//      _draw_param_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Uniform>(1);
//
//      _camera_ubo_handles_begin = get_resource_pool().create_buffer(gpu::BufferType::HostUniformBuffer, CameraSize);
//      _camera_ubo_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Uniform>(1);
//
//      draw_params_begin.camera_buffer_id = _camera_ubo_descriptor_offsets_begin.get_buffer_id();
//      // get index by offset to make the implementation more maintainable ?? 
//      draw_params_begin.camera_buffer_id = _camera_ubo_descriptor_offsets_begin.get_buffer_id();
//
//      _aabb_buffer_handles_begin = get_resource_pool().create_buffers(gpu::BufferType::DeviceStorageBuffer, AABBsSize);
//      _aabb_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(RoxiNumFrames);
//
//      draw_params_begin.aabb_buffer_id = _aabb_descriptor_offsets_begin.get_buffer_id();
//
//      _light_cell_buffer_handles_begin = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, LightCellsSize);
//      _light_cell_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(RoxiNumFrames);
//
//      draw_params_begin.light_cell_buffer_id = _light_cell_descriptor_offsets_begin.get_buffer_id();
//
//      _light_indices_buffer_handles_begin = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, LightIndicesSize);
//      _light_indices_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(RoxiNumFrames);
//
//      draw_params_begin.light_index_buffer_id = _light_indices_descriptor_offsets_begin.get_buffer_id();
//
//      _light_data_buffer_handles_begin = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, LightDataSize);
//      _light_data_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(RoxiNumFrames);
//
//      draw_params_begin.light_buffer_id = _light_data_descriptor_offsets_begin.get_buffer_id();
//      
//      _indirect_draw_buffer_handles_begin = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, IndirectCommandsSize);
//
//      _instance_buffer_handles_begin = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, InstanceDataSize);
//      _instance_buffer_descriptor_offsets_begin = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(RoxiNumFrames);
//
//      draw_params_begin.instance_buffer_id = _instance_buffer_descriptor_offsets_begin.get_buffer_id();
//
//      _depth_target_handles_begin = get_resource_pool().create_images(gpu::ImageType::DepthTarget, get_current_extent().value, RoxiNumFrames);
//      _render_target_handles_begin = get_resource_pool().create_images(gpu::ImageType::RenderTarget, get_current_extent().value, RoxiNumFrames);
//
//    _mesh_buffer_handle = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, MeshesSize);
//    _mesh_buffer_descriptor_offset = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(1);
//
//    _vertex_buffer_handle = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, VerticesSize);
//    _vertex_buffer_descriptor_offset = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(1);
//
//    _index_buffer_handle = get_resource_pool().create_buffer(gpu::BufferType::DeviceStorageBuffer, IndicesSize);
//    _index_buffer_descriptor_offset = get_descriptor_pool().allocate<vk::DescriptorBufferType::Storage>(1);
//
//    draw_params_begin.mesh_buffer_id = _mesh_buffer_descriptor_offset.get_buffer_id();
//    draw_params_begin.vertex_buffer_id = _vertex_buffer_descriptor_offset.get_buffer_id();
//    draw_params_begin.index_buffer_id = _index_buffer_descriptor_offset.get_buffer_id();
//
//
//    ImageHandle* const textures_begin = _texture_handles.push(texture_count);
//    vk::DescriptorAllocation* const texture_descriptor_handles_begin = _texture_handle_descriptor_offsets.push(texture_count);
//
//    for(u32 i = 0; i < texture_count; i++) {
//      Extent extent = resource::texture_size(i);
//      const u32 tex_depth = extent.depth;
//      const u32 tex_width = extent.width;
//      const u32 tex_height = extent.height;
//      VkFormat format = resource::texture_format(i);
//      textures_begin[i] = get_resource_pool().create_image(gpu::ImageType::Texture, VkExtent2D{tex_width, tex_height}, format);
//      texture_descriptor_handles_begin[i] = get_descriptor_pool().allocate<vk::DescriptorBufferType::CombinedImageSampler>(1);
//    }
//
//    // write static descriptors
//    BufferHandle _staging_buffer_handle = get_resource_pool().create_buffer(gpu::BufferType::StagingBuffer, StagingBufferSize);
//    const vk::Buffer& staging_buffer = get_resource_pool().obtain_buffer(_staging_buffer_handle);
//
//    void* buffer_ptr = staging_buffer.map();
//
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//
//      RX_CHECKF(get_descriptor(_draw_param_ubo_handles_begin + i, _draw_param_descriptor_offsets_begin + i, buffer_ptr), "failed to get draw param descriptor for frame %u", i);
//
//      RX_CHECKF(get_descriptor(_camera_ubo_handles_begin + i, _camera_ubo_descriptor_offsets_begin + i, buffer_ptr), "failed to get camera ubo descriptor for frame %u", i);
//
//      RX_CHECKF(get_descriptor(_aabb_buffer_handles_begin + i, _aabb_descriptor_offsets_begin + i, buffer_ptr), "failed to get aabb buffer descriptor for frame %u", i);
//
//      RX_CHECKF(get_descriptor(_light_cell_buffer_handles_begin + i, _light_cell_descriptor_offsets_begin + i, buffer_ptr), "failed to get light cell buffer descriptor for frame %u", i);
//
//      RX_CHECKF(get_descriptor(_light_indices_buffer_handles_begin + i, _light_indices_descriptor_offsets_begin + i, buffer_ptr), "failed to get light indices buffer descriptor for frame %u", i);
//
//      RX_CHECKF(get_descriptor(_light_data_buffer_handles_begin + i, _light_data_descriptor_offsets_begin + i, buffer_ptr), "failed to get light data buffer descriptor for frame %u", i);
//
//      RX_CHECKF(get_descriptor(_instance_buffer_handles_begin + i, _instance_buffer_descriptor_offsets_begin + i, buffer_ptr), "failed to get light cell buffer descriptor for frame %u", i);
//
//    }
//
//    return true;
//  }
//
//  const u32 ClusteredForwardRenderer::get_pool_handle(const frame::ID frame_id) {
//    return frame_id % RenderFrameCount;
//  }
//
//  b8 ClusteredForwardRenderer::record_frame(const frame::ID frame_id) {
//
//    vk::CommandBuffer command_buffer = get_command_pool().obtain_command_arena(_command_arenas_begin + GET_HOST_THREAD_ID(ThreadPool) + (frame_id * RoxiNumThreads)).obtain_primary_command_buffer();
//    vk::Extent<2> render_size = get_current_extent();
//    command_buffer.begin();
//    command_buffer.begin_render_pass(get_pipeline_pool().obtain_render_pass(_depth_pre_pass_handle), render_size.value.width, render_size.value.height, &_framebuffers_begin + frame_id);
//
//    command_buffer.bind_descriptor_pool(get_descriptor_pool());
//    command_buffer.bind_pipeline(get_pipeline_pool().obtain_pipeline(_skinning_pipeline_handle));
//    // const u32 loop_count = skinned_mesh_count / SkinningDispatchCount;
//    // for(u32 i = 0; i < loop_count; i++) {
//    command_buffer.record_dispatch(SkinningDispatchCount, 1, 1);
//    //}
//  }
//
//  b8 ClusteredForwardRenderer::terminate() {
//    FREE(_texture_handles.get_buffer());
//    FREE(_texture_handle_descriptor_offsets.get_buffer());
//    return true;
//  }
//
//  //b8 ClusteredForwardRenderer::record_draws(u64 frame_id, u64 job_id, u64 start, u64 end) {
//  //  //vk::CommandBuffer& buffer = _command_buffers[frame_id][job_id];
//  //  //buffer.begin(_context);
//  //  //buffer.bind_pipeline(_context, &_vertex_creation_pipeline);
//  //  //buffer.bind_descriptors<vk::PipelineType::Compute>(
//  //  for(size_t i = start; i < end; i++) {
//  //    //buffer.record_dispatch(_context, 256, 1, 1);
//  //  }
//  //  //_command_buffers[frame_id][job_id].end(_context);
//  //  return true;
//  //}
//
//  b8 ClusteredForwardRenderer::init_resources(const u32 width, const u32 height) {
//    vk::ResourcePoolBuilder resource_builder{};
//    ubo::DrawParams draw_params[RoxiNumFrames];
//
//    u32 draw_param_id = MAX_u32;
//
//    resource_builder.init(_context);
//    static constexpr u32 CameraSize = sizeof(ubo::Camera);
//    static constexpr u32 DrawParamsSize = sizeof(ubo::DrawParams);
//    static constexpr u32 LightCellsSize = sizeof(LightCell) * ClusterCount;
//    static constexpr u32 LightIndicesSize = sizeof(u32) * MaxLights;
//    static constexpr u32 LightDataSize = sizeof(Light) * MaxLights;
//    static constexpr u32 IndirectCommandsSize = sizeof(VkDrawIndexedIndirectCommand) * MaxDrawIndirectCount;
//    static constexpr u32 InstanceDataSize = sizeof(InstanceData) * s_max_instances;
//    static constexpr u32 AABBsSize = sizeof(AABB) * ClusterCount;
//    static constexpr u32 VerticesSize = sizeof(Vertex) * resource::vertex_count();
//    static constexpr u32 IndicesSize = sizeof(u32) * resource::vertex_index_count();
//    static constexpr u32 MeshesSize = sizeof(MeshData) * resource::mesh_count();
//    ResourceInfo resource_info{};
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      resource_info.buffer.size = CameraSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::HostUniformBuffer>(resource_info)
//      , "failed to allocate camera data");
//
//      resource_info.buffer.size = DrawParamsSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceUniformBuffer>(resource_info)
//      , "failed to allocate draw params");
//
//      resource_info.buffer.size = LightCellsSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//      , "failed to allocate light cells");
//
//      resource_info.buffer.size = LightIndicesSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//      , "failed to allocate light indices");
//
//      resource_info.buffer.size = LightDataSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//      , "failed to allocate light data");
//
//      resource_info.buffer.size = IndirectCommandsSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::IndirectCommand>(resource_info)
//      , "failed to allocate indirect commands");
//
//      resource_info.buffer.size = InstanceDataSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//      , "failed to allocate instance data");
//
//      resource_info.buffer.size = AABBsSize;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//      , "failed to allocate aabbs");
//
//      resource_info.image.depth = 1;
//      resource_info.image.height = height;
//      resource_info.image.width = width;
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::RenderTarget>(resource_info)
//      , "failed to allocate render targets");
//
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::DepthTarget>(resource_info)
//      , "failed to allocate depth targets");
//    }
//
//    resource_info.buffer.size = VerticesSize;
//    RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//    , "failed to allocate vertex buffer");
//  
//    resource_info.buffer.size = IndicesSize ;
//    RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//    , "failed to allocate index buffer");
//   
//    resource_info.buffer.size = MeshesSize;
//    RX_CHECK(resource_builder.add_resource<vk::ResourceType::DeviceStorageBuffer>(resource_info)
//    , "failed to allocate mesh buffer");
//
//    resource_info.buffer.size = StagingBufferSize;
//    RX_CHECK(resource_builder.add_resource<vk::ResourceType::StagingBuffer>(resource_info)
//    , "failed to allocate staging buffer");
// 
//    const u32 texture_count = resource::texture_count();
//    _texture_handles.move_ptr(ALLOCATE(sizeof(ResourceHandle) * texture_count));
//    for(u32 i = 0; i < texture_count; i++) {
//      Extent extent = resource::texture_size(i);
//      resource_info.image.depth = extent.depth;
//      resource_info.image.width = extent.width;
//      resource_info.image.height = extent.height;
//      resource_info.image.format = resource::texture_format(i);
//      RX_CHECK(resource_builder.add_resource<vk::ResourceType::Texture>(resource_info)
//      , "failed to allocate texture");
//    }
//    RX_CHECK
//    ( resource_builder.build(&_resource_pool)
//    , "failed to allocate resource pool");
//
//    // always allocate draw params first and flat to ensure that frame::ID plus draw_param spec constant = 0
//    for(u64 i = 0; i < RoxiNumFrames; i++) {
//      const u32 descriptor_buffer_offset_alignment = _descriptor_pool.get_offset_alignment();
//
//      _draw_param_ubo_handles[i] = _resource_pool.create_resource<vk::ResourceType::DeviceUniformBuffer>(DrawParamsSize);
//
//      _draw_param_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_uniform_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//    }
//
//    for(u64 i = 0; i < RoxiNumFrames; i++) {
//      _camera_ubo_handles[i] = _resource_pool.create_resource<vk::ResourceType::HostUniformBuffer>(CameraSize);
//      _camera_ubo_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_uniform_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//
//      // get index by offset to make the implementation more maintainable ?? 
//      draw_params[i].camera_buffer_id = _camera_ubo_descriptor_offsets[i].offset / _camera_ubo_descriptor_offsets[i].size;
//
//      _aabb_buffer_handles[i] = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(AABBsSize);
//      _aabb_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//      draw_params[i].aabb_buffer_id = _aabb_descriptor_offsets[i].offset / _aabb_descriptor_offsets[i].size;
//
//      _light_cell_buffer_handles[i] = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(LightCellsSize);
//      _light_cell_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//      draw_params[i].light_index_buffer_id = _light_cell_descriptor_offsets[i].offset / _light_cell_descriptor_offsets[i].size;
//
//      _light_indices_buffer_handles[i] = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(LightIndicesSize);
//      _light_indices_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//      draw_params[i].light_index_buffer_id = _light_indices_descriptor_offsets[i].offset / _light_indices_descriptor_offsets[i].size;
//
//      _light_data_buffer_handles[i] = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(LightDataSize);
//      _light_data_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//      draw_params[i].light_buffer_id = _light_data_descriptor_offsets[i].offset / _light_data_descriptor_offsets[i].size;
//      
//      _indirect_draw_buffer_handles[i] = _resource_pool.create_resource<vk::ResourceType::IndirectCommand>(IndirectCommandsSize);
//
//      _instance_buffer_handles[i] = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(InstanceDataSize);
//      _instance_buffer_descriptor_offsets[i] = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//      draw_params[i].instance_buffer_id = _instance_buffer_descriptor_offsets[i].offset / _instance_buffer_descriptor_offsets[i].size;
//
//      _depth_target_handles[i] = _resource_pool.create_resource<vk::ResourceType::DepthTarget>(VkExtent2D{width, height});
//      _render_target_handles[i] = _resource_pool.create_resource<vk::ResourceType::RenderTarget>(VkExtent2D{width, height});
//    }
//
//    _mesh_buffer_handle = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(MeshesSize);
//    _mesh_buffer_descriptor_offset = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      draw_params[i].mesh_buffer_id = _mesh_buffer_descriptor_offset.offset / _mesh_buffer_descriptor_offset.size;
//    }
//
//    _vertex_buffer_handle = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(VerticesSize);
//    _vertex_buffer_descriptor_offset = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      draw_params[i].vertex_buffer_id = _vertex_buffer_descriptor_offset.offset / _vertex_buffer_descriptor_offset.size;
//    }
//
//    _index_buffer_handle = _resource_pool.create_resource<vk::ResourceType::DeviceStorageBuffer>(IndicesSize);
//    _index_buffer_descriptor_offset = _descriptor_pool.obtain_arena(_storage_descriptor_buffer_handle).allocate(_context, 1, _descriptor_pool.get_offset_alignment());
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      draw_params[i].index_buffer_id = _index_buffer_descriptor_offset.offset / _index_buffer_descriptor_offset.size;
//    }
//
//    _staging_buffer_handle = _resource_pool.create_resource<vk::ResourceType::StagingBuffer>(StagingBufferSize);
//
//    ResourceHandle* const textures_begin = _texture_handles.push(texture_count);
//    for(u32 i = 0; i < texture_count; i++) {
//      Extent extent = resource::texture_size(i);
//      const u32 tex_depth = extent.depth;
//      const u32 tex_width = extent.width;
//      const u32 tex_height = extent.height;
//      VkFormat format = resource::texture_format(i);
//      textures_begin[i] = 
//        _resource_pool.create_resource<vk::ResourceType::Texture>(VkExtent2D{tex_width, tex_height}, format);
//    }
//
//    // write static descriptors
//    vk::Buffer& staging_buffer = _resource_pool.obtain_resource<vk::ResourceType::StagingBuffer>(_staging_buffer_handle);
//
//    void* buffer_ptr = staging_buffer.map();
//
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      vk::Buffer& camera_buffer = _resource_pool.obtain_resource<vk::ResourceType::HostUniformBuffer>(_camera_ubo_handles[i]);
//      vk::DescriptorAllocation camera_descriptor_allocation = _camera_ubo_descriptor_offsets[i];
//        VkDescriptorAddressInfoEXT address_info
//        { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT
//        , nullptr
//        , camera_buffer.get_device_address()
//        , camera_buffer.get_size()
//        , VK_FORMAT_UNDEFINED
//        };
//
//        VkDescriptorGetInfoEXT get_info{};
//        get_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
//        get_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        get_info.data.pUniformBuffer = &address_info;
//
//        _context->get_device().get_device_function_table()
//          .vkGetDescriptorEXT
//            ( _context->get_device().get_device()
//            , &get_info
//            , camera_descriptor_allocation.size
//            , (void*)((u8*)buffer_ptr + camera_descriptor_allocation.offset)
//            );
//
//        vk::Buffer& _draw_params = _resource_pool.obtain_resource<vk::ResourceType::DeviceUniformBuffer>(_draw_param_ubo_handles[i]);
//      vk::DescriptorAllocation draw_param_descriptor_allocation = _camera_ubo_descriptor_offsets[i];
//        VkDescriptorAddressInfoEXT address_info
//        { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT
//        , nullptr
//        , camera_buffer.get_device_address()
//        , camera_buffer.get_size()
//        , VK_FORMAT_UNDEFINED
//        };
//
//        VkDescriptorGetInfoEXT get_info{};
//        get_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
//        get_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        get_info.data.pUniformBuffer = &address_info;
//
//        _context->get_device().get_device_function_table()
//          .vkGetDescriptorEXT
//            ( _context->get_device().get_device()
//            , &get_info
//            , camera_descriptor_allocation.size
//            , (void*)((u8*)buffer_ptr + camera_descriptor_allocation.offset)
//            );
//
//
//
//
//
//    }
//
//
//
//    return true;
//  }
//
//  b8 ClusteredForwardRenderer::init_descriptor_buffers() {
//    vk::DescriptorPoolBuilder descriptor_builder;
//    descriptor_builder.init(_context);
//    _uniform_descriptor_buffer_handle = descriptor_builder.add_descriptor_buffer(vk::DescriptorBufferType::Uniform, vk::DescriptorBufferLevel::Device, vk::s_max_uniform_buffers);
//    _storage_descriptor_buffer_handle = descriptor_builder.add_descriptor_buffer(vk::DescriptorBufferType::Storage, vk::DescriptorBufferLevel::Device, vk::s_max_storage_buffers);
//    _image_sampler_descriptor_buffer_handle = descriptor_builder.add_descriptor_buffer(vk::DescriptorBufferType::CombinedImageSampler, vk::DescriptorBufferLevel::Device, vk::s_max_textures);
//    _storage_image_descriptor_buffer_handle = descriptor_builder.add_descriptor_buffer(vk::DescriptorBufferType::StorageImage, vk::DescriptorBufferLevel::Device, vk::s_max_storage_images);
//
//    RX_CHECK(descriptor_builder.build(&_descriptor_pool)
//    , "failed to build descriptor pool");
//
//    return true;
//  }
//
//  b8 ClusteredForwardRenderer::init_commands() {
//
//    for(u32 i = 0; i < RoxiNumFrames; i++) {
//      RX_CHECK
//      ( _command_pool[i].init(_context)
//      , "failed to initialize CommandBufferPool"
//      );
//    }
//
//    return true;
//  }
//
//  b8 ClusteredForwardRenderer::init_pipelines() {
//    const u32 draw_param_id = 0;
//    vk::PipelinePoolBuilder pool_builder{};
//    pool_builder.init(_context);
//    pool_builder.set_descriptor_pool(&_descriptor_pool);
//    pool_builder.add_specialization_constant(0, &draw_param_id);
//
//    static constexpr vk::RenderPassType depth_pre_pass_render_pass_type
//      = vk::RenderPassType::DepthOnly
//      | vk::RenderPassType::DepthLoad
//      ;
//
//    static constexpr vk::RenderPassType render_pass_type
//      = vk::RenderPassType::ColourClear
//      // maybe? | vk::RenderPassType::DepthStore
//      ;
//
//    _render_pass_handle = pool_builder.add_render_pass<render_pass_type>();
//
//    vk::PipelineInfo info{};
//    info.compute.shader_name = "mesh_skinning.comp";
//    _skinning_pipeline_handle = pool_builder.add_pipeline<vk::DispatchType::Compute>(info);
//
//    info.compute.shader_name = "frustum_draw_indirect.comp";
//    _frustum_cull_pipeline_handle = pool_builder.add_pipeline<vk::DispatchType::Compute>(info);
//
//    info.compute.shader_name = "aabb_render.vert";
//    _aabb_render_pipeline_handle = pool_builder.add_pipeline<vk::DispatchType::Draw>(info);
//
//    info.compute.shader_name = "light_culling.comp";
//    _light_culling_pipeline_handle = pool_builder.add_pipeline<vk::DispatchType::Compute>(info);
//
//    info.graphics.vertex_shader_name = "light_orb.vert";
//    info.graphics.fragment_shader_name = "light_orb.frag";
//    info.graphics.render_pass_id = _render_pass_handle;
//    _light_render_pipeline_handle = pool_builder.add_pipeline<vk::DispatchType::Draw>(info);
//
//    info.graphics.vertex_shader_name = "scene.vert";
//    info.graphics.fragment_shader_name = "scene.frag";
//    info.graphics.render_pass_id = _render_pass_handle;
//    _clustered_forward_pipeline_handle = pool_builder.add_pipeline<vk::DispatchType::DrawIndexedIndirect>(info);
//
//    RX_CHECK
//    ( pool_builder.build(&_pipelines)
//    , "failed to build pipeline pool"
//    );
//
//    pool_builder.terminate();
//
//    return true;
//  }
//
//
}		// -----  end of namespace roxi  ----- 
