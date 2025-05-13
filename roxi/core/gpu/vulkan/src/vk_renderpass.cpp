// =====================================================================================
//
//       Filename:  vk_renderpass.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-11-21 9:37:12 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_allocator.hpp"
#include "vk_renderpass.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {
  namespace vk {

    const RenderPassOutput RenderPassBuilder::fill_render_pass_output() const {
        RenderPassOutput output;
        output.reset();
   
        for ( u32 i = 0; i < _num_render_targets; ++i ) {
            output.color( _color_formats[ i ], _color_final_layouts[ i ], _color_operations[ i ] );
        }
        if ( _depth_stencil_format != VK_FORMAT_UNDEFINED ) {
            output.depth( _depth_stencil_format, _depth_stencil_final_layout );
        }
    
        output.depth_operation = _depth_operation;
        output.stencil_operation = _stencil_operation;
    
        return output;
    }

    static VkRenderPassCreateInfo vulkan_create_render_pass_info( const RenderPassOutput& output ) {
      VkAttachmentDescription color_attachments[ 8 ] = {};
      VkAttachmentReference color_attachments_ref[ 8 ] = {};
  
      VkAttachmentLoadOp depth_op, stencil_op;
      VkImageLayout depth_initial;
  
      switch ( output.depth_operation ) {
        case RenderPassOperationType::Load:
          depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
          depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
          break;
        case RenderPassOperationType::Clear:
          depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
          depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
          break;
        default:
          depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
          break;
      }
  
      switch ( output.stencil_operation ) {
        case RenderPassOperationType::Load:
          stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
          break;
        case RenderPassOperationType::Clear:
          stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
          break;
        default:
          stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          break;
      }
  
      // Color attachments
      u32 c = 0;
      for ( ; c < output.num_color_formats; ++c ) {
        VkAttachmentLoadOp color_op;
        VkImageLayout color_initial;
        switch ( output.color_operations[ c ] ) {
          case RenderPassOperationType::Load:
            color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
          case RenderPassOperationType::Clear:
            color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
          default:
            color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        }
  
        VkAttachmentDescription& color_attachment = color_attachments[ c ];
        color_attachment.format = output.color_formats[ c ];
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = color_op;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = stencil_op;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = color_initial;
        color_attachment.finalLayout = output.color_final_layouts[ c ];
  
        VkAttachmentReference& color_attachment_ref = color_attachments_ref[ c ];
        color_attachment_ref.attachment = c;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }
  
      // Depth attachment
      VkAttachmentDescription depth_attachment{};
      VkAttachmentReference depth_attachment_ref{};
  
      if(output.depth_stencil_format != VK_FORMAT_UNDEFINED) {
  
        depth_attachment.format = output.depth_stencil_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = depth_op;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = stencil_op;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = depth_initial;
        depth_attachment.finalLayout = output.depth_stencil_final_layout;
  
        depth_attachment_ref.attachment = c;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      }
  
      // Create subpass.
      // TODO: for now is just a simple subpass, evolve API.
      VkSubpassDescription subpass = {};
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  
      // Calculate active attachments for the subpass
      VkAttachmentDescription attachments[ s_max_image_outputs + 1 ]{};
      for ( u32 active_attachments = 0; active_attachments < output.num_color_formats; ++active_attachments ) {
        attachments[ active_attachments ] = color_attachments[ active_attachments ];
      }
      subpass.colorAttachmentCount = output.num_color_formats;
      subpass.pColorAttachments = color_attachments_ref;
  
      subpass.pDepthStencilAttachment = nullptr;
  
      u32 depth_stencil_count = 0;
      if ( output.depth_stencil_format != VK_FORMAT_UNDEFINED ) {
        attachments[ subpass.colorAttachmentCount ] = depth_attachment;
  
        subpass.pDepthStencilAttachment = &depth_attachment_ref;
  
        depth_stencil_count = 1;
      }
  
      VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  
      render_pass_info.attachmentCount = ( output.num_color_formats ) + depth_stencil_count;
      render_pass_info.pAttachments = attachments;
      render_pass_info.subpassCount = 1;
      render_pass_info.pSubpasses = &subpass;
  
      // Create external subpass dependencies
      //VkSubpassDependency external_dependencies[ 16 ];
      //u32 num_external_dependencies = 0;
  
      return render_pass_info;
    }
    FramebufferCreation& FramebufferCreation::set_extent( u16 width, u16 height ) {
      _width = width;
      _height = height;
      return *this;
    }

    VkFramebufferCreateInfo FramebufferCreation::get_create_info(ResourcePool& resource_pool) const {
      Array<VkImageView> image_views{ALLOCATE(sizeof(VkImageView) * _num_render_targets + 1)};
      image_views.clear();
      VkImageView* const image_view_begin = image_views.push(_num_render_targets + 1);
      VkFramebufferCreateInfo result{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
      result.pNext = nullptr;
      result.flags = 0;
      result.renderPass = _render_pass->get_render_pass();
      result.width = _width;
      result.height = _height;
      result.layers = 1;

      RX_TRACEF("getting image view count %u", _num_render_targets);
      for(u32 i = 0; i < _num_render_targets; i++) {
        RX_TRACEF("getting image view %u", i);
        image_view_begin[i] = resource_pool.obtain_image(_output_textures[i]).get_image_view();
      }
      b8 depth_texture_avail = _depth_stencil_texture != ResourcePool::MaxResourceHandle;
      if(depth_texture_avail) {
        image_views[_num_render_targets] = resource_pool.obtain_image(_depth_stencil_texture).get_image_view();
      }

      result.attachmentCount = depth_texture_avail ? _num_render_targets + 1 : _num_render_targets;
      result.pAttachments = image_views.get_buffer();

      return result;
    }

// RenderPassOutput ///////////////////////////////////////////////////////
RenderPassOutput& RenderPassOutput::reset() {
    num_color_formats = 0;
    for ( u32 i = 0; i < s_max_image_outputs; ++i) {
        color_formats[ i ] = VK_FORMAT_UNDEFINED;
        color_final_layouts[ i ] = VK_IMAGE_LAYOUT_UNDEFINED;
        color_operations[ i ] = RenderPassOperationType::DontCare;
    }
    depth_stencil_format = VK_FORMAT_UNDEFINED;
    depth_operation = stencil_operation = RenderPassOperationType::DontCare;
    return *this;
}

RenderPassOutput& RenderPassOutput::color( VkFormat format, VkImageLayout layout, RenderPassOperationType load_op ) {
    color_formats[ num_color_formats ] = format;
    color_operations[ num_color_formats ] = load_op;
    color_final_layouts[ num_color_formats++ ] = layout;
    return *this;
}

RenderPassOutput& RenderPassOutput::depth( VkFormat format, VkImageLayout layout ) {
    depth_stencil_format = format;
    depth_stencil_final_layout = layout;
    return *this;
}

RenderPassOutput& RenderPassOutput::set_depth_stencil_operations( RenderPassOperationType depth_, RenderPassOperationType stencil_ ) {
    depth_operation = depth_;
    stencil_operation = stencil_;

    return *this;
}


// RenderPassCreation /////////////////////////////////////////////////////
RenderPassBuilder& RenderPassBuilder::reset() {
    _num_render_targets = 0;
    _depth_stencil_format = VK_FORMAT_UNDEFINED;
    for ( u32 i = 0; i < s_max_image_outputs; ++ i ) {
        _color_operations[ i ] = RenderPassOperationType::DontCare;
    }
    _depth_operation = _stencil_operation = RenderPassOperationType::DontCare;

    return *this;
}

RenderPassBuilder& RenderPassBuilder::add_attachment( VkFormat format, VkImageLayout layout, RenderPassOperationType load_op ) {
    _color_formats[ _num_render_targets ] = format;
    _color_operations[ _num_render_targets ] = load_op;
    _color_final_layouts[ _num_render_targets++ ] = layout;

    return *this;
}

RenderPassBuilder& RenderPassBuilder::set_depth_stencil_texture( VkFormat format, VkImageLayout layout ) {
    _depth_stencil_format = format;
    _depth_stencil_final_layout = layout;

    return *this;
}

RenderPassBuilder& RenderPassBuilder::set_depth_stencil_operations( RenderPassOperationType depth, RenderPassOperationType stencil ) {
    _depth_operation = depth;
    _stencil_operation = stencil;

    return *this;
}

b8 RenderPassBuilder::build(Context* context, RenderPass* render_pass) const {

  RX_TRACEF("inside build render pass num render targets = %u", _num_render_targets);

  RX_TRACE("filling render pass output");
  render_pass->_output = fill_render_pass_output();

  VkAttachmentDescription color_attachments[ 8 ] = {};
  VkAttachmentReference color_attachments_ref[ 8 ] = {};

  VkAttachmentLoadOp depth_op, stencil_op;
  VkImageLayout depth_initial;

  RX_TRACE("switching on depth op");
  switch ( render_pass->_output.depth_operation ) {
      case RenderPassOperationType::Load:
        RX_TRACE("type load");
          depth_op = VK_ATTACHMENT_LOAD_OP_LOAD;
          depth_initial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
          break;
      case RenderPassOperationType::Clear:
        RX_TRACE("type clear");
          depth_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
          depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
          break;
      default:
        RX_TRACE("default (don't care)");
          depth_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          depth_initial = VK_IMAGE_LAYOUT_UNDEFINED;
          break;
  }

  RX_TRACE("switching on stencil op");
  switch ( render_pass->_output.stencil_operation ) {
      case RenderPassOperationType::Load:
        RX_TRACE("type load");
          stencil_op = VK_ATTACHMENT_LOAD_OP_LOAD;
          break;
      case RenderPassOperationType::Clear:
        RX_TRACE("type clear");
          stencil_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
          break;
      default:
        RX_TRACE("default (don't care)");
          stencil_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          break;
  }

  // Color attachments
  u32 c = 0;
  const u32 colour_format_count = _num_render_targets;
  render_pass->_output.num_color_formats = colour_format_count;
  RX_TRACEF("creating colour attachment count = %u", colour_format_count);
  for ( ; c < colour_format_count; ++c ) {
    RX_TRACEF("creating attachment %u", c);
      VkAttachmentLoadOp color_op;
      VkImageLayout color_initial;
      switch ( render_pass->_output.color_operations[ c ] ) {
          case RenderPassOperationType::Load:
              color_op = VK_ATTACHMENT_LOAD_OP_LOAD;
              color_initial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
              break;
          case RenderPassOperationType::Clear:
              color_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
              color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
              break;
          default:
              color_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
              color_initial = VK_IMAGE_LAYOUT_UNDEFINED;
              break;
      }

      VkImageLayout final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      VkAttachmentDescription& color_attachment = color_attachments[ c ];
      color_attachment.format = render_pass->_output.color_formats[ c ];
      color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      color_attachment.loadOp = color_op;
      color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color_attachment.stencilLoadOp = stencil_op;
      color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      color_attachment.initialLayout = color_initial;
      color_attachment.finalLayout = final_layout;
      
      render_pass->_output.color_final_layouts[ c ] = final_layout;
      VkAttachmentReference& color_attachment_ref = color_attachments_ref[ c ];
      color_attachment_ref.attachment = c;
      color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  // Depth attachment
  VkAttachmentDescription depth_attachment{};
  VkAttachmentReference depth_attachment_ref{};

  if ( render_pass->_output.depth_stencil_format != VK_FORMAT_UNDEFINED ) {

    RX_TRACE("depth stencil format set");
      depth_attachment.format = render_pass->_output.depth_stencil_format;
      depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      depth_attachment.loadOp = depth_op;
      depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      depth_attachment.stencilLoadOp = stencil_op;
      depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depth_attachment.initialLayout = depth_initial;
      depth_attachment.finalLayout = render_pass->_output.depth_stencil_final_layout;

      depth_attachment_ref.attachment = c;
      depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  // Create subpass.
  // TODO: for now is just a simple subpass, evolve API.
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Calculate active attachments for the subpass
  VkAttachmentDescription attachments[ s_max_image_outputs + 1 ]{};
  RX_TRACE("setting attachments for subpass");
  for ( u32 active_attachments = 0; active_attachments < render_pass->_output.num_color_formats; ++active_attachments ) {
      attachments[ active_attachments ] = color_attachments[ active_attachments ];
  }
  subpass.colorAttachmentCount = render_pass->_output.num_color_formats;
  subpass.pColorAttachments = color_attachments_ref;

  subpass.pDepthStencilAttachment = nullptr;

  u32 depth_stencil_count = 0;
  if ( render_pass->_output.depth_stencil_format != VK_FORMAT_UNDEFINED ) {
      attachments[ subpass.colorAttachmentCount ] = depth_attachment;

      subpass.pDepthStencilAttachment = &depth_attachment_ref;

      depth_stencil_count = 1;
  }

  VkRenderPassCreateInfo render_pass_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

  render_pass_info.attachmentCount = ( render_pass->_output.num_color_formats ) + depth_stencil_count;
  render_pass_info.pAttachments = attachments;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  // Create external subpass dependencies
  //VkSubpassDependency external_dependencies[ 16 ];
  //u32 num_external_dependencies = 0;
 
  RX_TRACE("vkCreateRenderPass...");
  VK_CHECK(context->get_device().get_device_function_table()
    .vkCreateRenderPass(context->get_device().get_device()
      , &render_pass_info, CALLBACKS(), &render_pass->_render_pass)
    , "failed to build RenderPass in RenderPassBuilder::build(Context*, RenderPass*)"
    );

  return true;
}

    // FramebufferCreation ////////////////////////////////////////////////////
    FramebufferCreation& FramebufferCreation::reset()
    {
        _num_render_targets = 0;
        _depth_stencil_texture = ResourcePool::MaxResourceHandle;
        _render_pass = nullptr;
    
        _resize = 0;
        _scale_x = 1.f;
        _scale_y = 1.f;
    
        return *this;
    }
    
    FramebufferCreation& FramebufferCreation::add_render_texture( ResourcePool::ImageHandle texture )
    {
        _output_textures[ _num_render_targets++ ] = texture;
    
        return *this;
    }
    
    FramebufferCreation& FramebufferCreation::set_depth_stencil_texture( ResourcePool::ImageHandle texture )
    {
        _depth_stencil_texture = texture;
    
        return *this;
    }
    
    FramebufferCreation& FramebufferCreation::set_render_pass( const RenderPass* render_pass )
    {
      RX_TRACEF("setting render pass at %llu", PTR2INT(render_pass->get_render_pass()));
      _render_pass = render_pass;
    
      return *this;
    }
    
    FramebufferCreation& FramebufferCreation::set_scaling( f32 scale_x_, f32 scale_y_, u8 resize_ ) {
        _scale_x = scale_x_;
        _scale_y = scale_y_;
        _resize = resize_;
    
        return *this;
    }

    b8 Framebuffer::init(Context* context, const FramebufferCreation& creation, ResourcePool& resource_pool) {
      RX_TRACE("getting create info");
      VkFramebufferCreateInfo info = creation.get_create_info(resource_pool);
      RX_TRACE("vkCreateFramebuffer");
      VK_CHECK(context->get_device().get_device_function_table()
        .vkCreateFramebuffer(context->get_device().get_device()
          , &info
          , CALLBACKS()
          , &_framebuffer
          )
        , "failed to create vk frame buffer!");

      if(info.attachmentCount > 0) {
        FREE((void*)info.pAttachments);
      }
      return true;
    }


  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
