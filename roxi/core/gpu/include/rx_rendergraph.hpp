// =====================================================================================
//
//       Filename:  r_rendergraph.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-26 11:44:00 AM
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
#include "rx_gpu_device.hpp"

//#define GRAPHICS_RESOURCE_TYPES(X) X(InputAttachment) X(Texture) X(UniformBuffer) X(StorageBuffer) X(Reference) X(MaxType)

namespace roxi {

//  enum class GraphicsResourceType {
//#define ENUM(priority) priority, 
//    GRAPHICS_RESOURCE_TYPES(ENUM)
//#undef ENUM
//  };
//
//  static const char* get_graphics_resource_type_string(GraphicsResourceType type) {
//#define STRING(X) if (type == GraphicsResourceType::X) return STRINGIFY(X);
//    GRAPHICS_RESOURCE_TYPES(STRING)
//#undef STRING
//      else
//        return "";
//  }

  static constexpr u32 MaxGPUGraphNodes = 64;

  struct GPUGraphResource {
    gpu::ResourceInfo info;
    u32 producer_id = MAX_u32;
    u32 base_resource_id = MAX_u32;
    u32 id = MAX_u32;
  };

  struct GPUGraphNode {
    Array<u32> inputs{};
    Array<u32> outputs{};
    Array<u32> edges{};

    u32 pipeline_id = MAX_u32;
    u32 id = MAX_u32;
  };

  using GPUGraphNodeHandle = GPUGraphNode*;


  class GPUGraphBuilder;

  class GPUGraph {
    friend class GPUGraphBuilder;
  public:

    b8 terminate();

    Array<GPUGraphNodeHandle> get_graph() const {
      return sorted_nodes;
    }

    String get_node_name(const u32 index) const {
      return node_names[index];
    }

    String get_resource_name(const u32 index) const {
      return resource_names[index];
    }

    gpu::ResourceInfo get_resource_info(const u32 index) const {
      return resources[index].info;
    }

    u64 get_resource_size(const u32 index) const {
      return get_size_from_resource_type(get_resource_info(index));
    }

  private:
    static u64 get_size_from_resource_type( const gpu::ResourceInfo& info) {
      lofi::dispatcher dispatch 
        { lofi::IdxV<(u64)gpu::ResourceType::NumTypes>
        , [&]<u64 I>(lofi::IdxT<I>) {
          static constexpr gpu::ResourceType type = (gpu::ResourceType)I;
          if constexpr ( resource_is_buffer_type<type>() ) 
          {
            return (u64)info.buffer.size;
          } else if constexpr ( resource_is_image_type<type>() ) {
            return (u64)(info.image.width * info.image.height * info.image.depth);
          } else {
            return 0;
          }
        }};
      return dispatch((u64)info.type);
    }

    Array<String> node_names{nullptr};
    Array<String> resource_names{nullptr};
    Array<GPUGraphResource> resources{nullptr};
    Array<GPUGraphNode> nodes{nullptr};
    Array<GPUGraphNodeHandle> sorted_nodes{nullptr};
  };
  

  class GPUGraphBuilder {
  public:
    b8 init(GPUGraph* rendergraph);

    u32 add_resource
      ( const char* name
      , gpu::ResourceInfo info
      , u32 base_resource_id = MAX_u32
      );

    u32 add_node
      ( const char* name
      , u32 num_inputs
      , u32 num_outputs
      , u32* input_ids
      , u32* output_ids
      );

    b8 compile();
    const GPUGraph* get();
    String get_node_name(u32 id) const;
    String get_resource_name(u32 id) const;

    void clear() {
      if(_graph) {
        _graph->terminate();
        _graph = nullptr;
      }
    }

  private:
    GPUGraph* _graph = nullptr;
  };

}		// -----  end of namespace roxi  ----- 
