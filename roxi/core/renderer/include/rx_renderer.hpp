// =====================================================================================
//
//       Filename:  rx_renderer.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-01-10 2:46:49 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "error.h"
#include "rx_gpu_device.hpp"
#include "rx_vocab.h"
#include "vk_buffer.hpp"
#include "vk_command.hpp"
#include "vk_descriptors.hpp"
#include "vk_image.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderpass.hpp"
#include "vk_resource.hpp"
#include "rx_resource_manager.hpp"

namespace roxi {

  class ClusteredForwardRendererBuilder;

  //class ClusteredForwardRenderer : Renderer {
  //private:
  //  friend class ClusteredForwardRendererBuilder;
  //  using ResourceHandle = vk::ResourcePool::ResourceHandle;
  //  using BufferHandle = vk::ResourcePool::BufferHandle;
  //  using ImageHandle = vk::ResourcePool::ImageHandle;
  //  using PipelineHandle = vk::PipelinePool::PipelineHandle;

  //  using CommandArenaHandle = vk::CommandPool::ArenaHandle;
  //  using RenderPassHandle = vk::PipelinePool::RenderPassHandle;

  //  static constexpr u32 RenderFrameCount = 2;

  //  static constexpr u32 CameraSize = sizeof(ubo::Camera);
  //  static constexpr u32 DrawParamsSize = sizeof(ubo::DrawParams);
  //  static constexpr u32 LightCellsSize = sizeof(LightCell) * GPUDeviceSettings::ClusterCount;
  //  static constexpr u32 LightIndicesSize = sizeof(u32) * GPUDeviceSettings::MaxLights;
  //  static constexpr u32 LightDataSize = sizeof(Light) * GPUDeviceSettings::MaxLights;
  //  static constexpr u32 IndirectCommandsSize = sizeof(VkDrawIndexedIndirectCommand) * GPUDeviceSettings::MaxDrawIndirectCount;
  //  static constexpr u32 InstanceDataSize = sizeof(InstanceData) * s_max_instances;
  //  static constexpr u32 AABBsSize = sizeof(AABB) * GPUDeviceSettings::ClusterCount;
  //  static constexpr u32 StagingBufferSize = GPUDeviceSettings::StagingBufferSize;
  //  static const u32 VerticesSize;
  //  static const u32 IndicesSize;
  //  static const u32 MeshesSize;

  //  static constexpr u32 SkinningDispatchCount = 256;

  //  BufferHandle                                            _camera_ubo_handles_begin;
  //  vk::DescriptorAllocation                     _camera_ubo_descriptor_offsets_begin;

  //  BufferHandle                                        _draw_param_ubo_handles_begin;
  //  vk::DescriptorAllocation                                          _draw_param_descriptor_offsets_begin;

  //  BufferHandle                                           _aabb_buffer_handles_begin;
  //  vk::DescriptorAllocation                                                _aabb_descriptor_offsets_begin;

  //  BufferHandle                                   _light_cell_buffer_handles_begin;
  //  vk::DescriptorAllocation                                          _light_cell_descriptor_offsets_begin;

  //  BufferHandle                                _light_indices_buffer_handles_begin;
  //  vk::DescriptorAllocation                                       _light_indices_descriptor_offsets_begin;

  //  BufferHandle                                   _light_data_buffer_handles_begin;
  //  vk::DescriptorAllocation                                          _light_data_descriptor_offsets_begin;
  //  BufferHandle                                     _instance_buffer_handles_begin;
  //  vk::DescriptorAllocation                                     _instance_buffer_descriptor_offsets_begin;

  //  BufferHandle                                _indirect_draw_buffer_handles_begin;
  //  // indirect draws don't need descriptors because they are pointed to directly by commands

  //  BufferHandle                                                         _mesh_buffer_handle;
  //  vk::DescriptorAllocation                                                         _mesh_buffer_descriptor_offset;

  //  BufferHandle                                                       _vertex_buffer_handle;
  //  vk::DescriptorAllocation                                                       _vertex_buffer_descriptor_offset;

  //  BufferHandle                                                        _index_buffer_handle;
  //  vk::DescriptorAllocation                                                        _index_buffer_descriptor_offset;


  //  Array<ImageHandle>                                                     _texture_handles;
  //  Array<vk::DescriptorAllocation>                                              _texture_handle_descriptor_offsets;

  //  ImageHandle                                           _depth_target_handles_begin;
  //  ImageHandle                                          _render_target_handles_begin;

