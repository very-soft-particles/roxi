// =====================================================================================
//
//       Filename:  vk_pipeline.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-05 8:04:57 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_resource_manager.hpp"
#include "vk_image.hpp"
#include "vk_renderpass.hpp"
#include "vk_descriptors.hpp"
#include "vk_resource.hpp"
#include "data.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {

  namespace vk {

    static constexpr u32 s_max_pipelines = 128;
 
    enum class PipelineType {
      Graphics,
      Compute,
      RayTracing,
      Max
    };

    enum class DispatchType {
      Compute,
      ComputeIndirect,
      Draw,
      DrawIndirect,
      DrawIndexed,
      DrawIndexedIndirect,
      Max
    };

    static VkPipelineBindPoint get_bind_point(const PipelineType type) {
        return type == PipelineType::Graphics ? 
          VK_PIPELINE_BIND_POINT_GRAPHICS :
            type == PipelineType::Compute ? 
          VK_PIPELINE_BIND_POINT_COMPUTE :
            type == PipelineType::RayTracing ? 
          VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR :
          VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    static constexpr PipelineType get_pipeline_type_from_dispatch_type(const DispatchType type) {
        return type == DispatchType::Draw 
            || type == DispatchType::DrawIndexed 
            || type == DispatchType::DrawIndexedIndirect 
            || type == DispatchType::DrawIndirect ? 
          PipelineType::Graphics :
            type == DispatchType::Compute 
            || type == DispatchType::ComputeIndirect ?
          PipelineType::Compute :
          PipelineType::Max;
    }


    template<PipelineType Type>
    static constexpr VkPipelineBindPoint get_bind_point() {
        return Type == PipelineType::Graphics ? 
          VK_PIPELINE_BIND_POINT_GRAPHICS :
            Type == PipelineType::Compute ? 
          VK_PIPELINE_BIND_POINT_COMPUTE :
            Type == PipelineType::RayTracing ? 
          VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR :
          VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    template<PipelineType Type>
    using PipelineCreateInfoT 
      = typename lofi::conditional
          < (Type == PipelineType::Graphics)
          , VkGraphicsPipelineCreateInfo
          , typename lofi::conditional
            < (Type == PipelineType::Compute) 
            , VkComputePipelineCreateInfo
            , VkRayTracingPipelineCreateInfoKHR
            >::type
          >::type;

    struct PipelineInfo {
      union {
        struct {
          const char* vertex_shader_name;
          const char* fragment_shader_name;
          u32 render_pass_id = MAX_u32;
          u32 extent_x = 0;
          u32 extent_y = 0;
        } graphics;
        struct {
          const char* shader_name;
        } compute;
      };

      DispatchType type;
//      Array<ResourceInfo*, s_max_resources> input_resources{};      // uniform buffers, samplers, readonly storage buffers, input attribute
//      Array<ResourceInfo*, s_max_resources> output_resources{};     // storage buffers, storage images, output attribute
    };

    template<PipelineType Type>
    class PipelineCreation;

    template<>
    class PipelineCreation<PipelineType::Graphics> {
    private:
      DispatchType _dispatch_type = DispatchType::Max;
      StackArray<VkPipelineShaderStageCreateInfo> _shader_stages;
      StackArray<VkVertexInputAttributeDescription> _vertex_attribute_description{};
      StackArray<VkVertexInputBindingDescription> _vertex_binding_description{};
      VkSpecializationInfo spec_info{};
      VkPipelineVertexInputStateCreateInfo _vertex_input_state;
      VkPipelineInputAssemblyStateCreateInfo _input_assembly;
      VkPipelineRasterizationStateCreateInfo _rasterizer;
      VkPipelineColorBlendAttachmentState _color_blend_attachment;
      VkPipelineColorBlendStateCreateInfo _color_blend_create_info;
      VkPipelineMultisampleStateCreateInfo _multisampling;
      VkPipelineLayout _pipeline_layout;
      VkPipelineDepthStencilStateCreateInfo _depth_stencil;
      StackArray<VkViewport> _viewports;
      StackArray<VkRect2D> _scissors;
      VkRenderPass _render_pass = VK_NULL_HANDLE;
      VkPipelineViewportStateCreateInfo _viewport_state{};
      u32 _render_pass_handle = MAX_u32;
      b8 _discard_fragments = true;
      b8 _shadow_pass = false;

    public:
      b8 init();
      const DispatchType get_dispatch_type() const;

      const DispatchType get_dispatch_type(); 

      const u32 get_render_pass_handle() {
        return _render_pass_handle;
      }

      b8 set_render_pass_handle(u32 render_pass_handle) {
        _render_pass_handle = render_pass_handle;
        return true;
      }

      b8 add_vertex_binding(u32 binding = 0, u32 stride = 0, VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX);

      b8 add_vertex_attribute(u32 binding = 0, u32 location = 0, VkFormat format = VK_FORMAT_B8G8R8A8_SRGB, u32 offset = 0);

      b8 set_shadow_pass(const b8 set_bool);
      b8 set_dispatch_type(const DispatchType type);

      b8 set_pipeline_layout(VkPipelineLayout layout);

      b8 set_vertex_descriptors(const u32 offset, const u32 size);

      b8 add_viewport(const float x, const float y, const float height, const float width, const float min_depth, const float max_depth);

      b8 add_scissor(const VkOffset2D offset, const VkExtent2D extent);

      b8 set_fragment_descriptors(const u32 offset, const u32 size);

      b8 set_spec_info(VkSpecializationInfo specialization_info);

      u32 get_vertex_descriptors_offset() const;

      u32 get_vertex_descriptors_size() const;

      u32 get_fragment_descriptors_offset() const;

      u32 get_fragment_descriptors_size() const;

      b8 set_vertex_shader(VkShaderModule shader);

      b8 set_fragment_shader(VkShaderModule shader);

      b8 set_render_pass(VkRenderPass renderpass);

      VkGraphicsPipelineCreateInfo get_create_info(u32 subpass_index = 0, RenderPass* render_pass = nullptr);
    };
 
    template<>
    class PipelineCreation<PipelineType::Compute> {
    private:
      VkPipelineShaderStageCreateInfo shader_stage{};
      DispatchType _dispatch_type = DispatchType::Max;
      VkSpecializationInfo spec_info{};
      StackArray<VkSpecializationInfo> _spec_infos;
      VkPipelineLayout                     _layout;

    public:
      const DispatchType get_dispatch_type() const {
        return _dispatch_type;
      }

      const DispatchType get_dispatch_type() {
        return _dispatch_type;
      }

      b8 set_dispatch_type(const DispatchType type) {
        RX_CHECK(type == DispatchType::Compute || type == DispatchType::ComputeIndirect, "tried to dispatch a compute pipeline with a graphics dispatch type!");
        _dispatch_type = type;
        return true;
      }

      b8 set_pipeline_layout(VkPipelineLayout layout) {
        _layout = layout;

        return true;
      }

      b8 set_shader(VkShaderModule shader) {
        shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage.pNext = nullptr;
        shader_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stage.flags = 0;
        shader_stage.module = shader;
        shader_stage.pName = "main"; 
        shader_stage.pSpecializationInfo = &spec_info;
        return true;
      }

      b8 set_spec_info(VkSpecializationInfo specialization_info) {
        spec_info = specialization_info;
        return true;
      }

      VkComputePipelineCreateInfo get_create_info() {
        VkComputePipelineCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        create_info.layout = _layout;
        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;
        create_info.stage = shader_stage;

        return create_info;
      }
    };

    class Pipeline {
    private:
      friend class PipelinePoolBuilder;
      VkPipeline _pipeline = VK_NULL_HANDLE;
      VkPipelineLayout _layout = VK_NULL_HANDLE;
      DispatchType _dispatch_type = DispatchType::Max;

    public:
      const VkPipeline get_pipeline() const;

      const VkPipelineLayout get_pipeline_layout() const;

      const VkPipelineBindPoint get_pipeline_bind_point() const;

      b8 terminate(Context* context);
    };

     static VkPipelineLayout create_pipeline_layout(Context* context, const DescriptorSetLayout* layouts, const u64 descriptor_set_count, const VkPushConstantRange* push_constant_ranges, const u64 push_constant_range_count);
 
    class PipelinePoolBuilder;

    class PipelinePool {
    private:
      friend class PipelinePoolBuilder;
      VkPipelineCache _pipeline_cache;
      Array<RenderPass> _render_passes;
      Array<Pipeline> _pipelines;
      Array<Framebuffer> _framebuffers;
      DescriptorSetLayout _descriptor_set_layouts[4];
      VkPipelineLayout _layout;
    public:
      using PipelineHandle = typename Array<Pipeline>::index_t;
      using RenderPassHandle = typename Array<RenderPass>::index_t;
      using FramebufferHandle = typename Array<Framebuffer>::index_t;
      static constexpr u32 PipelineHandleMax = lofi::index_type_max<PipelineHandle>::value;

    //  b8 init(Context* context, const u32 pool_id, const u32 params_ubo_index, const u32 graphics_pipeline_count, const u32 compute_pipeline_count, PipelineInfo* graphics_infos, PipelineInfo* compute_infos) {
    //    VkDescriptorSetLayoutBinding binding{};
    //    binding.stageFlags = VK_SHADER_STAGE_ALL;
    //    binding.descriptorCount = RoxiBindlessDescriptorCount;
    //    // ubo, storage buffer, texture, storage image
    //    const u32 set_count = 4;
    //    DescriptorSetLayoutCreation descriptor_creation;
    //    descriptor_creation.init();
    //    descriptor_creation.set_flags(
    //      VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT
    //      // ?? | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
    //    );
    //    for(u32 i = 0; i < set_count; i++) {
    //      binding.descriptorType = get_descriptor_type((vk::DescriptorBufferType)i);
    //      binding.binding = 0;
    //      // all descriptor buffers are device side, but the buffers themselves can be host side
    //      // indexing is handled by a combination of push constant and draw param uniform buffers
    //      descriptor_creation.add_binding(binding);
    //      _descriptor_set_layouts[i].init(context, descriptor_creation);
    //      descriptor_creation.clear_bindings();
    //    }

    //    VkSpecializationMapEntry spec_map_info{};
    //    spec_map_info.constantID = pool_id;
    //    spec_map_info.offset = 0;
    //    spec_map_info.size = sizeof(u32);

    //    VkPushConstantRange pc_range{};
    //    pc_range.offset = 0;
    //    pc_range.size = sizeof(pc::FrameParams);

    //    _layout = create_pipeline_layout(context, _descriptor_set_layouts, 4, &pc_range, 1);
    //    u8* buffer = (u8*)ALLOCATE(
    //        sizeof(PipelineCreation<PipelineType::Graphics>) * graphics_pipeline_count 
    //      + sizeof(PipelineCreation<PipelineType::Compute>) * compute_pipeline_count
    //      + sizeof(VkGraphicsPipelineCreateInfo) * graphics_pipeline_count
    //      + sizeof(VkPipeline) * (graphics_pipeline_count + compute_pipeline_count)
    //      + sizeof(VkComputePipelineCreateInfo) * compute_pipeline_count);

    //    Array<PipelineCreation<PipelineType::Graphics>> graphics_creations{buffer};
    //    PipelineCreation<PipelineType::Graphics>* const graphics_creation_begin = graphics_creations.push(graphics_pipeline_count);
    //    buffer += sizeof(PipelineCreation<PipelineType::Graphics>) * graphics_pipeline_count;

    //    Array<VkGraphicsPipelineCreateInfo> graphics_create_infos{buffer};
    //    VkGraphicsPipelineCreateInfo* const graphics_pipelines_begin = graphics_create_infos.push(graphics_pipeline_count);
    //    buffer += sizeof(VkGraphicsPipelineCreateInfo) * graphics_pipeline_count;
 
    //    for(u32 i = 0; i < graphics_pipeline_count; i++) {
    //      const u32 vert_shader_idx = resource::helpers::find_shader_index_from_shader_name(graphics_infos[i].graphics.vertex_shader_name);
    //      const u32 frag_shader_idx = resource::helpers::find_shader_index_from_shader_name(graphics_infos[i].graphics.fragment_shader_name);

    //      VkShaderModuleCreateInfo shader_create_info
    //      { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    //      , nullptr
    //      , 0
    //      , resource::shader_spirv_size(vert_shader_idx)
    //      , resource::shader_spirv_data(vert_shader_idx)
    //      };
   
    //      VkShaderModule   mod;
    //
    //      VK_CHECK(context->get_device().get_device_function_table()
    //        .vkCreateShaderModule(context->get_device().get_device()
    //            , &shader_create_info
    //            , CALLBACKS()
    //            , &mod
    //            )
    //        , "failed to create vertex shader module");

    //      graphics_creation_begin[i].set_vertex_shader(mod);

    //      shader_create_info.codeSize = resource::shader_spirv_size(frag_shader_idx);
    //      shader_create_info.pCode = resource::shader_spirv_data(frag_shader_idx);

    //      VK_CHECK(context->get_device().get_device_function_table()
    //        .vkCreateShaderModule(context->get_device().get_device()
    //            , &shader_create_info
    //            , CALLBACKS()
    //            , &mod
    //            )
    //        , "failed to create fragment shader module");

    //      VkSpecializationInfo spec_info;
    //      spec_info.dataSize = sizeof(u32);
    //      spec_info.pData = &params_ubo_index;
    //      spec_info.mapEntryCount = 1;
    //      spec_info.pMapEntries = &spec_map_info;

    //      graphics_creation_begin[i].set_fragment_shader(mod);
    //      graphics_creation_begin[i].set_dispatch_type(graphics_infos[i].type);
    //      graphics_creation_begin[i].set_render_pass(graphics_infos[i].graphics.render_pass_id);
    //      graphics_creation_begin[i].set_spec_info(spec_info);
    //      graphics_creation_begin[i].set_pipeline_layout(_layout);

    //      graphics_pipelines_begin[i] = graphics_creation_begin[i].get_create_info();
    //    }

    //    Array<PipelineCreation<PipelineType::Compute>> compute_creations{buffer};
    //    PipelineCreation<PipelineType::Compute>* const compute_creation_begin = compute_creations.push(compute_pipeline_count);
    //    buffer += sizeof(PipelineCreation<PipelineType::Compute>) * compute_pipeline_count;
    //    Array<VkComputePipelineCreateInfo> compute_create_infos{buffer};
    //    VkComputePipelineCreateInfo* const compute_pipelines_begin = compute_create_infos.push(graphics_pipeline_count);
    //    buffer += sizeof(VkComputePipelineCreateInfo) * compute_pipeline_count;

    //    for(u32 i = 0; i < compute_pipeline_count; i++) {
    //      const u32 shader_idx = resource::helpers::find_shader_index_from_shader_name(compute_infos[i].compute.shader_name);

    //      VkShaderModuleCreateInfo shader_create_info
    //      { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    //      , nullptr
    //      , 0
    //      , resource::shader_spirv_size(shader_idx)
    //      , resource::shader_spirv_data(shader_idx)
    //      };
   
    //      VkShaderModule   mod;
    //
    //      VK_CHECK(context->get_device().get_device_function_table()
    //        .vkCreateShaderModule(context->get_device().get_device()
    //            , &shader_create_info
    //            , CALLBACKS()
    //            , &mod
    //            )
    //        , "failed to create compute shader module");

    //      compute_creation_begin[i].set_shader(mod);

    //      VkSpecializationInfo spec_info;
    //      spec_info.dataSize = sizeof(u32);
    //      spec_info.pData = &params_ubo_index;
    //      spec_info.mapEntryCount = 1;
    //      spec_info.pMapEntries = &spec_map_info;

    //      compute_creation_begin[i].set_dispatch_type(graphics_infos[i].type);
    //      compute_creation_begin[i].set_spec_info(spec_info);
    //      compute_creation_begin[i].set_pipeline_layout(_layout);

    //      compute_pipelines_begin[i] = compute_creation_begin[i].get_create_info();
    //    }

    //    VkPipeline* vk_pipelines = (VkPipeline*)buffer;


    //    _pipelines.move_ptr(ALLOCATE(sizeof(Pipeline) * (graphics_pipeline_count + compute_pipeline_count)));
    //    Pipeline* pipelines_begin = _pipelines.push(compute_pipeline_count + graphics_pipeline_count);

    //    if(graphics_pipeline_count) {
    //      VK_CHECK
    //        ( context->get_device()
    //          .get_device_function_table()
    //          .vkCreateGraphicsPipelines(context->get_device().get_device()
    //            , s_pipeline_cache
    //            , graphics_pipeline_count
    //            , graphics_create_infos.get_buffer()
    //            , CALLBACKS()
    //            , vk_pipelines)
    //        , "failed to create graphics pipelines");
    //        vk_pipelines += graphics_pipeline_count;
    //    } 
    //    if(compute_pipeline_count) {
    //    VK_CHECK
    //      ( context->get_device()
    //        .get_device_function_table()
    //        .vkCreateComputePipelines(context->get_device().get_device()
    //          , s_pipeline_cache
    //          , compute_pipeline_count
    //          , compute_create_infos.get_buffer()
    //          , CALLBACKS()
    //          , vk_pipelines)
    //      , "failed to create compute pipelines");
    //      
    //    }
    //    
    //    if(graphics_infos == nullptr && compute_infos == nullptr) {
    //      LOG("failed to create pipelines, both graphics and compute pipeline infos were null", Warn);
    //      return false;
    //    }

    //    const u32 total_num_pipelines = compute_pipeline_count + graphics_pipeline_count;
    //    for(size_t i = 0; i < total_num_pipelines; i++) {
    //      pipelines_begin[i]._pipeline = vk_pipelines[i];
    //      if(i < graphics_pipeline_count) {
    //        _pipelines[i]._dispatch_type = graphics_infos[i].type;
    //        continue;
    //      }
    //      _pipelines[i]._dispatch_type = compute_infos[i - graphics_pipeline_count].type;
    //    }
    //    FREE(buffer);
    //    return true;
    //  }

      const VkDeviceSize get_uniform_layout_size(Context* context) const {
        return _descriptor_set_layouts[0].get_layout_size(context);
      }

      const VkDeviceSize get_storage_layout_size(Context* context) const {
        return _descriptor_set_layouts[1].get_layout_size(context);
      }

      const VkDeviceSize get_texture_layout_size(Context* context) const {
        return _descriptor_set_layouts[2].get_layout_size(context);
      }

      const VkDeviceSize get_image_layout_size(Context* context) const {
        return _descriptor_set_layouts[3].get_layout_size(context);
      }

      const Pipeline& obtain_pipeline(PipelineHandle handle) const {
        return _pipelines[handle];
      }

      const RenderPass& obtain_render_pass(RenderPassHandle handle) const {
        return _render_passes[handle];
      }

      b8 terminate(Context* context);

    private:
      b8 create_pipelines(Context* context, u32 num_graphics_pipelines, u32 num_compute_pipelines, PipelineCreation<PipelineType::Graphics>* graphics_creations, PipelineCreation<PipelineType::Graphics>* compute_creations);

    };
   
   class PipelinePoolBuilder {
    private:
      Context* _context = nullptr;
      const DescriptorPool* _descriptor_pool = nullptr;
      Array<RenderPass> _render_passes;
      Array<PipelineCreation<PipelineType::Graphics>> _graphics_pipelines;
      Array<PipelineCreation<PipelineType::Compute>> _compute_pipelines;
      DescriptorSetLayoutCreation _descriptor_set_layout_creations[4];
      struct SpecializationInfo {
        u32 id;
        u32 size;
        void* data;
      };
      Array<SpecializationInfo> _specialization_infos;

      PipelineCreation<PipelineType::Graphics> add_graphics_pipeline(const PipelineInfo info);
 
      PipelineCreation<PipelineType::Compute> add_compute_pipeline(const PipelineInfo info);

    public:
      using PipelineHandle = PipelinePool::PipelineHandle;
      using RenderPassHandle = PipelinePool::RenderPassHandle;

      b8 init(Context* context);

      template<typename T>
      const u32 set_specialization_constant(const u32 id, const T* data) {
        const u32 result = _specialization_infos.get_size();
        *(_specialization_infos.push(1)) = { id, sizeof(T), (void*)data };
        return result;
      }

      const PipelinePool::RenderPassHandle add_render_pass(const u32 attachment_count, AttachmentInfo* colour_attachments, AttachmentInfo depth_attachment);

      const PipelineHandle add_pipeline(const PipelineInfo info);

      b8 set_descriptor_pool(const DescriptorPool& new_pool) {
        _descriptor_pool = &new_pool;
        return true;
      }

      b8 build(PipelinePool* pool);

      b8 terminate();

    private:

      b8 create_pipelines(Pipeline* pipelines, u32 num_graphics_pipelines, u32 num_compute_pipelines, VkGraphicsPipelineCreateInfo* graphics_infos, VkComputePipelineCreateInfo* compute_infos, DispatchType* dispatch_types, const VkPipelineCache cache) const;

    };

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
