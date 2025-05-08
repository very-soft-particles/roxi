// ===================================================================================== // //       Filename:  ecs_config.cpp // //    Description:  //
//        Version:  1.0
//        Created:  2024-07-22 4:11:18 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================

#include "../lofi/core/include/l_string.hpp"
#include "../lofi/core/include/l_container.hpp"
#include <stdio.h>
#include <stdlib.h>

#define TYPE_DEF_TYPES(X) X(ENUM) X(PRIMITIVE) X(LOFI) X(MAX)

enum class TypeDefType {
#define ENUM_DEF(NAME) NAME,
  TYPE_DEF_TYPES(ENUM_DEF)
#undef ENUM_DEF
};

static lofi::String get_type_def_string(TypeDefType Type) {
#define STRING_DEF(type) if (Type == TypeDefType::type) return lofi::str_cstring(STRINGIFY(type));
TYPE_DEF_TYPES(STRING_DEF)
#undef STRING_DEF
  return lofi::String{};
}


template<typename T, u64 Size>
using Array = lofi::mem::ArrayContainerPolicy<T, Size, 8, lofi::mem::SubAllocPolicy>;

static constexpr u64 MaxNumComponentsPerArchetype = 64;
static constexpr u64 MaxNumMembersPerComponent = 32;
static constexpr u64 MaxNumLofiTemplateArgs = 16;
static constexpr u64 MaxNumComponents = 256;

lofi::mem::ArrayContainerPolicy<char, KB(32), 8, lofi::mem::MAllocPolicy> buffer{};

struct EnumTypeDescriptor {
  lofi::String name;
  lofi::mem::ArrayContainerPolicy<lofi::String, 64, 8, lofi::mem::StackAllocPolicy>
    enumerations;
};

lofi::mem::ArrayContainerPolicy<EnumTypeDescriptor, KB(1), 8, lofi::mem::MAllocPolicy> enum_types{};

struct PrimitiveDescriptor {
  lofi::String using_name;
  lofi::String cxx_name;
};

lofi::mem::ArrayContainerPolicy<PrimitiveDescriptor, KB(1), 8, lofi::mem::MAllocPolicy> primitive_types{};

struct LofiDescriptor {
  lofi::String using_name;
  lofi::String lofi_name;
  lofi::mem::ArrayContainerPolicy<lofi::String, MaxNumLofiTemplateArgs, 8, lofi::mem::StackAllocPolicy> template_args{};
};

lofi::mem::ArrayContainerPolicy<LofiDescriptor, KB(1), 8, lofi::mem::MAllocPolicy> lofi_types{};

struct ComponentTypeDescriptor {
  lofi::String name;
  lofi::mem::ArrayContainerPolicy<lofi::String, MaxNumMembersPerComponent, 8, lofi::mem::StackAllocPolicy> member_type_names;
  lofi::mem::ArrayContainerPolicy<lofi::String, MaxNumMembersPerComponent, 8, lofi::mem::StackAllocPolicy> member_variable_names;
  u32 id;
};

lofi::mem::ArrayContainerPolicy<ComponentTypeDescriptor, KB(1), 8, lofi::mem::MAllocPolicy> components{};

struct ArchetypeTypeDescriptor {
  lofi::String name;
  lofi::mem::ArrayContainerPolicy<lofi::String, MaxNumComponentsPerArchetype, 8, lofi::mem::StackAllocPolicy> component_names;
  u32 size = 0;
  u32 id;
};

lofi::mem::ArrayContainerPolicy<ArchetypeTypeDescriptor, KB(1), 8, lofi::mem::MAllocPolicy> archetypes{};


