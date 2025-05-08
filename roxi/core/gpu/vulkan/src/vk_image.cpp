// =====================================================================================
//
//       Filename:  vk_image.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-05 9:43:44 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_image.hpp"


namespace roxi {
 
  namespace vk {


    b8 Image::init
      ( Context* context
      , VkExtent3D extent
      , const gpu::ImageType type
      , u32 num_mip_levels
      , VkSampleCountFlagBits sample_count_flags
      , b8 image_3d
      ) 
    {
      _context = context;
      _image_extent = extent;
      _type = type;

      // only for sharing mode concurrent
      //DeviceQueueIndex current_index;
      //u32 num_queue_families = 0;
      //for(size_t i = 0; i < queue_count; i++) {
      //  current_index = vk_context->get_device().get_queue_family_index(queue_indices[i]);
      //  b8 to_continue = false;
      //  for(size_t j = 0; j < i; j++) {
      //    if(index_scratch[j] == current_index.get<0>()) {
      //      to_continue = true;
      //      break;
      //    }
      //  }
      //  if(to_continue) {
      //    continue;
      //  }
      //  *(index_scratch.push(1)) = vk_context->get_device().get_queue_family_index(queue_indices[i]).get<0>();
      //  num_queue_families++;
      //}
      
      VkImageCreateInfo create_info{};
      create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      create_info.pNext = nullptr;
      create_info.flags = 0;
      create_info.imageType = image_3d ? VK_IMAGE_TYPE_3D: VK_IMAGE_TYPE_2D;
      create_info.extent.height = _image_extent.height;
      create_info.extent.width = _image_extent.width;
      create_info.extent.depth = 1;
      create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      create_info.usage = get_image_usage_flags(type);
      create_info.format = get_image_format(type);
      create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      create_info.arrayLayers = 1;
      create_info.samples = sample_count_flags;
      create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
      create_info.mipLevels = num_mip_levels;
      create_info.pQueueFamilyIndices = nullptr;
      create_info.queueFamilyIndexCount = 0;

      VK_CHECK
        ( _context->get_device().get_device_function_table()
          .vkCreateImage
            ( _context->get_device().get_device()
            , &create_info
            , CALLBACKS()
            , &_image
            )
        , "failed to create vulkan image");
      return true;
    }

