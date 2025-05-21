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
#include "ubo.hpp"
#include "vk_resource.hpp"
#include <glm/fwd.hpp>

namespace roxi {
  b8 TestRenderer::init(GPUDevice* device, const GPUDevice::QueueHandle handle) {
    RX_TRACE("initializing renderer");
    Renderer::init(device, handle);

    _render_frame_count = get_swapchain_image_count();
    RX_TRACE("creating commands");
    RX_CHECK(create_commands(_render_frame_count)
      , "failed to create commands in TestRenderer");

    RX_TRACE("setting vertex ubo data");

    struct VertexUBO {
      alignas(16) struct {
        alignas(16) glm::vec3 vertex_position;
        alignas(4) u32 _pad0;
      } data[3];
    } ubo_data;

    vk::Extent<2> window_size = get_current_extent();
    // acquire from swapchain on each frame
    //  RX_TRACE("creating rendering frame infos");
    //  gpu::ResourceInfo image_infos[RenderFrameCount];
    //  for(u32 i = 0; i < RenderFrameCount; i++) {
    //    image_infos[i].type = gpu::ResourceType::RenderTarget;
    //    image_infos[i].image.width = window_size.value.width;
    //    image_infos[i].image.height = window_size.value.height;
    //    image_infos[i].image.depth = 1;
    //  }

    // [0] = ubo for vertices, [1] = param data
    const u32 param_handle = 0;
    const u32 ubo_handle = 1;
    //const u32 vertex_handle = 2;
    //const u32 index_handle = 3;
    RX_TRACE("creating buffer infos");
    gpu::ResourceInfo buffer_infos[4];
    buffer_infos[param_handle].type = gpu::ResourceType::DeviceUniformBuffer;
    buffer_infos[param_handle].buffer.size = sizeof(ubo::TestDrawParams);
    buffer_infos[ubo_handle].type = gpu::ResourceType::HostUniformBuffer;
    buffer_infos[ubo_handle].buffer.size = sizeof(VertexUBO);
    //buffer_infos[vertex_handle].type = gpu::ResourceType::DeviceStorageBuffer;
    //buffer_infos[vertex_handle].buffer.size = sizeof(Vertex) * resource::mesh_vertex_count(0);
    //buffer_infos[index_handle].type = gpu::ResourceType::DeviceStorageBuffer;
    //buffer_infos[index_handle].buffer.size = sizeof(u32) * resource::mesh_index_count(0);

    RX_TRACE("creating resources");
    RX_CHECK(create_resources(2, 0, buffer_infos, nullptr)
      , "failed to create commands in TestRenderer");

    const vk::Buffer& param_buffer = get_resource_pool().obtain_buffer(param_handle);
    const vk::Buffer& ubo = get_resource_pool().obtain_buffer(ubo_handle);
    //const vk::Buffer& vertex_buffer = get_resource_pool().obtain_buffer(vertex_handle);
    //const vk::Buffer& index_buffer = get_resource_pool().obtain_buffer(index_handle);

//    ubo::Camera& camera = *((ubo::Camera*)camera_buffer.map());
//    camera.projection = glm::perspective(glm::radians(45.0f), (f32)window_size.value.width / (f32)window_size.value.height, 0.1f, 100.f);
//    camera.view = glm::lookAt
//      ( glm::vec3(5.0f, 0.0f, 8.0f)
//      , glm::vec3(0.f, 0.f, 0.f)
//      , glm::vec3(0.f, 1.f, 0.f)
//      );
//    camera.projection[1][1] *= -1;

    ubo_data.data[0].vertex_position = glm::vec3(0.f, -0.3f, 0.f);
    ubo_data.data[1].vertex_position = glm::vec3(-0.3f, 0.3f, 0.f);
    ubo_data.data[2].vertex_position = glm::vec3(0.3f, 0.3f, 0.f);

    RX_TRACEF("mapping host ubo with pos[0] = {%f, %f, %f}, pos[1] = {%f, %f, %f}, pos[2] = {%f, %f, %f}", ubo_data.data[0].vertex_position.x, ubo_data.data[0].vertex_position.y, ubo_data.data[0].vertex_position.z, ubo_data.data[1].vertex_position.x, ubo_data.data[1].vertex_position.y, ubo_data.data[1].vertex_position.z, ubo_data.data[2].vertex_position.x, ubo_data.data[2].vertex_position.y, ubo_data.data[2].vertex_position.z);
    void* ubo_ptr = ubo.map();
    if(ubo_ptr == nullptr) {
      RX_ERROR("ubo mapping returned nullptr");
    }

    RX_TRACE("copying data to mapped pointer");
    MEM_COPY(ubo_ptr, &ubo_data, sizeof(VertexUBO));

    RX_TRACEF("host ubo mapped with pos[0] = {%f, %f, %f}, pos[1] = {%f, %f, %f}, pos[2] = {%f, %f, %f}", (*((VertexUBO*)ubo_ptr)).data[0].vertex_position.x, (*((VertexUBO*)ubo_ptr)).data[0].vertex_position.y, (*((VertexUBO*)ubo_ptr)).data[0].vertex_position.z, (*((VertexUBO*)ubo_ptr)).data[1].vertex_position.x, (*((VertexUBO*)ubo_ptr)).data[1].vertex_position.y, (*((VertexUBO*)ubo_ptr)).data[1].vertex_position.z, (*((VertexUBO*)ubo_ptr)).data[2].vertex_position.x, (*((VertexUBO*)ubo_ptr)).data[2].vertex_position.y, (*((VertexUBO*)ubo_ptr)).data[2].vertex_position.z);

    RX_TRACE("unmapping host ubo");
    ubo.unmap();

    RX_TRACE("obtaining render target from resource pool to generate colour attachment info");
    vk::RenderPassInfo render_pass_info{};
    vk::AttachmentInfo& colour_attachment = *(render_pass_info.colour_attachments.push(1));
    colour_attachment.format = vk::get_image_format(gpu::ImageType::RenderTarget);
    colour_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colour_attachment.load_op = RenderPassOperationType::Clear;

    _render_pass_handle = 0;
    _draw_pipeline_handle = 0;

    vk::PipelineInfo pipeline_info{};
    pipeline_info.type = vk::DispatchType::Draw;
    pipeline_info.graphics.render_pass_id = _render_pass_handle;
    pipeline_info.graphics.vertex_shader_name = "basic_shader.vert";
    pipeline_info.graphics.fragment_shader_name = "basic_shader.frag";
    pipeline_info.graphics.extent_x = window_size.value.width;
    pipeline_info.graphics.extent_y = window_size.value.height;

    RX_TRACE("creating pipelines");
    RX_CHECK(create_pipelines(1, 1, &pipeline_info, &render_pass_info)
      , "failed to create commands in TestRenderer");
    RX_TRACEF("VkRenderPass in TestRenderer = %llu", PTR2INT(get_pipeline_pool().obtain_render_pass(_render_pass_handle).get_render_pass()));
                                      
    vk::FramebufferCreation framebuffer_creations[RoxiNumFrames];
    RX_TRACEF("swapchain image count = %u", _render_frame_count);
    for(u32 i = 0; i < _render_frame_count; i++) {
      RX_TRACEF("creating framebuffer creation %u", i);
      framebuffer_creations[i].reset()
        .add_render_texture(get_swapchain_image_view(i))
        .set_scaling(1.f, 1.f, true)
        .set_render_pass(&(get_pipeline_pool().obtain_render_pass(_render_pass_handle)))
        .set_extent(window_size.value.width, window_size.value.height);
    }

    RX_TRACE("creating framebuffers");
    _framebuffer_handles_begin = create_framebuffers(_render_frame_count, framebuffer_creations);
    RX_TRACE("creating descriptors");
    RX_CHECK(create_descriptors(1, 0, 0, 0)
      , "failed to create commands in TestRenderer");

    RX_TRACE("allocating uniform descriptor");
    vk::DescriptorAllocation ubo_descriptor = get_descriptor_pool().allocate(vk::DescriptorBufferType::Uniform, 1);
//    vk::DescriptorAllocation vertex_descriptor = get_descriptor_pool().allocate(vk::DescriptorBufferType::Storage, 1);
//    vk::DescriptorAllocation index_descriptor = get_descriptor_pool().allocate(vk::DescriptorBufferType::Storage, 1);

//    draw_params.camera_id = camera_descriptor.get_buffer_id();
//    draw_params.vertex_buffer_id = vertex_descriptor.get_buffer_id();
//    draw_params.index_buffer_id = index_descriptor.get_buffer_id();

    RX_TRACE("allocating transfer buffer");

    _copy_fence = device->create_fence();
    _loader_handle = device->create_loader(handle);

    GPUDevice::Loader loader = device->obtain_loader(_loader_handle);

    GPUDevice::TransferAllocation transfer_allocation = loader.allocate(device, ubo_descriptor.size);

    RX_TRACEF("transfer data: ptr = %llx, offset = %u", PTR2INT(transfer_allocation.mapped_pointer), transfer_allocation.offset);

    void* transfer_ptr = (void*)((u8*)transfer_allocation.get_allocation_at_offset());
    RX_CHECK(transfer_allocation.mapped_pointer != nullptr, "failed to allocate descriptor data for vertex ubo in TestRenderer");

    RX_TRACE("getting descriptor");
    RX_CHECK(get_descriptor_pool().get_descriptor(ubo, gpu::BufferType::HostUniformBuffer, ubo_descriptor,
          (u8*)transfer_ptr)
        , "failed to get descriptor for TestDrawParams in TestRenderer::init()");

    RX_TRACE("getting ubo descriptor buffer");
    const vk::Buffer& ubo_descriptor_buffer = get_descriptor_pool().get_descriptor_buffer(vk::DescriptorBufferType::Uniform);

    RX_TRACE("loader.transfer");
    loader.transfer(device);

    VkBufferCopy copy_info{};
    copy_info.dstOffset = ubo_descriptor.offset;
    copy_info.srcOffset = transfer_allocation.offset;
    copy_info.size = ubo_descriptor.size;

    RX_TRACE("recording copy commands");
    vk::CommandBuffer copy_descriptors_cmd = get_command_pool(0).obtain_primary_command_buffer();
    copy_descriptors_cmd
      .begin()
      .record_copy_buffer(*transfer_allocation.transfer_buffer, get_descriptor_pool().get_descriptor_buffer(vk::DescriptorBufferType::Uniform), 1, &copy_info)
      .end();

    RX_TRACE("submitting copy");
    submit_immediate(copy_descriptors_cmd, loader.get_signal_semaphore_handle(), _copy_fence);

    RX_TRACE("waiting for fence");
    device->wait_for_fence(_copy_fence);

    get_command_pool(0).reset();


    RX_TRACE("exiting TestRenderer::init()");
    RX_END();
  }