b8 write_cpp_file(const char* file_path) {
  FILE* file;
  fopen_s(&file, file_path, "wb");

  if(file == nullptr) {
    PRINT("[ERROR] failed to open file at path: %s\n", file_path);
    return false;
  }

  const u64 num_component_descriptors = components.get_size();
  const u64 num_archetype_descriptors = archetypes.get_size();
  const u64 num_primitive_type_descriptors = primitive_types.get_size();
  const u64 num_enum_type_descriptors = enum_types.get_size();
  const u64 num_lofi_type_descriptors = lofi_types.get_size();

  lofi::mem::ArrayContainerPolicy<u64, MaxNumComponents, 8, lofi::mem::StackAllocPolicy> tags{};

  fprintf(file, "#pragma once\n");
  fprintf(file, "#include \"rx_vocab.h\"\n");
  fprintf(file, "#include \"rx_tuple.hpp\"\n");
  fprintf(file, "#include \"rx_container.hpp\"\n");
  fprintf(file, "\nnamespace roxi {");
  fprintf(file, "\n\tnamespace config {");
  for(u64 i = 0; i < num_primitive_type_descriptors; i++) {
    fprintf(file, "\n\t\tusing ");
    for(u64 j = 0; j < primitive_types[i].using_name.size; j++) {
      fprintf(file, "%c", primitive_types[i].using_name.str[j]);
    }
    fprintf(file, " = ");
    for(u64 j = 0; j < primitive_types[i].cxx_name.size; j++) {
      fprintf(file, "%c", primitive_types[i].cxx_name.str[j]);
    }
    fprintf(file, ";");
  }
  for(u64 i = 0; i < num_lofi_type_descriptors; i++) {
    const auto num_template_args = lofi_types[i].template_args.get_size();
    if(num_template_args) {
      fprintf(file, "\n\t\ttemplate<");
      for(u64 j = 0; j < num_template_args; j++) {
        if(j != 0) [[likely]] {
          fprintf(file, ", ");
        }
        for(u64 k = 0; k < lofi_types[i].template_args[j].size; k++) {
          fprintf(file, "%c", lofi_types[i].template_args[j].str[k]);
        }
      }
      fprintf(file, ">");
    }
    fprintf(file, "\n\t\tusing ");
    for(u64 j = 0; j < lofi_types[i].using_name.size; j++) {
      fprintf(file, "%c", lofi_types[i].using_name.str[j]);
    }
    fprintf(file, " = ");
    for(u64 j = 0; j < lofi_types[i].lofi_name.size; j++) {
      fprintf(file, "%c", lofi_types[i].lofi_name.str[j]);
    }
    fprintf(file, ";");
  }
  for(u64 i = 0; i < num_enum_type_descriptors; i++) {
    fprintf(file, "\n\t\tenum class ");
    for(u64 j = 0; j < enum_types[i].name.size; j++) {
      fprintf(file, "%c", enum_types[i].name.str[j]);
    }
    fprintf(file, " {\n\t\t\t");
    for(u64 j = 0; j < enum_types[i].enumerations.get_size(); j++) {
      if(j != 0) [[likely]] {
        fprintf(file, ",\n\t\t\t");
      }
      for(u64 k = 0; k < enum_types[i].enumerations[j].size; k++) {
        fprintf(file, "%c", enum_types[i].enumerations[j].str[k]);
      }
    }
    fprintf(file, "\n\t\t};");
  }
  fprintf(file, "\n\n\t\tnamespace components {");
  for(u64 i = 0; i < num_component_descriptors; i++) {
    fprintf(file, "\n\t\t\tstruct ");
    for(u64 j = 0; j < components[i].name.size; j++) {
      fprintf(file, "%c", components[i].name.str[j]);
    }
    fprintf(file, " {");
    const u64 num_members = components[i].member_type_names.get_size();
    if(!num_members) {
      fprintf(file, "};");
      *(tags.push(1)) = i;
      continue;
    }
    for(u64 j = 0; j < num_members; j++) {
      fprintf(file, "\n\t\t\t\t");
      for(u64 k = 0; k < components[i].member_type_names[j].size; k++) {
        fprintf(file, "%c", components[i].member_type_names[j].str[k]);
      }
      fprintf(file, " ");
      for(u64 k = 0; k < components[i].member_variable_names[j].size; k++) {
        fprintf(file, "%c", components[i].member_variable_names[j].str[k]);
      }
      fprintf(file, ";");
    }
    fprintf(file, "\n\t\t\t};");
  }
  fprintf(file, "\n\t\t} // -----  end of namespace components  ----- \n");
  fprintf(file, "\n\t\tusing ComponentList = List<\n\t\t\t\t");
  u64 tag_iter = 0;
  for(u64 i = 0; i < num_component_descriptors; i++) {
    if(i == tags[tag_iter]) {
      tag_iter++;
      continue;
    }
    if(i != 0) [[likely]] {
      fprintf(file, ", ");
      if(!(i & 3)) {
        fprintf(file, "\n\t\t\t");
      }
    }
    fprintf(file, "components::");
    
    for(u64 j = 0; j < components[i].name.size; j++) {
      fprintf(file, "%c", components[i].name.str[j]);
    }
  }
  fprintf(file, "\n\t\t>;\n");

  fprintf(file, "\n\t\tusing TagList = List<\n\t\t\t");
  for(u64 i = 0; i < tags.get_size(); i++) {
    const auto tag_index = tags[i];
    if(i != 0) [[likely]] {
      fprintf(file, ", ");
      if(!(i & 3)) {
        fprintf(file, "\n\t\t\t");
      }
    }
    const auto name_size = components[tag_index].name.size;
    fprintf(file, "components::");
    for(u64 j = 0; j < name_size; j++) {
      fprintf(file, "%c", components[tag_index].name.str[j]);
    }
  }
  fprintf(file, "\n\t\t>;\n");
  fprintf(file, "\n\t\tnamespace archetypes {");
  for(u64 i = 0; i < num_archetype_descriptors; i++) {
    fprintf(file, "\n\t\t\tstruct ");
    for(u64 j = 0; j < archetypes[i].name.size; j++) {
      fprintf(file, "%c", archetypes[i].name.str[j]);
    }
    fprintf(file, " {\n\t\t\t\t");
    fprintf(file, "using type = List<\n\t\t\t\t\t");
    for(u64 j = 0; j < archetypes[i].component_names.get_size(); j++) {
      if(j != 0) [[likely]] {
        fprintf(file, ", ");
        if(!(j & 7)) {
          fprintf(file, "\n\t\t\t\t\t");
        }
      }
      fprintf(file, "components::");
      for(u64 k = 0; k < archetypes[i].component_names[j].size; k++) {
        fprintf(file, "%c", archetypes[i].component_names[j].str[k]);
      }
    }
    fprintf(file, "\n\t\t\t\t>;\n");
    fprintf(file, "\t\t\t\tstatic constexpr u64 size = %u;", archetypes[i].size);
    fprintf(file, "\n\t\t\t};\n");
  }
  fprintf(file, "\n\t\t} // -----  end of namespace archetypes  ----- \n");

  fprintf(file, "\n\t\tusing ArchetypeList = List<\n\t\t\t\t");
  for(size_t i = 0; i < archetypes.get_size(); i++) {
    if(i != 0) [[likely]] {
      fprintf(file, ", ");
      if(!(i & 7)) {
        fprintf(file, "\n\t\t\t");
      }
    }
    fprintf(file, "archetypes::");
    for(u64 j = 0; j < archetypes[i].name.size; j++) {
      fprintf(file, "%c", archetypes[i].name.str[j]);
    }
  }
  fprintf(file, "\n\t\t>;\n");
  fprintf(file, "\n\t} // -----  end of namespace config  ----- \n");
  fprintf(file, "\n} // -----  end of namespace roxi  ----- \n");
  fclose(file);
  return true;
}

