// =====================================================================================
//
//       Filename:  compile_shaders.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-09-20 10:16:23 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../lofi/core/include/l_base.hpp"
#include "../lofi/core/include/l_file.hpp"
#include <vector>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

static constexpr u64 MaxSpirvSize = MB(16);
static constexpr u64 MaxNumShaders = KB(2);

using spirv_array_t = std::vector<u32>;

using shader_array_t = std::vector<glslang::TShader>;

shader_array_t g_shaders;

b8 write_spirv(const char* spirv_out_file_path, EShLanguage stage) {

  glslang::TProgram program;

  for(auto& shader : g_shaders) {
    program.addShader(&shader);
    program.link(EShMsgDefault);

    PRINT("[INFO] linking program %s : %s\n[DEBUG] %s\n", spirv_out_file_path, program.getInfoLog(), program.getInfoDebugLog());
  }

  spirv_array_t bytes;

  glslang::TIntermediate* intermediate = program.getIntermediate(stage);

  glslang::GlslangToSpv(*intermediate, bytes);

  lofi::File file = lofi::File::create<lofi::FileType::ReadWrite>(spirv_out_file_path);

  if(!file.is_open()) {
    PRINT_LINE("[ERROR] could not open output file");
    return false;
  }

  for(auto byte : bytes) {
    file << byte;
  }

  file.close();

  return true;
}

b8 parse_shader(lofi::String file_name, const EShLanguage stage, const char* glsl_buffer) {
    const u64 index = g_shaders.size();
    g_shaders.emplace_back(glslang::TShader(stage));
    g_shaders[index].setStrings((const char* const*)(&glsl_buffer), 1);
    g_shaders[index].setPreamble("#extension GL_GOOGLE_include_directive : require\n");
    g_shaders[index].setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
    g_shaders[index].setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
    g_shaders[index].setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_4);

    if(!g_shaders[index].parse
      ( GetDefaultResources()
      , 100
      , false
      , EShMsgDefault
      ))
      return false;

#if(OS_WINDOWS)
    const u32 final_slash_index = lofi::str_find_last('\\', file_name);
#elif(OS_LINUX)
    const u32 final_slash_index = lofi::str_find_last('/', file_string);
#elif(OS_MAC)
    const u32 final_slash_index = lofi::str_find_last('/', file_string);
#endif
    const char* shader_name = (char*)file_name.str + final_slash_index + 1;
    PRINT("[INFO] parsing shader %s : %s\n[DEBUG] %s\n", shader_name, g_shaders[index].getInfoLog(), g_shaders[index].getInfoDebugLog());

    return true;
}

b8 parse_glsl_file(const char* file_path) {

  lofi::File file = lofi::File::create<lofi::FileType::ReadOnly>(file_path);

  const u64 glsl_file_size = file.get_size();
  void* glsl_buffer = malloc(glsl_file_size);

  if(glsl_buffer == nullptr) {
    file.close();
    return false;
  }

  if(!file.copy_all_to_buffer(glsl_buffer)) {
    file.close();
    return false;
  }

  if(!file.close()) {
    return false;
  }

  lofi::String file_string = lofi::str_cstring(file_path);
  const u64 last_period_index = lofi::str_find_last('.', file_string);
  lofi::String postfix = lofi::str(file_string.str + last_period_index + 1, file_string.size - (last_period_index + 1)); 

  if(lofi::str_compare(postfix, lofi::str_lit("vert"))) {
    const EShLanguage stage = EShLangVertex;
    if(!parse_shader(file_string, stage, (const char*)glsl_buffer))
    return false;

  } else if (lofi::str_compare(postfix, lofi::str_lit("frag"))) {
    const EShLanguage stage = EShLangFragment;
    if(!parse_shader(file_string, stage, (const char*)glsl_buffer))
    return false;

  } else if (lofi::str_compare(postfix, lofi::str_lit("comp"))) {
    const EShLanguage stage = EShLangCompute;
    if(!parse_shader(file_string, stage, (const char*)glsl_buffer))
    return false;

  }

  free(glsl_buffer);
  return true;
}

int main(int argc, char** argv) {

  glslang::InitializeProcess();

  for(u64 i = 1; i < argc - 1; i++) {
    parse_glsl_file(argv[i]);
  }

  write_spirv(argv[argc - 1], );

  glslang::FinalizeProcess();

  return 0;
}
