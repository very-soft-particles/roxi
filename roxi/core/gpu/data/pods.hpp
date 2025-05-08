// =====================================================================================
//
//       Filename:  pods.hpp
//
//    Description:  plain old data structs 
//
//        Version:  1.0
//        Created:  2024-10-10 9:43:58 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"
#include "glm/gtc/quaternion.hpp"
#include "aabb.hpp"

#define MATERIAL_TYPES(X) X(Opaque) X(Transparent) X(Specular) X(Light)

namespace roxi {

  static constexpr u64 s_max_instances = KB(4);
  static constexpr u64 s_max_animation_node_children = 64;

  enum class MaterialType : u8 {
#define ENUM(Type) Type,
    MATERIAL_TYPES(ENUM)
#undef ENUM
  };

  template<MaterialType TypeT>
  static constexpr const char* get_material_name() {
#define NAME(Type) if constexpr (MaterialType::Type == TypeT) return STRINGIFY(Type);
    MATERIAL_TYPES(NAME)
#undef NAME
  }


  namespace ubo {
    
    struct TestDrawParams {
    alignas(4)
      u32 vertex_buffer_id;
    };

  
    struct DrawParams {
    alignas(4)
      u32 camera_buffer_id;
    alignas(4)
      u32 vertex_buffer_id;
    alignas(4)
      u32 index_buffer_id;
    alignas(4)
      u32 mesh_buffer_id;

    alignas(4)
      u32 material_buffer_id;
    alignas(4)
      u32 model_buffer_id;
    alignas(4)
      u32 instance_buffer_id;
    alignas(4)
      u32 light_buffer_id;

    alignas(4)
      u32 light_cell_buffer_id;
    alignas(4)
      u32 light_index_buffer_id;
    alignas(4)
      u32 clustered_forward_params_buffer_id;
    alignas(4)
      u32 aabb_buffer_id;
    };

  }		// -----  end of namespace ubo  ----- 

  // For bindless textures
  struct MeshData {
    alignas(4)
  	u32 vertex_offset = 0;
    alignas(4)
  	u32 index_offset = 0;
  
  	// PBR Texture IDs
    alignas(4)
  	u32 albedo = 0;
    alignas(4)
  	u32 normal = 0;

    alignas(4)
  	u32 metalness = 0;
    alignas(4)
  	u32 roughness = 0;
    alignas(4)
  	u32 ao = 0;
    alignas(4)
  	u32 emissive = 0;
  
  	// For sorting
    alignas(4)
  	MaterialType material{};
  };

  struct Model {
    alignas(16)
      glm::mat4 model;
  };


  struct InstanceData
  {
  	/*Update model matrix and update the buffer
  	Need two indices to access instanceMapArray_*/
    Model model;
  };
  
  // Needed for updating bounding boxes
  struct InstanceMap
  {
  	// Pointing to modelSSBO_
  	u32 model_matrix_index = 0;
  
  	// List of global instance indices that share the same model matrix
  	Array<u32> instance_data_indices{};
  };
  
  struct ModelCreateInfo
  {
  	String file_name{};
  
  	// Allows multiple draw calls 
  	u32 instance_count = 1u; 
  	
  	// No effect if the model does not have animation
  	b8 play_animation = false;
  
  	b8 clickable = false;
  };
  
  // Skinning
  struct BoneInfo
  {
  	// ID is index in finalBoneMatrices
  	i32 id = 0; // The first matrix is identity
  
  	// Offset matrix transforms vertex from model space to bone space
  	glm::mat4 offset_matrix = glm::mat4(1.0);
  };
  
  // Skinning
  struct AnimationNode
  {
  	glm::mat4 transformation = glm::mat4(1.0);
  	String name{};
  	u32 children_count = 0u;
  	Array<AnimationNode*> children{};
  };
  
  // Skinning
  struct KeyPosition
  {
  	glm::vec3 position{};
  	f32 time_stamp = 0.0f;
  };
  
  // Skinning
  struct KeyRotation
  {
    glm::quat orientation;
  	f32 time_stamp = 0.0f;
  };
  
  // Skinning
  struct KeyScale
  {
  	glm::vec3 scale{};
  	f32 time_stamp = 0.0f;
  };

}		// -----  end of namespace roxi  ----- 
