// =====================================================================================
//
//       Filename:  vk_image.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-04 1:59:02 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_allocator.hpp"
#include <vulkan/vulkan_core.h>

#define VK_IMAGE_TYPES(X) X(Texture) X(InputAttachment) X(RenderTarget) X(DepthTarget) X(StorageImage) X(Reference)
#define SAMPLER_WRAP_TYPES(X) X(Wrap) X(Mirror) X(ClampEdge) X(ClampBorder)

namespace roxi {


  namespace gpu {
#define ENUM(Type) Type, 
    enum class ImageType : u8 {
      VK_IMAGE_TYPES(ENUM)
      Max
    };
#undef ENUM
    static const char* get_vk_image_type_name(ImageType type) {
#define STR(TypeT) if (type == ImageType::TypeT) return (const char*)STRINGIFY(TypeT);
      VK_IMAGE_TYPES(STR)
#undef STR
        else
          return "";
    }

  }		// -----  end of namespace gpu  ----- 
  
  namespace vk {

    static VkImageUsageFlags get_image_usage_flags(const gpu::ImageType type) {
      static const VkImageUsageFlags _s_flags[] = 
        { VK_IMAGE_USAGE_SAMPLED_BIT
        , VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        , VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        , VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        , 0
        , VK_IMAGE_USAGE_STORAGE_BIT
        };
      return _s_flags[(u8)type];
    }

    static VkFormat get_image_format(const gpu::ImageType type) {
      static const VkFormat _s_flags[] = 
        { VK_FORMAT_R32G32B32A32_SFLOAT
        , VK_FORMAT_UNDEFINED
        , VK_FORMAT_R32G32B32A32_SFLOAT
        , VK_FORMAT_R32_SFLOAT
        , VK_FORMAT_UNDEFINED
        , VK_FORMAT_UNDEFINED
        };
      return _s_flags[(u8)type];
    }

    enum class SampleWrapType {
#define ENUM(wrap_t) wrap_t,
      SAMPLER_WRAP_TYPES(ENUM)
#undef ENUM
   };

    static const char* get_sampler_wrap_type_string(SampleWrapType type) {
#define STRING(X) if (type == SampleWrapType::X) return STRINGIFY(X);
      SAMPLER_WRAP_TYPES(STRING)
#undef STRING
        else
          return "";
    }

    static const VkSamplerAddressMode get_sampler_address_mode(SampleWrapType type) {
      switch(type) {
        case SampleWrapType::Wrap       : return          VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SampleWrapType::ClampBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SampleWrapType::ClampEdge  : return   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SampleWrapType::Mirror     : return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default                         : return        VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
      }
    }

    static constexpr u8 RoxiImageBitDepth = 4;

    static constexpr u32 s_max_input_attachments = 256;
    static constexpr u32 s_max_render_targets    =  64;
    static constexpr u32 s_max_depth_buffers     =  16;
    static constexpr u32 s_max_textures          = 256;
    static constexpr u32 s_max_storage_images    = 256;

    static constexpr u32 s_max_images            
      = s_max_input_attachments 
      + s_max_render_targets 
      + s_max_depth_buffers 
      + s_max_textures
      + s_max_storage_images;

    template<u8 Dimensions>
    struct Extent;

    template<>
    struct Extent<2> {
      using type = VkExtent2D;
      type value;
    };

    template<>
    struct Extent<3> {
      using type = VkExtent3D;
      type value;
    };

    class Image {
    private:
      Context* _context = nullptr;
      VkImage _image = VK_NULL_HANDLE;
      VkExtent3D _image_extent{};
      gpu::ImageType _type;
      VkImageView _image_view = VK_NULL_HANDLE;
      VkImageLayout _image_layout{};
      mem::Allocation _current_allocation{};
      u32 _alignment = 0;
     
