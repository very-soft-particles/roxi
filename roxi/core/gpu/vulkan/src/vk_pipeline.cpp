// =====================================================================================
//
//       Filename:  vk_pipeline.cpp
//
//    Description:  vulkan pipeline and pipeline pool implementations 
//
//        Version:  1.0
//        Created:  2024-11-18 10:21:51 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_pipeline.hpp"
#include "rx_allocator.hpp"
#include "rx_resource_manager.hpp"
#include "vk_renderpass.hpp"
#include <vulkan/vulkan_core.h>


namespace roxi {
  namespace vk {
     const VkPipeline Pipeline::get_pipeline() const {
      return _pipeline;
    }

    const VkPipelineLayout Pipeline::get_pipeline_layout() const {
      return _layout;
    }

    const VkPipelineBindPoint Pipeline::get_pipeline_bind_point() const {
      return get_bind_point(get_pipeline_type_from_dispatch_type(_dispatch_type));
    }

    b8 Pipeline::terminate(Context* context) {
      if(_pipeline == VK_NULL_HANDLE) {
        return false;
      }
      context->get_device().get_device_function_table()
        .vkDestroyPipeline(context->get_device().get_device()
            , _pipeline
            , CALLBACKS());
      return true;
    }
 
    const DispatchType PipelineCreation<PipelineType::Graphics>::get_dispatch_type() const {
      return _dispatch_type;
    }

    const DispatchType PipelineCreation<PipelineType::Graphics>::get_dispatch_type() {
      return _dispatch_type;
    }