  //  PipelineHandle                                                   _skinning_pipeline_handle;
  //  PipelineHandle                                               _frustum_cull_pipeline_handle;
  //  PipelineHandle                                                _aabb_render_pipeline_handle;
  //  PipelineHandle                                              _light_culling_pipeline_handle;
  //  PipelineHandle                                              _draw_indirect_pipeline_handle;
  //  PipelineHandle                                               _light_render_pipeline_handle;
  //  PipelineHandle                                          _clustered_forward_pipeline_handle;

  //  RenderPassHandle                                                    _depth_pre_pass_handle;
  //  RenderPassHandle                                                       _render_pass_handle;

  //  vk::Framebuffer                                                        _framebuffers_begin;

  //  const u32 get_pool_handle(const frame::ID frame_id);


  //public:

  //  b8 init();

  //  b8 record_frame(const frame::ID frame_id);

  //  b8 resize(const u64 width, const u64 height);

  //  b8 terminate();

  //};

  //class ClusteredForwardRendererBuilder {
  //private:
  //  GPUDeviceBuilder* _gpu_builder = nullptr;


  //public:

  //  b8 init(GPUDeviceBuilder* extern_builder) {
  //    _gpu_builder = extern_builder;

  //    return true;
  //  }


  //  b8 build(ClusteredForwardRenderer* renderer) {


  //    return true;
  //  }


  //  b8 terminate() {

  //    return true;
  //  }

  //};

  class TestRenderer : Renderer {
  private:
    static constexpr u32 RenderFrameCount = 2;

    ubo::TestDrawParams draw_params{};
    vk::ResourcePool::ImageHandle _render_target_handles_begin = 0;
    FramebufferHandle               _framebuffer_handles_begin;
    vk::PipelinePool::PipelineHandle     _draw_pipeline_handle;
    vk::PipelinePool::RenderPassHandle     _render_pass_handle;

    vk::ResourcePool::BufferHandle                               _ubo_handle;
    vk::ResourcePool::BufferHandle                            _params_handle;

    GPUDevice::LoaderHandle                                   _loader_handle;

    GPUDevice::FenceHandle                                       _copy_fence;

    // resets command buffer, must only be called once per frame
    vk::CommandBuffer get_primary_command_buffer(const frame::ID frame_id) {
      vk::CommandPool& pool = get_command_pool(get_pool_handle(frame_id));
      pool.reset();
      return pool.obtain_primary_command_buffer();
    }

    // only will get a secondary command buffer if get_primary_command_buffer has already been
    // called at least once for this frame, ie: must get primary buffer first for this frame

    static const u32 get_pool_handle(const frame::ID frame_id) {
      return frame_id % RenderFrameCount;
    }