    const VkMemoryRequirements Image::get_memory_requirements() {
      VkImageMemoryRequirementsInfo2 image_mem_reqs{VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
      image_mem_reqs.image = _image;
      VkMemoryRequirements2 mem_reqs{VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};

      _context->get_device().get_device_function_table()
        .vkGetImageMemoryRequirements2(_context->get_device().get_device()
            , &image_mem_reqs
            , &mem_reqs);

      _alignment = mem_reqs.memoryRequirements.alignment;

      return mem_reqs.memoryRequirements;
    }

    b8 Image::bind(mem::Allocation memory)
    {
      const u32 offset = ALIGN_POW2(memory.offset, _alignment);
      VK_CHECK(_context->get_device()
        .get_device_function_table()
        .vkBindImageMemory(
          _context->get_device().get_device()
          , _image
          , memory.data
          , offset)
        , "failed to bind 2D image");
      _current_allocation = memory;
      return true;
    }

    b8 Image::terminate() {
      if(_image != VK_NULL_HANDLE) {
        _context->get_device()
          .get_device_function_table()
          .vkDestroyImage(
            _context->get_device().get_device()
            , _image
            , CALLBACKS());
        return true;
      }
      return false;
    }

    const VkImage Image::get_image() const {
      return _image;
    }

    const mem::Allocation Image::get_current_allocation() {
      return _current_allocation;
    }

    const VkImageView Image::get_image_view() const {
      return _image_view;
    }

    const VkImageLayout Image::get_image_layout() const {
      return _image_layout;
    }

    const VkDescriptorImageInfo Image::get_image_info(VkSampler sampler) const {
      VkDescriptorImageInfo info
      { sampler
      , _image_view
      , _image_layout
      };
      return info;
    }

//    b8 Image<3>::init
//      ( Context* context
//      , VkExtent3D extent
//      , const gpu::ImageType type
//      , VkFormat format
//      , VkImageUsageFlags usage_flags
//      , u32 num_mip_levels
//      , VkSampleCountFlagBits sample_count_flags
//      ) 
//    {
//      _context = context;
//      _image_extent = extent;
//      _type = type;
//
//      //DeviceQueueIndex current_index;
//      //u32 num_queue_families = 0;
//      //for(size_t i = 0; i < queue_count; i++) {
//      //  current_index = vk_context->get_device().get_queue_family_index(queue_indices[i]);
//      //  b8 to_continue = false;
//      //  for(size_t j = 0; j < i; j++) {
//      //    if(index_scratch[j] == current_index.get<0>()) {
//      //      to_continue = true;
//      //      break;
//      //    }
//      //  }
//      //  if(to_continue) {
//      //    continue;
//      //  }
//      //  *(index_scratch.push(1)) = vk_context->get_device().get_queue_family_index(queue_indices[i]).get<0>();
//      //  num_queue_families++;
//      //}
//
//      VkImageCreateInfo create_info{};
//      create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//      create_info.pNext = nullptr;
//      create_info.flags = 0;
//      create_info.imageType = VK_IMAGE_TYPE_3D;
//      create_info.extent.height = _image_extent.height;
//      create_info.extent.width = _image_extent.width;
//      create_info.extent.depth = _image_extent.depth;
//      create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//      create_info.usage = usage_flags;
//      create_info.format = format;
//      create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//      create_info.arrayLayers = 1;
//      create_info.samples = sample_count_flags;
//      create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
//      create_info.mipLevels = num_mip_levels;
//      create_info.pQueueFamilyIndices = nullptr;
//      create_info.queueFamilyIndexCount = 0;
//
//      VK_CHECK
//        ( _context->get_device().get_device_function_table()
//          .vkCreateImage
//            ( _context->get_device().get_device()
//            , &create_info
//            , CALLBACKS()
//            , &_image
//            )
//        , "failed to create 3D vulkan image");
//      return true;
//
//    }

    b8 Image::construct_image_view(const VkImageAspectFlags aspect_mask) {
      
      VkComponentMapping mapping{};
      mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;

      VkImageSubresourceRange view_range{};
      view_range.layerCount = 1;
      view_range.levelCount = 1;
      view_range.baseArrayLayer = 0;
      view_range.baseMipLevel = 0;
      view_range.aspectMask = aspect_mask;

      VkImageViewCreateInfo view_create_info{};
      view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_create_info.pNext = nullptr;
      view_create_info.flags = 0;
      view_create_info.image = _image;
      view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
      view_create_info.format = get_image_format(_type);
      view_create_info.components = mapping;
      view_create_info.subresourceRange = view_range;

      VK_CHECK
        ( _context->get_device().get_device_function_table()
          .vkCreateImageView
            ( _context->get_device().get_device()
            , &view_create_info
            , CALLBACKS()
            , &_image_view
            )
          , "failed to create 2D image view"
        );

      return true;
    }

//    b8 Image<3>::bind(mem::Allocation memory) {
//      VK_CHECK(_context->get_device()
//        .get_device_function_table()
//        .vkBindImageMemory(
//          _context->get_device().get_device()
//          , _image
//          , memory.data
//          , memory.offset)
//        , "failed to bind 3D image");
//      _current_allocation = memory;
//      return true;
//    }
//
//    const VkMemoryRequirements Image<3>::get_memory_requirements() {
//      VkImageMemoryRequirementsInfo2 image_mem_reqs{VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
//      image_mem_reqs.image = _image;
//      VkMemoryRequirements2 mem_reqs{VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
//
//      _context->get_device().get_device_function_table()
//        .vkGetImageMemoryRequirements2(_context->get_device().get_device()
//            , &image_mem_reqs
//            , &mem_reqs);
//
//      return mem_reqs.memoryRequirements;
//    }
//
//    b8 Image<3>::construct_image_view(const VkImageAspectFlags aspect_mask) {
//      
//      VkComponentMapping mapping{};
//      mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//      mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//      mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//      mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//
//      VkImageSubresourceRange view_range{};
//      view_range.layerCount = 1;
//      view_range.levelCount = 1;
//      view_range.baseArrayLayer = 0;
//      view_range.baseMipLevel = 0;
//      view_range.aspectMask = aspect_mask;
//
//      VkImageViewCreateInfo view_create_info{};
//      view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//      view_create_info.pNext = nullptr;
//      view_create_info.flags = 0;
//      view_create_info.image = _image;
//      view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
//      view_create_info.format = _current_format;
//      view_create_info.components = mapping;
//      view_create_info.subresourceRange = view_range;
//
//      VK_CHECK
//        ( _context->get_device().get_device_function_table()
//          .vkCreateImageView
//            ( _context->get_device().get_device()
//            , &view_create_info
//            , CALLBACKS()
//            , &_image_view
//            )
//          , "failed to create 2D image view"
//        );
//
//      return true;
//    }
//
//
//    b8 Image<3>::terminate() {
//      if(_image != VK_NULL_HANDLE) {
//        _context->get_device()
//          .get_device_function_table()
//          .vkDestroyImage(
//            _context->get_device().get_device()
//            , _image
//            , CALLBACKS());
//        return true;
//      }
//      return false;
//    }
//
//    const VkDescriptorImageInfo Image<3>::get_image_info(VkSampler sampler) const {
//      VkDescriptorImageInfo info
//      { sampler
//      , _image_view
//      , _image_layout
//      };
//      return info;
//    }

    VkSampler create_sampler
      ( Context* context
      , const SampleWrapType u_wrap
      , const SampleWrapType v_wrap
      , const SampleWrapType w_wrap
      , const u32 min_lod
      , const u32 max_lod
      , const b8 anisotropy_enable
      ) 
    {
      VkSampler result = VK_NULL_HANDLE;

      VkSamplerCreateInfo sampler_create_info{};
      sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      sampler_create_info.addressModeU = get_sampler_address_mode(u_wrap);
      sampler_create_info.addressModeV = get_sampler_address_mode(v_wrap);
      sampler_create_info.addressModeW = get_sampler_address_mode(w_wrap);
      sampler_create_info.minFilter = VK_FILTER_NEAREST;
      sampler_create_info.magFilter = VK_FILTER_NEAREST;
      sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      sampler_create_info.anisotropyEnable = anisotropy_enable ? VK_TRUE : VK_FALSE;
      sampler_create_info.compareEnable = VK_FALSE;
      sampler_create_info.unnormalizedCoordinates = VK_FALSE;
      sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      sampler_create_info.minLod = min_lod;
      sampler_create_info.maxLod = max_lod;
      
      RX_RETURN( context->get_device().get_device_function_table()
          .vkCreateSampler
            ( context->get_device().get_device()
            , &sampler_create_info
            , CALLBACKS()
            , &result
            ) != VK_SUCCESS
          ,"failed to create sampler"
          , VK_NULL_HANDLE
        );

      return result;
    }

    VkSampler create_default_sampler(Context* context) {
      VkSampler result = VK_NULL_HANDLE;

      VkSamplerCreateInfo sampler_create_info{};
      sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      sampler_create_info.minFilter = VK_FILTER_NEAREST;
      sampler_create_info.magFilter = VK_FILTER_NEAREST;
      sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      sampler_create_info.anisotropyEnable = VK_FALSE;
      sampler_create_info.compareEnable = VK_FALSE;
      sampler_create_info.unnormalizedCoordinates = VK_FALSE;
      sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      sampler_create_info.minLod = 0;
      sampler_create_info.maxLod = 16;
     
      RX_RETURN( context->get_device().get_device_function_table()
          .vkCreateSampler
            ( context->get_device().get_device()
            , &sampler_create_info
            , CALLBACKS()
            , &result
            ) != VK_SUCCESS
          ,"failed to create default sampler"
          , VK_NULL_HANDLE
        );

      return result;
    }
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
