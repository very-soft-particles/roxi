// =====================================================================================
//
//       Filename:  vk_descriptors.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-05 5:07:22 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_resource.hpp"

namespace roxi {
  namespace vk {

    static constexpr u32 s_max_descriptor_sets = KB(4);
    static constexpr u32 s_max_descriptor_set_layouts = KB(1);
    static constexpr u32 s_max_descriptor_buffers = 64;
    static constexpr u32 s_max_descriptor_set_layout_bindings = 16;
    static constexpr u32 s_max_descriptors = KB(4);

    enum class DescriptorBufferType : u8 {
      Uniform,
      Storage,
      CombinedImageSampler,
      StorageImage,
      Max
    };

    VkBufferUsageFlags get_descriptor_buffer_usage_flags(DescriptorBufferType type);

    const VkDescriptorType get_descriptor_type(DescriptorBufferType type);
    const VkDescriptorType get_descriptor_type(gpu::BufferType type);
    const VkDescriptorType get_descriptor_type(gpu::ImageType type);

    struct DescriptorAllocation {
      VkDeviceSize size = 0;
      u32 offset = MAX_u32;

      const u32 get_buffer_id() const {
        return offset / size;
      }

      DescriptorAllocation operator+(const u32 rhs) {
        return DescriptorAllocation{size, offset + (rhs * (u32)size)};
      }
    };

    class DescriptorBufferArena {
    private:
      Buffer _descriptor_buffer;
      u64 _descriptor_counter = 0;
      VkDeviceSize _descriptor_size = 0;
      VkDeviceSize _buffer_size = 0;
      DescriptorBufferType _type = DescriptorBufferType::Max;
    public:
      b8 init(Context* context, const DescriptorBufferType type, const VkDeviceSize descriptor_size, const VkDeviceSize offset_alignment, const u32 count);

      const VkBufferUsageFlags get_usage_flags() const {
        return get_descriptor_buffer_usage_flags(_type);
      }

      b8 bind(mem::Allocation memory);

      const u32 get_descriptor_size() const {
        return _descriptor_size;
      }

      DescriptorAllocation allocate(Context* context, const u64 descriptor_count, const u64 descriptor_offset_alignment);

      void clear();

      Buffer& get_descriptor_buffer() {
        return _descriptor_buffer;
      }

      const Buffer& get_descriptor_buffer() const {
        return _descriptor_buffer;
      }

      const u64 get_allocated_descriptor_count() const {
        return _descriptor_counter;
      }

      const u64 get_buffer_byte_size() const {
        return (u64)_buffer_size;
      }

      b8 terminate();

    };

    class DescriptorSetLayoutCreation {
    private:
      VkDescriptorSetLayoutCreateFlags _flags = 0;
      struct BindingInfo {
        VkDescriptorSetLayoutBinding binding;
      };
      Array<VkDescriptorSetLayoutBinding> _bindings;

    public:
      b8 init();

      b8 add_binding(VkDescriptorSetLayoutBinding binding);

      void set_flags(VkDescriptorSetLayoutCreateFlags flags) {
        _flags = flags;
      }

      VkDescriptorSetLayoutCreateInfo get_create_info() const;

      void clear_bindings();

      void clear();

      void clear_flags();

      b8 terminate();

    };

    class DescriptorSetLayout {
    private:
      VkDescriptorSetLayout _descriptor_set_layout = VK_NULL_HANDLE;
      SizedStackArray<VkDescriptorType, s_max_descriptor_set_layout_bindings> _bindings;
    public:
      b8 init(Context* context, const DescriptorSetLayoutCreation& creation);

      VkDescriptorSetLayout get_descriptor_set_layout() const;

      VkDeviceSize get_layout_size(Context* context) const;

      const u32 get_num_bindings() const;

      VkDescriptorType get_layout_binding_type(u32 binding) const;

      VkDeviceSize get_layout_binding_offset(Context* context, u32 binding) const;

      ~DescriptorSetLayout();

      b8 terminate(Context* context);
    };

    class DescriptorPoolBuilder;
    class CommandBuffer;

    class DescriptorPool {
    private:
      friend class DescriptorPoolBuilder;
      friend class CommandBuffer;

      Context*            _context;
      mem::MemoryPool _memory_pool;
      DescriptorBufferArena _descriptor_arenas[4];
      u32 _descriptor_offset_alignment{};

      static constexpr u32 UniformArenaIndex = 0;
      static constexpr u32 StorageArenaIndex = 1;
      static constexpr u32 TextureArenaIndex = 2;
      static constexpr u32 ImageArenaIndex   = 3;

      template<DescriptorBufferType Type>
      static constexpr u32 get_arena_index() {
        if constexpr (Type == DescriptorBufferType::Uniform) {
          return UniformArenaIndex;
        } else if constexpr (Type == DescriptorBufferType::Storage) {
          return StorageArenaIndex;
        } else if constexpr (Type == DescriptorBufferType::CombinedImageSampler) {
          return TextureArenaIndex;
        } else if constexpr (Type == DescriptorBufferType::StorageImage) {
          return ImageArenaIndex;
        }
        return MAX_u32;
      }

      template<DescriptorBufferType Type>
      const DescriptorBufferArena& obtain_arena() const {
        return _descriptor_arenas[get_arena_index<Type>()];
      }

      template<DescriptorBufferType Type>
      DescriptorBufferArena& obtain_arena() {
        return _descriptor_arenas[get_arena_index<Type>()];
      }

      const u32 get_offset_alignment() const;

    public:
      using DescriptorBufferHandle = u32;

      b8 init(Context* context, const u32 uniform_buffer_count, const u32 storage_buffer_count, const u32 texture_count, const u32 storage_image_count);

      const b8 get_descriptor(const vk::Buffer& buffer, const gpu::BufferType type, const vk::DescriptorAllocation allocation, void* dst);

      template<vk::DescriptorBufferType Type>
      const vk::DescriptorAllocation allocate(const u32 count) {
        return obtain_arena<Type>().allocate(_context, count, get_offset_alignment());
      }

      template<vk::DescriptorBufferType Type>
      const vk::Buffer& get_descriptor_buffer() const {
        return obtain_arena<Type>().get_descriptor_buffer();
      }

      template<vk::DescriptorBufferType Type>
      vk::Buffer& get_descriptor_buffer() {
        return obtain_arena<Type>().get_descriptor_buffer();
      }


      b8 terminate();
    };

//    class DescriptorPoolBuilder {
//    private:
//      struct descriptor_buffer_init_info {
//        DescriptorBufferType type;
//        DescriptorBufferLevel level;
//        u32 count;
//      };
//      Context* _context = nullptr;
//      mem::MemoryPoolBuilder _memory_builder{};
//      StackArray<descriptor_buffer_init_info>
//        _init_infos;
//
//      u32 _descriptor_buffer_offset = MAX_u32;
//
//      u32 _ubo_descriptor_size = MAX_u32;
//      u32 _storage_descriptor_size = MAX_u32;
//      u32 _image_descriptor_size         = MAX_u32;
//      u32 _texture_descriptor_size = MAX_u32;
//
//
//    public:
//      b8 init(Context* context);
//
//      DescriptorPool::DescriptorBufferHandle add_descriptor_buffer(const DescriptorBufferType type, const DescriptorBufferLevel level, const u32 descriptor_count);
//
//      b8 build(DescriptorPool* const pool);
//
//      b8 terminate();
//    };

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