b8 parse_csv_archetype_file(const char* file_path) {

  FILE* file;
  fopen_s(&file, file_path, "rb");

  if(file == nullptr) {
    return false;
  }

  if(archetypes.get_buffer() == nullptr) {
    PRINT_LINE("[ERROR] could not allocate heap memory for archetypes");
    return false;
  }

  lofi::mem::ArrayContainerPolicy<char, KB(4), 8, lofi::mem::StackAllocPolicy> line{};
  int c = ' ';
  do {
    c = fgetc(file);
    if(c == '\n' || c == EOF) {
      // parse line here
      const auto size = line.get_size();
      u32 i = 0;
      u64 begin = buffer.get_size();
      while(line[i] != ',' && i < size) {
        if(line[i] == '\0') {
          i++;
          continue;
        }
        *(buffer.push(1)) = line[i];
        i++;
      }
      i++;
      u64 end = buffer.get_size();
      u64 token_size = end - begin;
      if(token_size == 0) {
        break;
      }
      auto archetype_ptr = archetypes.push(1);
      archetype_ptr->name.str = (u8*)&buffer[begin];
      archetype_ptr->name.size = token_size;
      char* iter = &line[i];
      while(line[i] != ',' && i < size) {
        i++;
      }
      archetype_ptr->size = strtoul(iter, &iter, 10);
      i++;
      while(i < size) {
        const u64 begin = buffer.get_size();
        while(line[i] != ',' && i < size) {
          if(line[i] == '\0') {
            break;
          }
          *(buffer.push(1)) = line[i];
          i++;
        }
        i++;
        const u64 end = buffer.get_size();
        const u64 token_size = end - begin;
        if(token_size == 0) {
          break;
        }
        auto component_name_ptr = archetype_ptr->component_names.push(1);
        component_name_ptr->str = (u8*)&buffer[begin];
        component_name_ptr->size = token_size;
      }
      line.clear();
      continue;
    }
    if(line.get_size() == 0 && (char)c == '#') {
      while((c = fgetc(file)) != '\n' && c != EOF) {}
      continue;
    }
    if(c == '\r') {
      continue;
    }
    *(line.push(1)) = (char)c;
  } while(c != EOF);
  fclose(file);
  return true;
}

