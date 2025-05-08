// =====================================================================================
//
//       Filename:  vk_renderpass.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-09 1:25:27 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"
#include "vk_resource.hpp"
#include <vulkan/vulkan_core.h>

#define RENDER_PASS_OPERATION_TYPES(X) X(DontCare) X(Load) X(Clear) X(MaxType)

namespace roxi {

   enum class RenderPassOperationType {
#define ENUM(priority) priority, 
    RENDER_PASS_OPERATION_TYPES(ENUM)
#undef ENUM
  };

  static const char* get_render_pass_operation_type_string(RenderPassOperationType type) {
#define STRING(X) if (type == RenderPassOperationType::X) return STRINGIFY(X);
    RENDER_PASS_OPERATION_TYPES(STRING)
#undef STRING
      else
        return "";
  }

 
  namespace vk {

    static constexpr u8 s_max_image_outputs = 8;
    static constexpr u8 s_max_render_passes = 8;
    static constexpr u8 s_max_framebuffers = RoxiNumFrames;

    struct AttachmentInfo {
      VkFormat format;
      VkImageLayout layout;
      RenderPassOperationType load_op = RenderPassOperationType::MaxType;
    };

    struct RenderPassInfo {
      SizedStackArray<AttachmentInfo, s_max_image_outputs> colour_attachments;
      AttachmentInfo depth_attachment;
    };

    //
    //
    struct RenderPassOutput {
    
        VkFormat                        color_formats[ s_max_image_outputs ];
        VkImageLayout                   color_final_layouts[ s_max_image_outputs ];
        RenderPassOperationType         color_operations[ s_max_image_outputs ];
    
        VkFormat                        depth_stencil_format;
        VkImageLayout                   depth_stencil_final_layout;
    
        u32                             num_color_formats;
    
        RenderPassOperationType       depth_operation         = RenderPassOperationType::DontCare;
        RenderPassOperationType       stencil_operation       = RenderPassOperationType::DontCare;
    
        RenderPassOutput&               reset();
        RenderPassOutput&               color( VkFormat format, VkImageLayout layout, RenderPassOperationType load_op);
        RenderPassOutput&               depth( VkFormat format, VkImageLayout layout );
        RenderPassOutput&               set_depth_stencil_operations( RenderPassOperationType depth, RenderPassOperationType stencil );
    
    }; // struct RenderPassOutput
    
    //
    //
    class RenderPassBuilder;
    //
    //
    class RenderPass {
    private: 
      friend class RenderPassBuilder;
        // NOTE(marco): this will be a null handle if dynamic rendering is available
        VkRenderPass                    _render_pass;
    
        RenderPassOutput                _output;
    
        u16                             _dispatch_x  = 0;
        u16                             _dispatch_y  = 0;
        u16                             _dispatch_z  = 0;
    
        u8                              _num_render_targets = 0;

    public:
      RenderPass(const RenderPass& pass) {
        *this = pass;
      }

      RenderPass& operator=(const RenderPass& pass) {
        _render_pass = pass._render_pass;
        _output = pass._output;
        _dispatch_x = pass._dispatch_x;
        _dispatch_y = pass._dispatch_y;
        _dispatch_z = pass._dispatch_z;
        _num_render_targets = pass._num_render_targets;
        return *this;
      }

      const VkRenderPass& get_render_pass() const {
        return _render_pass;
      }

      b8 terminate(Context* context) {
        if(_render_pass == VK_NULL_HANDLE)
          return false;
        context->get_device().get_device_function_table()
          .vkDestroyRenderPass(context->get_device().get_device()
            , _render_pass
            , CALLBACKS());
        _render_pass = VK_NULL_HANDLE;
        return true;
      }
    }; // struct RenderPass
    
    //
    //
    class FramebufferCreation {
    private:
        u16                             _num_render_targets  = 0;
        const RenderPass*                  _render_pass;
    
        ResourcePool::ImageHandle                   _output_textures[ s_max_image_outputs ];
        ResourcePool::ImageHandle              _depth_stencil_texture = { ResourcePool::MaxResourceHandle };
    
        u16                             _width       = 0;
        u16                             _height      = 0;
    
        f32                             _scale_x             = 1.f;
        f32                             _scale_y             = 1.f;
        u8                              _resize              = 1;

    public:
        FramebufferCreation&            reset();
        FramebufferCreation&            add_render_texture( ResourcePool::ImageHandle texture );
        FramebufferCreation&            set_depth_stencil_texture( ResourcePool::ImageHandle texture );
        FramebufferCreation&            set_scaling( f32 scale_x, f32 scale_y, u8 resize );
        FramebufferCreation&            set_extent( u16 width, u16 height );
        FramebufferCreation&            set_render_pass( const RenderPass* render_pass );

        VkFramebufferCreateInfo get_create_info(ResourcePool& resource_pool) const;
    
    }; // struct RenderPassCreation
    
    //
    //
    class Framebuffer {
    private: 
      // NOTE(marco): this will be a null handle if dynamic rendering is available
      VkFramebuffer                   _framebuffer;
    
      // NOTE(marco): cache render pass handle
      //RenderPassHandle                render_pass;
    
      u16                             _width       = 0;
      u16                             _height      = 0;
    
      f32                             _scale_x     = 1.f;
      f32                             _scale_y     = 1.f;
    
      ResourcePool::ImageHandle       _color_attachments[ s_max_image_outputs ];
      ResourcePool::ImageHandle       _depth_stencil_attachment;
      u32                             _num_color_attachments;

    public:
      b8 init(Context* context, const FramebufferCreation& creation, ResourcePool& resource_pool);

      const VkFramebuffer& get_framebuffer() const {
        return _framebuffer;
      }
    
    }; // struct Framebuffer
    
    class RenderPassBuilder {
    private: 
        u16                             _num_render_targets  = 0;
    
        VkFormat                        _color_formats[ s_max_image_outputs ];
        VkImageLayout                   _color_final_layouts[ s_max_image_outputs ];
        RenderPassOperationType         _color_operations[ s_max_image_outputs ];
    
        VkFormat                        _depth_stencil_format = VK_FORMAT_UNDEFINED;
        VkImageLayout                   _depth_stencil_final_layout;
    
        RenderPassOperationType         _depth_operation         = RenderPassOperationType::DontCare;
        RenderPassOperationType         _stencil_operation       = RenderPassOperationType::DontCare;

    const RenderPassOutput fill_render_pass_output() const;
    public:
        RenderPassBuilder&             reset();
        RenderPassBuilder&             add_attachment( VkFormat format, VkImageLayout layout, RenderPassOperationType load_op );
        RenderPassBuilder&             set_depth_stencil_texture( VkFormat format, VkImageLayout layout );
        RenderPassBuilder&             set_depth_stencil_operations( RenderPassOperationType depth, RenderPassOperationType stencil );

        b8 build(Context* context, RenderPass* render_pass) const;
    
    }; // struct RenderPassCreation
    

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