  b8 TestRenderer::update(const frame::ID frame_id) {
    RX_TRACEF("getting TestRenderer primary rendering command buffer for frame = %llu", frame_id);
   
    u32 render_image_index;
    while(!acquire_next_swapchain_index(&render_image_index, frame_id)){
      RX_TRACE("image not acquired, sleeping!");
      RX_SLEEP_FOR(1000);
    }

    RX_TRACEF("current render image index = %u", render_image_index);
    vk::CommandPool& command_pool = get_command_pool(frame_id);
    wait_for_render(frame_id);
    command_pool.reset();
    vk::CommandBuffer command_buffer = command_pool.obtain_primary_command_buffer();

    RX_TRACEF("recording rendering command buffer for frame = %llu", frame_id);

//    VkImageMemoryBarrier2 image_conversion{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
//    image_conversion.image = get_swapchain_image(render_image_index);
//    image_conversion.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//    image_conversion.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//    image_conversion.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//    image_conversion.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
//    image_conversion.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//    image_conversion.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
//    image_conversion.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    image_conversion.subresourceRange.baseMipLevel = 0;
//    image_conversion.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
//    image_conversion.subresourceRange.baseArrayLayer = 0;
//    image_conversion.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkViewport viewport{0.f, 0.f, (float)get_current_extent().value.width, (float)get_current_extent().value.height, 0.f, 1.f};
    VkRect2D scissor{{0, 0}, {get_current_extent().value.width, get_current_extent().value.height}};
    command_buffer
      .begin()
      .set_viewport(0, 1, &viewport)
      .set_scissor(0, 1, &scissor)
      .begin_render_pass
        ( get_pipeline_pool()
            .obtain_render_pass(_render_pass_handle)
          , get_current_extent().value.width
          , get_current_extent().value.height
          , obtain_framebuffer(_framebuffer_handles_begin + render_image_index)
        )
      .bind_pipeline(get_pipeline_pool().obtain_pipeline(_draw_pipeline_handle))
      .bind_descriptor_pool(get_descriptor_pool())
      .record_draw(3, 1)
      .end_render_pass()
//      .pipeline_barriers(0, 0, 1, 0, nullptr, &image_conversion)
      .end();

    RX_TRACEF("submitting rendering command buffer for frame = %llu", frame_id);
    RX_CHECKF(submit(frame_id, command_buffer)
      , "failed to submit command buffer for frame_id = %u"
      , frame_id);

    RX_CHECKF(present(frame_id, render_image_index)
        , "failed to present image for frame_id = %u, and render image index = %u"
        , frame_id, render_image_index);

    RX_TRACEF("exiting TestRenderer update for frame = %llu, and render image index = %u", frame_id, render_image_index);
    return true;
  }
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
