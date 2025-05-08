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
#include "rx_gpu_device.hpp"
#include "rx_vocab.h"
#include "vk_buffer.hpp"
#include "vk_command.hpp"
#include "vk_descriptors.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderpass.hpp"
#include "vk_resource.hpp"
#include "rx_resource_manager.hpp"
#include <glm/gtc/constants.hpp>

namespace roxi {

  class ClusteredForwardRendererBuilder;

  class ClusteredForwardRenderer : Renderer {
  private:
    friend class ClusteredForwardRendererBuilder;
    using ResourceHandle = vk::ResourcePool::ResourceHandle;
    using BufferHandle = vk::ResourcePool::BufferHandle;
    using ImageHandle = vk::ResourcePool::ImageHandle;
    using PipelineHandle = vk::PipelinePool::PipelineHandle;

    using CommandArenaHandle = vk::CommandPool;
    using RenderPassHandle = vk::PipelinePool::RenderPassHandle;

    static constexpr u32 CameraSize = sizeof(ubo::Camera);
    static constexpr u32 DrawParamsSize = sizeof(ubo::DrawParams);
    static constexpr u32 LightCellsSize = sizeof(LightCell) * GPUDeviceSettings::ClusterCount;
    static constexpr u32 LightIndicesSize = sizeof(u32) * GPUDeviceSettings::MaxLights;
    static constexpr u32 LightDataSize = sizeof(Light) * GPUDeviceSettings::MaxLights;
    static constexpr u32 IndirectCommandsSize = sizeof(VkDrawIndexedIndirectCommand) * GPUDeviceSettings::MaxDrawIndirectCount;
    static constexpr u32 InstanceDataSize = sizeof(InstanceData) * s_max_instances;
    static constexpr u32 AABBsSize = sizeof(AABB) * GPUDeviceSettings::ClusterCount;
    static constexpr u32 StagingBufferSize = GPUDeviceSettings::StagingBufferSize;
    static const u32 VerticesSize;
    static const u32 IndicesSize;
    static const u32 MeshesSize;

    static constexpr u32 SkinningDispatchCount = 256;

    BufferHandle                                            _camera_ubo_handles_begin;
    vk::DescriptorAllocation                     _camera_ubo_descriptor_offsets_begin;

    BufferHandle                                        _draw_param_ubo_handles_begin;
    vk::DescriptorAllocation                                          _draw_param_descriptor_offsets_begin;

    BufferHandle                                           _aabb_buffer_handles_begin;
    vk::DescriptorAllocation                                                _aabb_descriptor_offsets_begin;

    BufferHandle                                   _light_cell_buffer_handles_begin;
    vk::DescriptorAllocation                                          _light_cell_descriptor_offsets_begin;

    BufferHandle                                _light_indices_buffer_handles_begin;
    vk::DescriptorAllocation                                       _light_indices_descriptor_offsets_begin;

    BufferHandle                                   _light_data_buffer_handles_begin;
    vk::DescriptorAllocation                                          _light_data_descriptor_offsets_begin;
    BufferHandle                                     _instance_buffer_handles_begin;
    vk::DescriptorAllocation                                     _instance_buffer_descriptor_offsets_begin;

    BufferHandle                                _indirect_draw_buffer_handles_begin;
    // indirect draws don't need descriptors because they are pointed to directly by commands

    BufferHandle                                                         _mesh_buffer_handle;
    vk::DescriptorAllocation                                                         _mesh_buffer_descriptor_offset;

    BufferHandle                                                       _vertex_buffer_handle;
    vk::DescriptorAllocation                                                       _vertex_buffer_descriptor_offset;

    BufferHandle                                                        _index_buffer_handle;
    vk::DescriptorAllocation                                                        _index_buffer_descriptor_offset;


    Array<ImageHandle>                                                     _texture_handles;
    Array<vk::DescriptorAllocation>                                              _texture_handle_descriptor_offsets;

    ImageHandle                                           _depth_target_handles_begin;
    ImageHandle                                          _render_target_handles_begin;

    PipelineHandle                                                   _skinning_pipeline_handle;
    PipelineHandle                                               _frustum_cull_pipeline_handle;
    PipelineHandle                                                _aabb_render_pipeline_handle;
    PipelineHandle                                              _light_culling_pipeline_handle;
    PipelineHandle                                              _draw_indirect_pipeline_handle;
    PipelineHandle                                               _light_render_pipeline_handle;
    PipelineHandle                                          _clustered_forward_pipeline_handle;

    RenderPassHandle                                                    _depth_pre_pass_handle;
    RenderPassHandle                                                       _render_pass_handle;

