// =====================================================================================
//
//       Filename:  ubo.hpp
//
//    Description:  ubo structs 
//
//        Version:  1.0
//        Created:  2024-10-10 7:43:41 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"

namespace roxi {
  namespace ubo {
    
    struct Camera {
      alignas(16)
        glm::mat4 projection;
      alignas(16)
        glm::mat4 view;
      alignas(16)
        glm::vec4 position;
      alignas(4)
        f32 camera_near;
      alignas(4)
        f32 camera_far;
    };

    struct ViewProjection {
      alignas(16)
        glm::mat4 projection;
      alignas(16)
        glm::mat4 view;
    };

    struct ClusterForward {
      alignas(16)
        glm::mat4 camera_inverse_projection;
      alignas(16)
        glm::mat4 camera_view;
      alignas(8)
        glm::vec2 screen_size;
      alignas(4)
        f32 slice_scaling;
      alignas(4)
        f32 slice_bias;
      alignas(4)
        f32 camera_near;
      alignas(4)
        f32 camera_far;
      alignas(4)
        u32 slice_count_x;
      alignas(4)
        u32 slice_count_y;
      alignas(4)
        u32 slice_count_z;
    };

    struct Frustum {
      alignas(16)
        glm::vec4 planes[6];
      alignas(16)
        glm::vec4 corners[8];
    };

    struct SSAO {
      alignas(16)
        glm::mat4 projection;
      alignas(4)
        f32 radius;
      alignas(4)
        f32 bias;
      alignas(4)
        f32 power;
      alignas(4)
        f32 screen_width;
      alignas(4)
        f32 screen_height;
      alignas(4)
        f32 noise_size;
    };

  }		// -----  end of namespace ubo  ----- 

}		// -----  end of namespace roxi  ----- 