b8 parse_csv_typedef_file(const char* file_path) {

  FILE* file;
  fopen_s(&file, file_path, "rb");

  if(file == nullptr) {
    PRINT("[ERROR] failed to find obj file at path: %s\n", file_path);
    return false;
  }

  if(enum_types.get_buffer() == nullptr || primitive_types.get_buffer() == nullptr || lofi_types.get_buffer() == nullptr || buffer.get_buffer() == nullptr) {
    PRINT_LINE("[ERROR] could not allocate heap memory for types");
    return false;
  }

  lofi::mem::ArrayContainerPolicy<char, KB(4), 8, lofi::mem::StackAllocPolicy> line{};
  int c = ' ';
  do {
    c = fgetc(file);
    if(c == '\n' || c == EOF) {
      u32 i = 0;
      const u64 line_size = line.get_size();
      u64 begin = buffer.get_size();
      while(line[i] != ',') {
        if(line[i] == '\0') {
          break;
        }
        *(buffer.push(1)) = line[i];
        i++;
      }
      i++;
      u64 end = buffer.get_size();
      lofi::String sub_string{};
      sub_string.str = (u8*)&buffer[begin];
      sub_string.size = end - begin;
      lofi::String string_to_check{};
      u64 s_i = 0;
      for(; s_i < (u64)TypeDefType::MAX; s_i++) {
        string_to_check = get_type_def_string((TypeDefType)s_i);
        for(size_t i = 0; i < string_to_check.size; i++) {
        }
        if(lofi::str_compare(sub_string, string_to_check)) {
          break;
        }
      }
      if((TypeDefType)s_i == TypeDefType::ENUM) {
        u64 begin = buffer.get_size();
        while(line[i] != ',' && i < line_size) {
//          if(line[i] == '\0') {
//            break;
//          }
          *(buffer.push(1)) = line[i];
          i++;
        }
        i++;
        u64 end = buffer.get_size();
        u64 token_size = end - begin;
        if(token_size == 0) {
          break;
        }
        auto new_enum_type_ptr = enum_types.push(1);
        new_enum_type_ptr->name.str = (u8*)&buffer[begin];
        new_enum_type_ptr->name.size = token_size;
        while(i < line_size) {
          begin = buffer.get_size();
          while(line[i] != ',' && i < line_size) {
            if(line[i] == '\0') {
              break;
            }
            *(buffer.push(1)) = line[i];
            i++;
          }
          i++;
          end = buffer.get_size();
          token_size = end - begin;
          if(token_size == 0) {
            break;
          }
          auto enumeration = new_enum_type_ptr->enumerations.push(1);
          enumeration->str = (u8*)&buffer[begin];
          enumeration->size = token_size;
        }
      } else if((TypeDefType)s_i == TypeDefType::PRIMITIVE) {
        auto new_primitive_type_ptr = primitive_types.push(1);
        while(i < line_size) {
          u64 begin = buffer.get_size();
          while(line[i] != ',' && i < line_size) {
            if(line[i] == '\0') {
              break;
            }
            *(buffer.push(1)) = line[i];
            i++;
          }
          i++;
          u64 end = buffer.get_size();
          u64 token_size = end - begin;
          if(token_size == 0) {
            break;
          }
          new_primitive_type_ptr->using_name.str = (u8*)&buffer[begin];
          new_primitive_type_ptr->using_name.size = end - begin;
          begin = buffer.get_size();
          while(line[i] != ',' && i < line_size) {
            if(line[i] == '\0') {
              break;
            }
            *(buffer.push(1)) = line[i];
            i++;
          }
          i++;
          end = buffer.get_size();
          new_primitive_type_ptr->cxx_name.str = (u8*)&buffer[begin];
          new_primitive_type_ptr->cxx_name.size = end - begin;
        }
      } else if((TypeDefType)s_i == TypeDefType::LOFI) {
        u64 begin = buffer.get_size();
        while(line[i] != ',' && i < line_size) {
          if(line[i] == '\"') {
            continue;
          }
          if(line[i] == '\0') {
            break;
          }
          *(buffer.push(1)) = line[i];
          i++;
        }
        i++;
        u64 end = buffer.get_size();
        u64 token_size = end - begin;
        if(token_size == 0) {
          break;
        }
        auto new_lofi_type_ptr = lofi_types.push(1);
        new_lofi_type_ptr->using_name.str = (u8*)&buffer[begin];
        new_lofi_type_ptr->using_name.size = end - begin;
        begin = buffer.get_size();
        while(line[i] != ',' && i < line_size) {
          if(line[i] == '\"') {
            i++;
            while(line[i] != '\"') {
              *(buffer.push(1)) = line[i];
              i++;
            }
            i++;
          }
        }
        i++;
        end = buffer.get_size();
        new_lofi_type_ptr->lofi_name.str = (u8*)&buffer[begin];
        new_lofi_type_ptr->lofi_name.size = end - begin;
        while(i < line_size) {
          const u64 begin = buffer.get_size();
          while(line[i] != ',' && i < line_size) {
            if(line[i] == '\0') {
              break;
            }
            if(line[i] == '\r') {
              break;
            }
            *(buffer.push(1)) = line[i];
            i++;
          }
          const u64 end = buffer.get_size();
          i++;
          const u64 token_size = end - begin;
          if(token_size == 0) {
            break;
          }
          auto template_arg_ptr = new_lofi_type_ptr->template_args.push(1);
          template_arg_ptr->str = (u8*)&buffer[begin];
          template_arg_ptr->size = token_size;
        }
      }
      line.clear();
      continue;
    }
    if(line.get_size() == 0 && (char)c == '#') {
      while((c = fgetc(file)) != '\n') {
      }
      continue;
    }
    if(c == '\r') {
      continue;
    }
    *(line.push(1)) = (char)c;
  } while(c != EOF);
  fclose(file);
  return true;
}

