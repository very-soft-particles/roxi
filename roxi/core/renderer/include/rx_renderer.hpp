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
#include "rx_thread_pool.hpp"
#include "rx_vocab.h"
#include "vk_buffer.hpp"
#include "vk_command.hpp"
#include "vk_descriptors.hpp"
#include "vk_image.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderpass.hpp"
#include "vk_resource.hpp"
#include "rx_resource_manager.hpp"
#include <vulkan/vulkan_core.h>

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

    const u32 get_pool_handle(const frame::ID frame_id) {
      return frame_id % _render_frame_count;
    }

  public:
    b8 init(GPUDevice* device, const GPUDevice::QueueHandle handle);

    b8 update(const frame::ID frame_id);

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