  public:
    b8 init(GPUDevice* device, const GPUDevice::QueueHandle handle) {
      RX_TRACE("initializing renderer");
      Renderer::init(device, handle);

      RX_TRACE("creating commands");
      RX_CHECK(create_commands(RenderFrameCount)
        , "failed to create commands in TestRenderer");
      struct VertexUBO {
        alignas(16)
          glm::vec3 vertex_positions[3];
      };

      RX_TRACE("setting vertex ubo data");
      VertexUBO ubo_data{};
      ubo_data.vertex_positions[0] = glm::vec3(-1.f, -1.f, -1.f);
      ubo_data.vertex_positions[1] = glm::vec3(1.f, -1.f, -1.f);
      ubo_data.vertex_positions[2] = glm::vec3(0.f, 1.f, -1.f);

      RX_TRACE("creating rendering frame infos");
      gpu::ResourceInfo image_infos[RenderFrameCount];
      vk::Extent<2> window_size = get_current_extent();
      for(u32 i = 0; i < RenderFrameCount; i++) {
        image_infos[i].type = gpu::ResourceType::RenderTarget;
        image_infos[i].image.width = window_size.value.width;
        image_infos[i].image.height = window_size.value.height;
        image_infos[i].image.depth = 1;
      }

      // [0] = ubo for vertices, [1] = param data
      const u32 ubo_handle = 0;
      const u32 param_handle = 1;
      RX_TRACE("creating buffer infos");
      gpu::ResourceInfo buffer_infos[2];
      buffer_infos[ubo_handle].type = gpu::ResourceType::HostUniformBuffer;
      buffer_infos[ubo_handle].buffer.size = sizeof(VertexUBO);
      buffer_infos[param_handle].type = gpu::ResourceType::HostUniformBuffer;
      buffer_infos[param_handle].buffer.size = sizeof(ubo::TestDrawParams);

      RX_TRACE("creating resources");
      RX_CHECK(create_resources(2, RenderFrameCount, buffer_infos, image_infos)
        , "failed to create commands in TestRenderer");

      const vk::Buffer& ubo = get_resource_pool().obtain_buffer(ubo_handle);

      RX_TRACE("mapping host ubo");
      void* ubo_ptr = ubo.map();
      if(ubo_ptr == nullptr) {
        RX_ERROR("ubo mapping returned nullptr");
      }

      RX_TRACE("copying data to mapped pointer");
      MEM_COPY(ubo_ptr, &ubo_data, sizeof(VertexUBO));

      RX_TRACE("unmapping host ubo");
      ubo.unmap();

      _render_target_handles_begin = 0;
      RX_TRACE("obtaining render target from resource pool to generate colour attachment info");
      vk::Image render_target = get_resource_pool().obtain_image(_render_target_handles_begin);
      vk::RenderPassInfo render_pass_info{};
      vk::AttachmentInfo& colour_attachment = *(render_pass_info.colour_attachments.push(1));
      colour_attachment.format = vk::get_image_format(render_target.get_image_type());
      colour_attachment.layout = render_target.get_image_layout();
      colour_attachment.load_op = RenderPassOperationType::Clear;

      _render_pass_handle = 0;
      _draw_pipeline_handle = 0;

      vk::PipelineInfo info{};
      info.type = vk::DispatchType::Draw;
      info.graphics.render_pass_id = _render_pass_handle;
      info.graphics.vertex_shader_name = "basic_shader.vert";
      info.graphics.fragment_shader_name = "basic_shader.frag";
      info.graphics.extent_x = window_size.value.width;
      info.graphics.extent_y = window_size.value.height;

      RX_TRACE("creating pipelines");
      RX_CHECK(create_pipelines(1, 1, &info, &render_pass_info)
        , "failed to create commands in TestRenderer");
      RX_TRACEF("VkRenderPass in TestRenderer = %llu", PTR2INT(get_pipeline_pool().obtain_render_pass(_render_pass_handle).get_render_pass()));

      vk::FramebufferCreation framebuffer_creations[RenderFrameCount];
      for(u32 i = 0; i < RenderFrameCount; i++) {
        RX_TRACEF("creating framebuffer %u", i);
        framebuffer_creations[i].reset()
          .add_render_texture(_render_target_handles_begin + i)
          .set_scaling(1.f, 1.f, true)
          .set_render_pass(&(get_pipeline_pool().obtain_render_pass(_render_pass_handle)))
          .set_extent(window_size.value.width, window_size.value.height);
      }

      RX_TRACE("creating framebuffers");
      _framebuffer_handles_begin = create_framebuffers(RenderFrameCount, framebuffer_creations);
      RX_TRACE("creating descriptors");
      RX_CHECK(create_descriptors(1, 0, 0, 0)
        , "failed to create commands in TestRenderer");

      RX_TRACE("allocating uniform descriptor");
      vk::DescriptorAllocation ubo_descriptor = get_descriptor_pool().allocate(vk::DescriptorBufferType::Uniform, 1);

      draw_params.vertex_buffer_id = ubo_descriptor.get_buffer_id();

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

    b8 update(const frame::ID frame_id) {
      RX_TRACEF("getting TestRenderer primary rendering command buffer for frame = %llu", frame_id);
      vk::CommandBuffer command_buffer = get_command_pool(frame_id).obtain_primary_command_buffer();
      RX_TRACEF("recording rendering command buffer for frame = %llu", frame_id);
      command_buffer
        .begin()
        .begin_render_pass
          ( get_pipeline_pool()
              .obtain_render_pass(_render_pass_handle)
            , get_current_extent().value.width
            , get_current_extent().value.height
            , obtain_framebuffer(_framebuffer_handles_begin + frame_id)
          )
        .bind_pipeline(get_pipeline_pool().obtain_pipeline(_draw_pipeline_handle))
        .bind_descriptor_pool(get_descriptor_pool())
        .record_draw(3, 1)
        .end_render_pass()
        .end();
      RX_TRACEF("submitting rendering command buffer for frame = %llu", frame_id);
      RX_CHECK(submit(frame_id, command_buffer)
          , "failed to submit command buffer in TestRenderer::update()");
      RX_TRACEF("exiting TestRenderer update for frame = %llu", frame_id);
      return true;
    }

  };

  namespace renderer {

//    DEFINE_JOB(async_load_job) {
//      ClusteredForwardRenderer* renderer = (ClusteredForwardRenderer*)param;
//
//      return true;
//
//    }
   
//    static Job create_async_load_task() {
//      Job result{};
//      return result;
//    }

  }		// -----  end of namespace renderer  ----- 

}		// -----  end of namespace roxi  ----- 