b8 parse_csv_component_file(const char* file_path) {

  FILE* file;
  fopen_s(&file, file_path, "rb");

  if(file == nullptr) {
    return false;
  }

  if(components.get_buffer() == nullptr || buffer.get_buffer() == nullptr) {
    PRINT_LINE("[ERROR] could not allocate heap memory for components");
    return false;
  }

  lofi::mem::ArrayContainerPolicy<char, KB(4), 8, lofi::mem::StackAllocPolicy> line{};
  int c = ' ';
  do {
    c = fgetc(file);
    if(c == '\n' || c == EOF) {
      // parse line here
      const u32 size = line.get_size();
      u32 i = 0;
      u64 begin = buffer.get_size();
      while(line[i] != ',' && i < size) {
        if(line[i] == '\0') {
          break;
        }
        *(buffer.push(1)) = line[i];
        i++;
      }
      i++;
      u64 end = buffer.get_size();
      u64 token_size = end - begin;
      if(token_size == 0) {
        break;
      }
      auto component = components.push(1);
      component->name.str = (u8*)&buffer[begin];
      component->name.size = token_size;
      while(i < size) {
        begin = buffer.get_size();
        while(line[i] != ',' && i < size) {
          if(line[i] == '\0') {
            break;
          }
          *(buffer.push(1)) = line[i];                       
          i++;
        }
        i++;
        end = buffer.get_size();
        token_size = end - begin;
        if(token_size == 0) {
          break;
        }
        auto type_name_ptr = component->member_type_names.push(1);
        type_name_ptr->str = (u8*)&buffer[begin];
        type_name_ptr->size = token_size;
        begin = buffer.get_size();
        while(line[i] != ',' && i < size) {
          if(line[i] == '\0') {
            break;
          }
          *(buffer.push(1)) = line[i];
          i++;
        }
        i++;
        end = buffer.get_size();
        token_size = end - begin;
        auto var_name_ptr = component->member_variable_names.push(1);
        var_name_ptr->str = (u8*)&buffer[begin];
        var_name_ptr->size = token_size;
      }
      line.clear();
      continue;
    }
    if(line.get_size() == 0 && (char)c == '#') {
      while((c = fgetc(file)) != '\n') {}
      continue;
    }
    if(c == '\r') {
      continue;
    }
    *(line.push(1)) = (char)c;
  } while(c != EOF);
  fclose(file);
  return true;
}

int main(int argc, char** argv) {
  if(argc < 4) {
    PRINT_LINE("[ERROR] must provide 3 arguments to ecs_config: [0] = type_def_csv_file_path, [1] = component_csv_file_path, [2] = archetype_csv_file_path, [3] = output_cxx_file_path\n");
    return 1;
  }
  if(!parse_csv_typedef_file(argv[1])) {
    PRINT_LINE("[ERROR] could not parse typedef csv file\n");
    return 1;
  }
  if(!parse_csv_component_file(argv[2])) {
    PRINT_LINE("[ERROR] could not parse component csv file\n");
    return 1;
  }
  if(!parse_csv_archetype_file(argv[3])) {
    PRINT_LINE("[ERROR] could not parse archetype csv file\n");
    return 1;
  }
  if(!write_cpp_file(argv[4])) {
    PRINT_LINE("[ERROR] could not write cpp file\n");
    return 1;
  }
  return 0;
}