    vk::Framebuffer                                                        _framebuffers_begin;

    CommandArenaHandle                                                   _command_arenas_begin;

  public:

    b8 init();

    b8 record_frame(const frame::ID frame_id);

    b8 resize(const u64 width, const u64 height);

    b8 terminate();

  };

  class ClusteredForwardRendererBuilder {
  private:
    GPUDeviceBuilder* _gpu_builder = nullptr;


  public:

    b8 init(GPUDeviceBuilder* extern_builder) {
      _gpu_builder = extern_builder;

      return true;
    }


    b8 build(ClusteredForwardRenderer* renderer) {


      return true;
    }


    b8 terminate() {

      return true;
    }

  };

  class TestRenderer : Renderer {
  private:
    vk::ResourcePool::ImageHandle _render_target_handles_begin;
    FramebufferHandle               _framebuffer_handles_begin;
    vk::PipelinePool::PipelineHandle     _draw_pipeline_handle;
    vk::PipelinePool::RenderPassHandle     _render_pass_handle;

    // first call will always return primary command buffer
    // reset before calling to reset and retrieve the primary buffer
    vk::CommandBuffer get_command_buffer(const frame::ID frame_id) {
      vk::CommandPool& pool = get_command_pool(frame_id);
      return pool.obtain_command_buffer(pool.obtain_command_arena());
    }

  public:
    b8 init() {

      create_commands(RoxiNumFrames);
      create_descriptors(1, 0, 0, 0);

     
      gpu::ResourceInfo image_infos[RoxiNumFrames];
      vk::Extent<2> window_size = get_current_extent();
      for(u32 i = 0; i < RoxiNumFrames; i++) {
        image_infos[i].type = gpu::ResourceType::RenderTarget;
        image_infos[i].image.width = window_size.value.width;
        image_infos[i].image.height = window_size.value.height;
        image_infos[i].image.depth = 1;
      }

      create_resources(0, RoxiNumFrames, nullptr, image_infos);

      _render_target_handles_begin = get_resource_pool().create_images(gpu::ImageType::RenderTarget, window_size.value);

      vk::PipelinePoolBuilder pipeline_builder{};

      pipeline_builder.set_descriptor_pool(get_descriptor_pool());

      vk::AttachmentInfo colour_attachment{};
      colour_attachment.format = get_image_format(gpu::ImageType::RenderTarget);
      colour_attachment.layout = get_resource_pool().obtain_image(_render_target_handles_begin).get_image_layout();
      colour_attachment.load_op = RenderPassOperationType::Clear;

      _render_pass_handle = pipeline_builder.add_render_pass(1, &colour_attachment, {});

      vk::PipelineInfo info{};
      info.type = vk::DispatchType::Draw;
      info.graphics.render_pass_id = _render_pass_handle;
      info.graphics.vertex_shader_name = "test.vert";
      info.graphics.fragment_shader_name = "test.frag";

      _draw_pipeline_handle = pipeline_builder.add_pipeline(info);

      create_pipelines(pipeline_builder);

      vk::FramebufferCreation framebuffer_creations[RoxiNumFrames];
      for(u32 i = 0; i < RoxiNumFrames; i++) {
        framebuffer_creations[i].reset();
        framebuffer_creations[i].add_render_texture(_render_target_handles_begin);
        framebuffer_creations[i].set_scaling(1.f, 1.f, true);
      }

      _framebuffer_handles_begin = create_framebuffers(RoxiNumFrames, framebuffer_creations);

      return true;
    }

    b8 update(const frame::ID frame_id) {
      vk::CommandBuffer command_buffer = get_command_buffer(frame_id);
      command_buffer
        .begin()
        .begin_render_pass
          ( get_pipeline_pool()
              .obtain_render_pass(_render_pass_handle)
            , get_current_extent().value.width
            , get_current_extent().value.height
            , obtain_framebuffer(_framebuffer_handles_begin + frame_id)
          )
        .bind_descriptor_pool(get_descriptor_pool())
        .bind_pipeline(get_pipeline_pool().obtain_pipeline(_draw_pipeline_handle))
        .record_draw(3, 1)
        .end_render_pass()
        .end();



      return true;
    }

  };

  namespace renderer {

    DEFINE_JOB(async_load_job) {
      ClusteredForwardRenderer* renderer = (ClusteredForwardRenderer*)param;

      return true;

    }
   
    static Job create_async_load_task() {
      Job result{};
      return result;
    }

  }		// -----  end of namespace renderer  ----- 


}		// -----  end of namespace roxi  ----- 