    public:
      b8 init
        ( Context* context
        , VkExtent3D extent
        , const gpu::ImageType type
        , u32 num_mip_levels
        , VkSampleCountFlagBits sample_count_flags
        , b8 image_3d = false
        );
      const VkMemoryRequirements get_memory_requirements();
      b8 bind(mem::Allocation memory);
      b8 construct_image_view(const VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);
      b8 terminate();
      const VkDescriptorImageInfo get_image_info(VkSampler sampler) const;
      const mem::Allocation get_current_allocation();
      const VkImage get_image() const;
      const VkImageView get_image_view() const;
      const VkImageLayout get_image_layout() const;
      const u32 get_alignment() {
        return _alignment;
      }
      const gpu::ImageType get_image_type() {
        return _type;
      }
      VkExtent3D get_extent() {
        return _image_extent;
      }
    };
 
//    template<>
//    class Image<3> {
//    private:
//      Context* _context = nullptr;
//      VkImage _image = VK_NULL_HANDLE;
//      VkExtent3D _image_extent{};
//      gpu::ImageType _type;
//      VkDeviceSize _image_size = 0;
//      VkDeviceSize _image_alignment = 0;
//      VkImageView _image_view = VK_NULL_HANDLE;
//      VkImageLayout _image_layout{};
//      VkFormat _current_format{};
//      mem::Allocation _current_allocation{};
//      
//    public:
//      Image() {}
//      b8 init
//        ( Context* context
//        , VkExtent3D extent
//        , const gpu::ImageType type
//        , VkFormat format
//        , VkImageUsageFlags usage_flags
//        , u32 num_mip_levels
//        , VkSampleCountFlagBits sample_count_flags
//        );
//      const VkMemoryRequirements get_memory_requirements();
//      b8 bind(mem::Allocation memory);
//      b8 construct_image_view(const VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);
//      b8 terminate();
//      const mem::Allocation get_current_allocation();
//      const VkDescriptorImageInfo get_image_info(VkSampler sampler) const;
//      VkImage get_image();
//      const gpu::ImageType get_image_type() {
//        return _type;
//      }
//    };

    static VkSampler create_sampler
      ( Context* context
      , const SampleWrapType u_wrap
      , const SampleWrapType v_wrap
      , const SampleWrapType w_wrap
      , const u32 min_lod
      , const u32 max_lod
      , const b8 anisotropy_enable
      );

    static VkSampler create_default_sampler(Context* context);

    static Image create_render_target
      ( Context* context
      , VkExtent2D extent
      , mem::Allocation memory
      , VkImageUsageFlags extra_flags = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT 
      , VkFormat format = VK_FORMAT_R8G8B8A8_UNORM
      ) 
    {
      Image result{};
      VkImageUsageFlags flags 
        =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
        | extra_flags;

      result.init
        ( context
        , VkExtent3D{extent.width, extent.height, 1}
        , gpu::ImageType::RenderTarget
        , 1
        , VK_SAMPLE_COUNT_1_BIT
        );

      result.bind(memory);

      result.construct_image_view(VK_IMAGE_ASPECT_COLOR_BIT);
      return result;
    }

    static Image create_depth_target
      ( Context* context
      , VkExtent2D extent
      , mem::Allocation memory
      , VkImageUsageFlags extra_flags = 0
      , VkFormat format = VK_FORMAT_R8G8B8A8_UNORM
      ) 
    {
      Image result{};

      VkImageUsageFlags flags 
        = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
        | extra_flags;

      VkImageAspectFlags aspect_flags 
        = VK_IMAGE_ASPECT_DEPTH_BIT 
        | VK_IMAGE_ASPECT_STENCIL_BIT;

      result.init
        ( context
        , VkExtent3D{extent.width, extent.height, 1}
        , gpu::ImageType::DepthTarget
        , 1
        , VK_SAMPLE_COUNT_1_BIT
        );

      result.bind(memory);
      result.construct_image_view(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
      return result;
    }

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  -----
