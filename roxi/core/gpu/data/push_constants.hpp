// =====================================================================================
//
//       Filename:  push_constants.hpp
//
//    Description:  push constant structs 
//
//        Version:  1.0
//        Created:  2024-10-10 7:53:46 AM
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
  namespace pc {

    struct FrameParams {
      u32 frame_id;
    };
    
    // For IBL Lookup Table
    struct PushConstBRDFLUT {
    	u32 width;
    	u32 height;
    	u32 sample_count;
    };
    
    // For generating specular and diffuse maps
    struct PushConstCubeFilter {
    	f32 roughness = 0.f;
    	u32 output_diffuse_sample_count = 1u;
    };
    
    // Additional customization for PBR
    struct PushConstPBR {
    	f32 light_intensity = 1.f;
    	f32 base_reflectivity = 0.04f;
    	f32 max_reflection_lod = 4.f;
    	f32 light_falloff = 1.0f; // Small --> slower falloff, Big --> faster falloff
    	f32 albedo_multipler = 0.0f; // Show albedo color if the scene is too dark, default value should be zero
    };

  }		// -----  end of namespace pc  ----- 
}		// -----  end of namespace roxi  ----- 