    b8 PipelineCreation<PipelineType::Graphics>::set_dispatch_type(const DispatchType type) {
      RX_CHECK(type != DispatchType::Compute && type != DispatchType::ComputeIndirect, "tried to dispatch a graphics pipeline with a compute dispatch type!");
      _dispatch_type = type;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::set_pipeline_layout(VkPipelineLayout layout) {
      _pipeline_layout = layout;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::set_spec_info(VkSpecializationInfo specialization_info) {
      spec_info = specialization_info;
      return true;
    }
    b8 PipelineCreation<PipelineType::Graphics>::init() {
      _viewports.clear();
      _scissors.clear();
      _shader_stages.clear();
      _vertex_attribute_description.clear();
      _vertex_binding_description.clear();
      _shadow_pass = false;
      _discard_fragments = false;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::set_vertex_shader(VkShaderModule shader) {
      VkPipelineShaderStageCreateInfo& shader_stage = *(_shader_stages.push(1));
      shader_stage.sType 
        = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shader_stage.pNext 
        = nullptr;
      shader_stage.stage 
        = VK_SHADER_STAGE_VERTEX_BIT;
      shader_stage.flags 
        = 0;
      shader_stage.module 
        = shader;
      shader_stage.pName 
        = "main"; 
      shader_stage.pSpecializationInfo = &spec_info;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::set_fragment_shader(VkShaderModule shader) {
      _discard_fragments = false;
      VkPipelineShaderStageCreateInfo& shader_stage = *(_shader_stages.push(1));
      shader_stage.sType 
        = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shader_stage.pNext 
        = nullptr;
      shader_stage.stage 
        = VK_SHADER_STAGE_FRAGMENT_BIT;
      shader_stage.flags 
        = 0;
      shader_stage.module 
        = shader;
      shader_stage.pName 
        = "main"; 
      shader_stage.pSpecializationInfo = &spec_info;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::add_vertex_binding(u32 binding, u32 stride, VkVertexInputRate input_rate) {
      VkVertexInputBindingDescription& desc = *(_vertex_binding_description.push(1));
      desc.binding = binding;
      desc.inputRate = input_rate;
      desc.stride = stride;

      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::add_vertex_attribute(u32 binding, u32 location, VkFormat format, u32 offset) {

      VkVertexInputAttributeDescription& desc = *(_vertex_attribute_description.push(1));
      desc.binding = binding;
      desc.format = format;
      desc.location = location;
      desc.offset = offset;

      return true;
    }

    VkPipelineLayout create_pipeline_layout(Context* context, const DescriptorSetLayout* layouts, const u64 descriptor_set_count, const VkPushConstantRange* push_constant_ranges, const u64 push_constant_range_count) {
      VkPipelineLayout result{};
      VkDescriptorSetLayout descriptor_set_layouts[4];
      for(u64 i = 0; i < descriptor_set_count; i++) {
        descriptor_set_layouts[i] = layouts[i].get_descriptor_set_layout();
      }
      VkPipelineLayoutCreateInfo create_info
      { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
      , nullptr
      , 0
      , (u32)descriptor_set_count
      , descriptor_set_layouts
      , (u32)push_constant_range_count
      , push_constant_ranges
      };

      VK_ASSERT(context->get_device().get_device_function_table()
        .vkCreatePipelineLayout(context->get_device().get_device()
            , &create_info, CALLBACKS(), &result),
        "failed to create pipeline layout");
      return result;
    }

    b8 PipelinePool::create_pipelines(Context* context, u32 graphics_pipeline_count, u32 compute_pipeline_count, PipelineCreation<PipelineType::Graphics>* graphics_creations, PipelineCreation<PipelineType::Graphics>* compute_creations) {     // NOLINT
     

     return true;
    }


    b8 PipelineCreation<PipelineType::Graphics>::set_render_pass(VkRenderPass renderpass) {
      _render_pass = renderpass;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::add_scissor(const VkOffset2D offset, const VkExtent2D extent) {
      VkRect2D& scissor = *(_scissors.push(1));
      scissor.offset = offset;
      scissor.extent = extent;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::add_viewport(const float x, const float y, const float height, const float width, const float min_depth, const float max_depth) {
      VkViewport& viewport = *(_viewports.push(1));
      viewport.x = x;
      viewport.y = y;
      viewport.height = height;
      viewport.width = width;
      viewport.minDepth = min_depth;
      viewport.maxDepth = max_depth;
      return true;
    }

    b8 PipelineCreation<PipelineType::Graphics>::set_shadow_pass(const b8 set_bool) {
      _shadow_pass = set_bool;
      return true;
    }

    VkGraphicsPipelineCreateInfo PipelineCreation<PipelineType::Graphics>::get_create_info(u32 subpass_index, RenderPass* render_pass) {
      _color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      _color_blend_create_info.pNext = nullptr;
      _color_blend_create_info.flags = 0;
      _color_blend_create_info.logicOpEnable = VK_FALSE;
      _color_blend_create_info.attachmentCount = 1;
      _color_blend_create_info.pAttachments = &_color_blend_attachment;

      _rasterizer = {};
      _rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      RX_TRACEF("discard fragments? %u", _discard_fragments);
      _rasterizer.rasterizerDiscardEnable = _discard_fragments ? VK_TRUE : VK_FALSE;
      _rasterizer.depthClampEnable = VK_FALSE;
      _rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      _rasterizer.lineWidth = 1.f;
      //_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      _rasterizer.cullMode = VK_CULL_MODE_NONE;
      _rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      _rasterizer.depthBiasEnable = _shadow_pass ? VK_TRUE : VK_FALSE;

      _input_assembly = {};
      _input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      _input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      _input_assembly.primitiveRestartEnable = VK_FALSE;

      _vertex_input_state = {};
      _vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      _vertex_input_state.pVertexAttributeDescriptions = _vertex_attribute_description.get_buffer();
      _vertex_input_state.vertexAttributeDescriptionCount = _vertex_attribute_description.get_size();
      _vertex_input_state.pVertexBindingDescriptions = _vertex_binding_description.get_buffer();
      _vertex_input_state.vertexBindingDescriptionCount = _vertex_binding_description.get_size();


      const u32 viewport_count = _viewports.get_size();
      const u32 scissor_count = _scissors.get_size();

      _viewport_state = {};
      _viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      _viewport_state.pNext = nullptr;
      _viewport_state.viewportCount = viewport_count;
      _viewport_state.scissorCount = scissor_count;
      _viewport_state.pViewports = viewport_count ? _viewports.get_buffer() : nullptr;
      _viewport_state.pScissors = scissor_count ? _scissors.get_buffer() : nullptr;


      _multisampling = {};
      _multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      _multisampling.sampleShadingEnable = VK_FALSE;
      _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      
      VkGraphicsPipelineCreateInfo create_info{};
      create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      create_info.pNext = nullptr;
      create_info.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
      create_info.pMultisampleState = &_multisampling;
      create_info.pStages = _shader_stages.get_buffer();
      create_info.stageCount = _shader_stages.get_size();
      create_info.pVertexInputState = &_vertex_input_state;
      create_info.pInputAssemblyState = &_input_assembly;
      create_info.pColorBlendState = &_color_blend_create_info;
      create_info.layout = _pipeline_layout;
      create_info.renderPass = render_pass ? render_pass->get_render_pass() : VK_NULL_HANDLE;
      create_info.subpass = subpass_index;
      create_info.basePipelineHandle = VK_NULL_HANDLE;
      create_info.basePipelineIndex = -1;
      create_info.pRasterizationState = &_rasterizer;
      create_info.pViewportState = &_viewport_state;
      create_info.pInputAssemblyState = &_input_assembly;
      create_info.renderPass = _render_pass;

      return create_info;
    }

    b8 PipelinePool::terminate(Context* context) {
      b8 result = true;
      const PipelineHandle pipeline_count = _pipelines.get_size();
      const RenderPassHandle render_pass_count = _render_passes.get_size();
      for(PipelineHandle i = 0; i < pipeline_count; i++) {
        if(!_pipelines[i].terminate(context)) {
          LOG("failed to terminate a pipeline in PipelinePool::terminate(Context*)", Error);
          result = false;
        }
      }
      if(pipeline_count > 0)
        FREE(_pipelines.get_buffer());
      if(render_pass_count > 0) 
        FREE(_render_passes.get_buffer());
      return true;
    }
 
    b8 PipelinePoolBuilder::create_pipelines(Pipeline* pipelines, u32 num_graphics_pipelines, u32 num_compute_pipelines, VkGraphicsPipelineCreateInfo* graphics_infos, VkComputePipelineCreateInfo* compute_infos, DispatchType* dispatch_types, const VkPipelineCache cache) const {     // NOLINT

      Array<VkPipeline> _pipelines{ALLOCATE(sizeof(VkPipeline) * s_max_pipelines)};
      _pipelines.clear();
      if (compute_infos != nullptr && num_compute_pipelines != 0) {
        RX_TRACEF("creating %u compute pipelines", num_compute_pipelines);
        VK_CHECK
          ( _context->get_device()
            .get_device_function_table()
            .vkCreateComputePipelines(_context->get_device().get_device()
              , cache
              , num_compute_pipelines
              , compute_infos
              , CALLBACKS()
              , _pipelines.push(num_compute_pipelines))
          , "failed to create compute pipelines");
      }
      if(graphics_infos != nullptr && num_graphics_pipelines != 0) {
        RX_TRACEF("creating %u graphics pipelines", num_graphics_pipelines);
        VK_CHECK
          ( _context->get_device()
            .get_device_function_table()
            .vkCreateGraphicsPipelines(_context->get_device().get_device()
              , cache
              , num_graphics_pipelines
              , graphics_infos
              , CALLBACKS()
              , _pipelines.push(num_graphics_pipelines))
          , "failed to create graphics pipelines");
      } 
      const u32 total_num_pipelines = num_compute_pipelines + num_graphics_pipelines;
      RX_TRACEF("copying over %u total pipelines", total_num_pipelines);
      for(size_t i = 0; i < total_num_pipelines; i++) {
        pipelines[i]._pipeline = _pipelines[i];
        pipelines[i]._dispatch_type = dispatch_types[i];
        if(i < num_compute_pipelines) {
          RX_TRACEF("compute pipeline at %llu", i);
          pipelines[i]._layout = compute_infos[i].layout;
          continue;
        }
        RX_TRACEF("graphics pipeline at %llu", i);
        pipelines[i]._layout = graphics_infos[i - num_compute_pipelines].layout;
      }
      RX_TRACE("freeing pipelines");
      if(total_num_pipelines > 0)
        FREE(_pipelines.get_buffer());
      RX_TRACE("returning from create pipelines in pool builder");
      return true;
    }

    PipelineCreation<PipelineType::Graphics> PipelinePoolBuilder::add_graphics_pipeline(const PipelineInfo info) {
      PipelineCreation<PipelineType::Graphics> creation{};
      creation.init();

      creation.add_viewport(0.f, 0.f, (float)info.graphics.extent_x, (float)info.graphics.extent_y, 0.f, 1.f);
      creation.add_scissor({0,0}, {info.graphics.extent_x, info.graphics.extent_y});

      const u64 vertex_shader_index = resource::helpers::find_shader_index_from_shader_name((const char*)info.graphics.vertex_shader_name);
      RX_RETURN(vertex_shader_index != MAX_u64
          , "failed to find vertex shader"
          , PipelineCreation<PipelineType::Graphics>{});

      const u64 fragment_shader_index = resource::helpers::find_shader_index_from_shader_name((const char*)info.graphics.fragment_shader_name);
      RX_RETURN(fragment_shader_index != MAX_u64
          , "failed to find vertex shader"
          , PipelineCreation<PipelineType::Graphics>{});

      RX_RETURN(creation.set_dispatch_type(info.type), "failed to set correct dispatch type when adding graphics pipeline"
          , PipelineCreation<PipelineType::Graphics>{});

      RX_RETURN(creation.set_render_pass_handle(info.graphics.render_pass_id)
          , "failed to set render pass handle when adding graphics pipeline"
          , PipelineCreation<PipelineType::Graphics>{});
      RX_TRACE("getting vertex spirv file");
      File vertex_spirv_file = resource::shader_spirv_file(vertex_shader_index);
      u64 spirv_size = vertex_spirv_file.get_size();

      RX_TRACEF("vertex spirv size = %llu", spirv_size);

      u32* spirv_buffer = (u32*)ALLOCATE(spirv_size);

      RX_RETURN(vertex_spirv_file.copy_all_to_buffer((u8*)spirv_buffer)
        , "failed to copy vertex spirv file to buffer"
        , PipelineCreation<PipelineType::Graphics>{}
        );

      VkShaderModuleCreateInfo shader_create_info
      { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
      , nullptr
      , 0
      , spirv_size
      , spirv_buffer
      };

      VkShaderModule   mod;
      VK_RETURN_VAL(_context->get_device().get_device_function_table()
        .vkCreateShaderModule(_context->get_device().get_device()
            , &shader_create_info
            , CALLBACKS()
            , &mod
            )
        , "failed to create vertex shader module"
        , PipelineCreation<PipelineType::Graphics>{});

      creation.set_vertex_shader(mod);

      RX_TRACE("closing vertex file");
      if(spirv_size > 0)
        FREE(spirv_buffer);
      spirv_buffer = nullptr;
      spirv_size = 0;
      if(vertex_spirv_file.is_open())
        vertex_spirv_file.close();

      RX_TRACE("opening fragment file");
      File fragment_spirv_file = resource::shader_spirv_file(fragment_shader_index);
      spirv_size = fragment_spirv_file.get_size();
      RX_TRACEF("fragment spirv size = %llu", spirv_size);

      spirv_buffer = (u32*)ALLOCATE(spirv_size);

      RX_RETURN(fragment_spirv_file.copy_all_to_buffer(spirv_buffer)
        , "failed to copy fragment spirv file to buffer"
        , PipelineCreation<PipelineType::Graphics>{}
        );

      shader_create_info.codeSize = spirv_size;
      shader_create_info.pCode = spirv_buffer;

      VK_RETURN_VAL(_context->get_device().get_device_function_table()
        .vkCreateShaderModule(_context->get_device().get_device()
            , &shader_create_info
            , CALLBACKS()
            , &mod
            )
        , "failed to create fragment shader module"
        , PipelineCreation<PipelineType::Graphics>{});

      creation.set_fragment_shader(mod);

      if(spirv_size > 0)
        FREE(spirv_buffer);

      spirv_buffer = nullptr;

      if(fragment_spirv_file.is_open())
        fragment_spirv_file.close();

      RX_TRACE("returning from pipeline create");
      return creation;
    }
  
    PipelineCreation<PipelineType::Compute> PipelinePoolBuilder::add_compute_pipeline(const PipelineInfo info) {
      PipelineCreation<PipelineType::Compute> creation{};
      RX_RETURN(creation.set_dispatch_type(info.type)
        , "failed to set correct dispatch type when adding compute pipeline"
        , PipelineCreation<PipelineType::Compute>{}
        );
        
      const u64 shader_index = resource::helpers::find_shader_index_from_shader_name((const char*)info.compute.shader_name);
      const VkShaderStageFlags stage_flags = resource::shader_stage(shader_index);
      VkDescriptorSetLayoutBinding binding{};
      binding.stageFlags = stage_flags;

      File spirv_file = resource::shader_spirv_file(shader_index);
      const u64 spirv_size = spirv_file.get_size();

      u32* spirv_buffer = (u32*)ALLOCATE(spirv_size);

      RX_RETURN(spirv_file.copy_all_to_buffer((u8*)spirv_buffer)
        , "failed to copy spirv file to buffer"
        , PipelineCreation<PipelineType::Compute>{}
        );

      VkShaderModuleCreateInfo shader_create_info
      { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
      , nullptr
      , 0
      , spirv_file.get_size()
      , spirv_buffer
      };
 
      VkShaderModule mod;
 
      VK_RETURN_VAL(_context->get_device().get_device_function_table()
        .vkCreateShaderModule(_context->get_device().get_device()
            , &shader_create_info
            , CALLBACKS()
            , &mod
            )
        , "failed to create compute shader module"
        , PipelineCreation<PipelineType::Compute>{});
 
      creation.set_shader(mod);

      FREE((void*)spirv_buffer);
      spirv_file.close();

      return creation;
    }

    const PipelinePool::PipelineHandle PipelinePoolBuilder::add_pipeline(const PipelineInfo info) {
        if (get_pipeline_type_from_dispatch_type(info.type) == PipelineType::Graphics) {
          RX_TRACEF("vertex shader name in add_pipeline = %s", info.graphics.vertex_shader_name);
          const auto result = _graphics_pipelines.get_size();
          *(_graphics_pipelines.push(1)) = add_graphics_pipeline(info);
          return result;
        } else if (get_pipeline_type_from_dispatch_type(info.type) == PipelineType::Compute) {
          const auto result = _compute_pipelines.get_size();
          *(_compute_pipelines.push(1)) = add_compute_pipeline(info);
          return result;
        }
        return PipelinePool::PipelineHandleMax;
      };

    const PipelinePool::RenderPassHandle PipelinePoolBuilder::add_render_pass(const u32 attachment_count, AttachmentInfo* colour_attachments, AttachmentInfo depth_attachment) {
      const PipelinePool::RenderPassHandle result = _render_passes.get_size();
      RenderPassBuilder builder;
      builder.reset();
      for(u32 i = 0; i < attachment_count; i++) {
        builder.add_attachment(colour_attachments[i].format, colour_attachments[i].layout, colour_attachments[i].load_op);
      }
      builder.set_depth_stencil_operations(depth_attachment.load_op, RenderPassOperationType::Clear);
      builder.set_depth_stencil_texture(depth_attachment.format, depth_attachment.layout);
      builder.build(_context, _render_passes.push(1));
      return result;
    }


    b8 PipelinePoolBuilder::init(Context* context) {

      _context = context;

      RX_TRACE("allocating pipeline pool builder buffer");
      u8* buffer = (u8*)ALLOCATE(
            sizeof(RenderPass) * s_max_render_passes
          + sizeof(PipelineInfo) * s_max_pipelines
          + sizeof(PipelineInfo) * s_max_pipelines
          + sizeof(SpecializationInfo) * 16
          );

      RX_TRACE("moving render pass builder pointer");
      _render_passes.move_ptr(buffer);
      _render_passes.clear();
      buffer += sizeof(RenderPass) * s_max_render_passes;

      RX_TRACE("moving graphics pipelines pointer");
      _graphics_pipelines.move_ptr(buffer);
      _graphics_pipelines.clear();
      buffer += sizeof(PipelineInfo) * s_max_pipelines;

      RX_TRACE("moving compute pipelines pointer");
      _compute_pipelines.move_ptr(buffer);
      _compute_pipelines.clear();
      buffer += sizeof(PipelineInfo) * s_max_pipelines;

      RX_TRACE("moving spec infos pointer");
      _specialization_infos.move_ptr(buffer);
      _specialization_infos.clear();

      return true;
    }
     

    b8 PipelinePoolBuilder::build(PipelinePool* pool) {
      const u32 render_pass_count = _render_passes.get_size();
      const u32 specialization_constant_count = _specialization_infos.get_size();
      const u32 graphics_pipeline_count = _graphics_pipelines.get_size();
      const u32 compute_pipeline_count = _compute_pipelines.get_size();
      const u32 pipeline_count = graphics_pipeline_count + compute_pipeline_count;
      RX_TRACE("building pipeline");
      pool->_pipelines.clear();

      pool->_pipelines.move_ptr(ALLOCATE(sizeof(Pipeline) * (pipeline_count)));
      Pipeline* pipeline_begin = pool->_pipelines.push(pipeline_count);
      DispatchType* dispatch_type_buffer = (DispatchType*)ALLOCATE(sizeof(DispatchType) * pipeline_count);
      void* buffer_base_ptr = ALLOCATE(
            sizeof(VkSpecializationMapEntry) * specialization_constant_count
          + sizeof(VkGraphicsPipelineCreateInfo) * graphics_pipeline_count
          + sizeof(VkComputePipelineCreateInfo) * compute_pipeline_count
          );

      u8* buffer = (u8*)buffer_base_ptr;
   
      pool->_render_passes.clear();
      pool->_render_passes.move_ptr(ALLOCATE(sizeof(RenderPass) * render_pass_count));
      RenderPass* const render_pass_begin = pool->_render_passes.push(render_pass_count);
      RX_TRACEF("building %u render passes", render_pass_count);

      for(u32 i = 0; i < render_pass_count; i++) {
        RX_TRACEF("copying over renderpass %u = %llu", i, PTR2INT(_render_passes[i].get_render_pass()));
        render_pass_begin[i] = _render_passes[i];
      }

      RX_TRACE("creating descriptor set layout bindings");
      VkDescriptorSetLayoutBinding binding{};
      binding.stageFlags = VK_SHADER_STAGE_ALL;
      binding.descriptorCount = RoxiBindlessDescriptorCount;
      // ubo, storage buffer, texture, storage image
      const u32 set_count = 4;
      DescriptorSetLayoutCreation descriptor_creation{};
      RX_TRACE("initializing descriptor layout creation");
      descriptor_creation.init();
      descriptor_creation.set_flags(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT
        // ?? | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
      );
      RX_TRACEF("creating descriptors, set count = %u", set_count);
      for(u32 i = 0; i < set_count; i++) {
        binding.descriptorType = get_descriptor_type((vk::DescriptorBufferType)i);
        binding.binding = 0;
        // all descriptor buffers are device side, but the buffers themselves can be host side
        // indexing is handled by a combination of push constant and draw param uniform buffers
        descriptor_creation.add_binding(binding);
        RX_TRACEF("creating descriptor %u", i);
        pool->_descriptor_set_layouts[i].init(_context, descriptor_creation);
        descriptor_creation.clear_bindings();
      }

      RX_TRACE("creating specialization infos");
      // one binding per set
      Array<VkSpecializationMapEntry> specialization_infos{buffer};
      buffer += sizeof(VkSpecializationMapEntry) * specialization_constant_count;
      VkSpecializationMapEntry* specialization_begin = specialization_infos.push(specialization_constant_count);
      u32 offset = 0;
      for(u32 i = 0; i < specialization_constant_count; i++) {
        specialization_infos[i].constantID = _specialization_infos[i].id;
        specialization_infos[i].offset = offset;
        const u32 current_size = _specialization_infos[i].size;
        specialization_infos[i].size = current_size;
        offset += current_size;
      }
      void* spec_data_ptr = ALLOCATE(offset);
      for(u32 i = 0; i < specialization_constant_count; i++) {
        MEM_COPY(((u8*)spec_data_ptr + specialization_infos[i].offset), _specialization_infos[i].data, specialization_infos[i].size);
      }

      VkSpecializationInfo spec_info{};
      spec_info.dataSize = offset;
      spec_info.pData = spec_data_ptr;
      spec_info.mapEntryCount = specialization_constant_count;
      spec_info.pMapEntries = specialization_begin;

      RX_TRACE("creating push constant range");
      VkPushConstantRange pc_range{};
      pc_range.offset = 0;
      pc_range.size = sizeof(pc::FrameParams);
      pc_range.stageFlags = VK_SHADER_STAGE_ALL;

      RX_TRACE("creating pipeline layout");
      VkPipelineLayout pipeline_layout = create_pipeline_layout(_context, pool->_descriptor_set_layouts, set_count, &pc_range, 1);

      Array<VkGraphicsPipelineCreateInfo> 
        graphics_create_infos{buffer};
      VkGraphicsPipelineCreateInfo* graphics_info_begin = graphics_create_infos.push(graphics_pipeline_count);
      buffer += sizeof(VkGraphicsPipelineCreateInfo) * graphics_pipeline_count;

      Array<VkComputePipelineCreateInfo> 
        compute_create_infos{buffer};
      VkComputePipelineCreateInfo* compute_info_begin = compute_create_infos.push(graphics_pipeline_count);
      buffer += sizeof(VkComputePipelineCreateInfo) * compute_pipeline_count;

      pc_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

      RX_TRACE("creating pipelines");
      for(u64 i = 0; i < compute_pipeline_count; i++) {
        PipelineCreation<PipelineType::Compute> creation = _compute_pipelines[i];
        RX_CHECK(creation.get_dispatch_type() != DispatchType::Max
          , "failed to create compute pipeline creation"
          );
        creation.set_spec_info(spec_info);
        dispatch_type_buffer[i] = creation.get_dispatch_type();
        compute_info_begin[i] = creation.get_create_info();
      }

      pc_range.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

      VkPipelineCacheCreateInfo cache_create_info{};
      cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
      cache_create_info.initialDataSize = 0;
      cache_create_info.flags = 0;

      _context->get_device().get_device_function_table()
        .vkCreatePipelineCache(_context->get_device().get_device()
            , &cache_create_info, CALLBACKS(), &pool->_pipeline_cache);

      for(u64 i = 0; i < graphics_pipeline_count; i++) {
        PipelineCreation<PipelineType::Graphics>& creation = _graphics_pipelines[i];
        RX_CHECK(creation.get_dispatch_type() != DispatchType::Max
          , "failed to create graphics pipeline creation"
          );
        const u32 render_pass_handle = _graphics_pipelines[i].get_render_pass_handle();
        creation.set_pipeline_layout(pipeline_layout);
        creation.set_spec_info(spec_info);
        if(render_pass_handle != MAX_u32) {
          creation.set_render_pass(pool->_render_passes[render_pass_handle].get_render_pass());
        }
        dispatch_type_buffer[i + compute_pipeline_count] = _graphics_pipelines[i].get_dispatch_type();
        graphics_info_begin[i] = creation.get_create_info();
      }

      RX_CHECK
        ( create_pipelines(pipeline_begin, graphics_pipeline_count, compute_pipeline_count, graphics_create_infos.get_buffer(), compute_create_infos.get_buffer(), dispatch_type_buffer, pool->_pipeline_cache)
        , "failed to create pipelines in PipelinePoolBuilder::build(PipelinePool*)"
        );

      RX_TRACE("freeing data");
      if(specialization_constant_count > 0) {
        RX_TRACEF("freeing spec constant data, count = %u", specialization_constant_count);
        FREE(spec_data_ptr);
      }
      if((render_pass_count > 0) 
        || (specialization_constant_count > 0)
        || (pipeline_count > 0)) {
        RX_TRACE("freeing buffer");
        FREE(buffer_base_ptr);
        FREE(dispatch_type_buffer);
      }

      RX_TRACE("returning from pipeline pool build");
      return true;
    }

    b8 PipelinePoolBuilder::terminate() {
      FREE(_render_passes.get_buffer());
      return true;
    }

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
