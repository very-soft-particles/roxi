// =====================================================================================
//
//       Filename:  graphics_config.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-14 9:33:00 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../lofi/core/include/l_vocab.hpp"
#include "../lofi/core/include/l_container.hpp"
#include "../lofi/core/include/l_tuple.hpp"
#include "../lofi/core/include/l_file.hpp"
#include <stdio.h>
#include <stdlib.h>

template<typename T, size_t N>
using Array = lofi::mem::ArrayContainerPolicy<T, N, 8, lofi::mem::SubAllocPolicy>;

template<typename T>
using Vec4 = lofi::tuple<T, T, T, T>;

template<typename T>
using Vec3 = lofi::tuple<T, T, T>;

template<typename T>
using Vec2 = lofi::tuple<T, T>;

template<typename T>
struct Vertex {
  Vec3<T> position;
  Vec3<T> normal;
  Vec2<T> texture_coord;
};

struct VertexIndex {
  u32 vertex, normal, texture_coord;
};

struct Obj {
  using VertexT = Vertex<float>;
  Obj(size_t num_verts, size_t num_inds) {
    vertices.move_ptr(malloc(sizeof(VertexT) * num_verts));
    indices.move_ptr(malloc(sizeof(VertexIndex) * num_inds));
  }
  ~Obj() {
    free(vertices.get_buffer());
    free(indices.get_buffer());
  }
  lofi::String name;
  Array<Vertex<float>, KB(8)> vertices;
  Array<VertexIndex, KB(32)> indices;
};

b8 write_cpp_file(Obj* objs, u32 num_objs, const char* file_path) {
  PRINT_LINE("entering write function");
  lofi::File file = lofi::File::create<lofi::FileType::WriteOnly>(file_path);

  if(!file.is_open()) {
    PRINT("failed to open file at path: %s\n", file_path);
    return false;
  }
  PRINT("successfully opened write file at path: %s\n", file_path);
  u64 vertices_start_index = 0;
  u64 indices_start_index = 0;
  file << "#pragma once\n"
  << "#include \"rx_vocab.h\"\n"
  << "\nnamespace roxi {"
  << "\n\tnamespace graphics {"
  << "\n\t\tnamespace resources {\n"
  << "\n\t\t\tinline constexpr u32 num_objs = " << num_objs << ';' << lofi::NewLine
  << "\n\t\t\tinline constexpr u32 obj_handles[] = {\n\t\t\t\t";
  for(size_t i = 0; i < num_objs; i++) {
    if(i != 0) [[likely]] {
      file << ", ";
      if(!(i & 7)) {
        file << "\n\t\t\t\t";
      }
    }
    file << vertices_start_index;
    vertices_start_index += objs[i].vertices.get_size();
  }
  file << "\n\t\t\t};\n"
  << "\n\t\t\tinline constexpr u64 total_num_vertices = " << vertices_start_index << ';' << lofi::NewLine
  << "\n\t\t\tinline constexpr u32 index_handles[] = {\n\t\t\t\t";
  for(size_t i = 0; i < num_objs; i++) {
    if(i != 0) [[likely]] {
      file << ", ";
      if(!(i & 7)) {
        file << "\n\t\t\t\t";
      }
    }
    file << indices_start_index;
    indices_start_index += objs[i].indices.get_size();
  }
  file << "\n\t\t\t};\n"
  << "\n\t\t\tinline constexpr u64 total_num_indices = " << indices_start_index << ';' << lofi::NewLine
  << "\n\t\t\t"
  << "inline constexpr u64 obj_sizes[] {\n\t\t\t\t";
  for(size_t i = 0; i < num_objs; i++) {
    if(i != 0) [[likely]] {
      file << ", ";
      if(!(i & 7)) {
        file << "\n\t\t\t\t";
      }
    }
    file << objs[i].vertices.get_size();
  }
  file << "\n\t\t\t};\n\n\t\t\t"
  << "inline constexpr u64 obj_index_count[] {\n\t\t\t\t";
  for(size_t i = 0; i < num_objs; i++) {
    if(i != 0) [[likely]] {
      file << ", ";
      if(!(i & 7)) {
        file << "\n\t\t\t\t";
      }
    }
    file << objs[i].indices.get_size();
  }
  file << "\n\t\t\t};\n\n\t\t\t"
  << "inline constexpr const char* obj_names[] {\n\t\t\t\t";
  for(size_t i = 0; i < num_objs; i++) {
    if(i != 0) [[likely]] {
      file << ", ";
      if(!(i & 7)) {
        file << "\n\t\t\t\t";
      }
    }
    lofi::String name = objs[i].name;
    name.size -= 4;
    name.str[name.size] = '\0';
    size_t delim = 0;
    for(size_t i = 0; i < name.size; i++) {
      if(name.str[i] == '\\' || name.str[i] == '/') {
        delim = i;
      }
    }
    name.str = name.str + delim + 1;
    file << "\"" << (char*)name.str << "\"";
  }
  file << "\n\t\t\t};\n\n\t\t\t"

  << "// position[3], normal[3], tex_coord[2]"

  << "\n\n\t\t\t"

  << "inline constexpr float vertices[][8] {\n\t\t\t\t";
  size_t accum = 0;
  for(size_t i = 0; i < num_objs; i++) {
    const size_t num_verts = objs[i].vertices.get_size();
    for(size_t j = 0; j < num_verts; j++) {
      if(accum != 0) [[likely]] {
        file << ", ";
        if(!(accum & 7)) {
          file << "\n\t\t\t\t";
        }
      }
      file << "{ " << objs[i].vertices[j].position.get<0>() << ','
      << objs[i].vertices[j].position.get<1>() << ','
      << objs[i].vertices[j].position.get<2>() << ','
      << objs[i].vertices[j].normal.get<0>() << ','
      << objs[i].vertices[j].normal.get<1>() << ','
      << objs[i].vertices[j].normal.get<2>() << ','
      << objs[i].vertices[j].texture_coord.get<0>() << ','
      << objs[i].vertices[j].texture_coord.get<1>() << '}';
      accum++;
    }
  }
  file << "\n\t\t\t};\n\n\t\t\t"
  << "inline constexpr u32 indices[][3] {\n\t\t\t\t";
  accum = 0;
  for(size_t i = 0; i < num_objs; i++) {
    const size_t num_inds = objs[i].indices.get_size();
    for(size_t j = 0; j < num_inds; j++) {
      if(accum != 0) [[likely]] {
        file << ", ";
        if(!(accum & 7)) {
          file << "\n\t\t\t\t";
        }
      }
      file << '{' << objs[i].indices[j].vertex << ','
      << objs[i].indices[j].normal << ','
      << objs[i].indices[j].texture_coord << '}';
      accum++;
    }
  }
  file << "\n\t\t\t};\n\t\t}// -------- end of namespace resources ---------\n\t}// -------- end of namespace graphics ---------\n}// -------- end of namespace roxi ---------\n";
  
  file.close();
  return true;
}

b8 parse_obj_file(Obj* obj, const char* file_path) {
  lofi::File file = lofi::File::create<lofi::FileType::ReadOnly>(file_path);

  PRINT_LINE("starting parse");

  if(!file.is_open()) {
    PRINT("failed to find obj file at path: %s\n", file_path);
    return false;
  }

  const u32 file_size = file.get_size();

  lofi::mem::ArrayContainerPolicy<Vec3<float>, KB(32), 8, lofi::mem::StackAllocPolicy> vertex_positions{};
  lofi::mem::ArrayContainerPolicy<Vec3<float>, KB(32), 8, lofi::mem::StackAllocPolicy> vertex_normals{};
  lofi::mem::ArrayContainerPolicy<Vec2<float>, KB(32), 8, lofi::mem::StackAllocPolicy> texture_coords{};

  lofi::mem::ArrayContainerPolicy<u32, KB(32), 8, lofi::mem::StackAllocPolicy> position_indices{};
  lofi::mem::ArrayContainerPolicy<u32, KB(32), 8, lofi::mem::StackAllocPolicy> normal_indices{};
  lofi::mem::ArrayContainerPolicy<u32, KB(32), 8, lofi::mem::StackAllocPolicy> tex_coord_indices{}; 

  lofi::mem::ArrayContainerPolicy<char, KB(4), 8, lofi::mem::StackAllocPolicy> line{};
  int c;
  u32 current_size = 0;
  while((c = file.copy_one_at_current_offset()) != EOF && (current_size < file_size)) {
    //PRINT("%c", c);                                       // to see file print out
    if(c == '\n') {
      *(line.push(1)) = (char)c;
      const size_t line_length = line.get_size();
      current_size += line_length;
      if(line[0] == '#' || line[0] == 'o') {
      }
      else if(line[0] == 's') {
      }
      else if(line[0] == 'f') {
        char* iter = &line[2];
        int count = 0;
        //char* end = &line[line_length];
        while(*iter != '\n') {
          if(*iter == ' ') {
            iter++;
            count = 0;
            continue;
          }
          if(*iter == '/') {
            iter++;
            count++;
            continue;
          }
          if(count == 0) {
            *(position_indices.push(1)) = strtoul(iter, &iter, 10);
          } else if(count == 1) {
            *(tex_coord_indices.push(1)) = strtoul(iter, &iter, 10);
          } else if(count == 2) {
            *(normal_indices.push(1)) = strtoul(iter, &iter, 10);
          }
        }
      }
      else if(line[0] == 'v' && line[1] == ' ') {
        Vec3<float>* pos = vertex_positions.push(1);
        char* iter = &line[2];
        pos->get<0>() = strtof(&line[2], &iter);
        pos->get<1>() = strtof(iter + 1, &iter);
        pos->get<2>() = strtof(iter + 1, &iter);
        //PRINT("vertex position = %f, %f, %f\n", pos->get<0>(), pos->get<1>(), pos->get<2>());
      }
      else if(line[0] == 'v' && line[1] == 't') {
        Vec2<float>* coord = texture_coords.push(1);
        char* iter = &line[2];
        coord->get<0>() = strtof(&line[2], &iter);
        coord->get<1>() = strtof(iter + 1, &iter);
      }
      else if(line[0] == 'v' && line[1] == 'n') {
        Vec3<float>* norm = vertex_normals.push(1);
        char* iter = &line[2];
        norm->get<0>() = strtof(&line[2], &iter);
        norm->get<1>() = strtof(iter + 1, &iter);
        norm->get<2>() = strtof(iter + 1, &iter);
      }
      line.clear();
      continue;
    }
    *(line.push(1)) = (char)c;
  }
  PRINT_LINE("finished parsing... writing to obj struct");
  const size_t vertex_count = vertex_positions.get_size();
  const size_t index_count = position_indices.get_size();
  new(obj) Obj(vertex_count, index_count);
  obj->name = lofi::str_cstring(file_path);
  EVAL_PRINT_ULL(vertex_count);
  Vertex<float>* vertices = obj->vertices.push(vertex_count);
  for(size_t i = 0; i < vertex_count; i++) {
    vertices[i].position = vertex_positions[i];
    vertices[i].normal = vertex_normals[i];
    vertices[i].texture_coord = texture_coords[i];
  }
  EVAL_PRINT_ULL(index_count);
  VertexIndex* indices = obj->indices.push(index_count);
  for(size_t i = 0; i < index_count; i++) {                         // minus 1 because obj file indices
    indices[i].vertex = position_indices[i] - 1;                    // are ordinal and not actual indices
    indices[i].normal = normal_indices[i] - 1;
    indices[i].texture_coord = tex_coord_indices[i] - 1;
  }
  file.close();
  return true;
}

int main(int argc, char** argv) {
  lofi::mem::ArrayContainerPolicy<Obj, KB(1), 8, lofi::mem::MAllocPolicy> objs;
  if(objs.get_buffer() == nullptr) {
    PRINT_LINE("[ERROR] : could not allocate heap memory for objs");
    return EXIT_SUCCESS;
  }
  for(size_t i = 1; i < argc - 1; i++) {
    Obj* obj = objs.push(1);
    PRINT("obj file name: %s\n", argv[i]);
    if(!parse_obj_file(obj, argv[i])) {
      PRINT_LINE("[ERROR] : failed to parse obj");
      return EXIT_SUCCESS;
    }
    PRINT("successfully parsed obj %s", argv[i]);
  }
  PRINT_LINE("successfully parsed objs");
  const size_t obj_count = objs.get_size();
  if(!write_cpp_file(objs.get_buffer(), obj_count, argv[argc - 1])) {
    PRINT_LINE("[ERROR] : failed to write cpp file");
    return EXIT_SUCCESS;
  }
  for(size_t i = 0; i < obj_count; i++) {
    (objs.get_buffer() + i)->~Obj();
  }
  return EXIT_SUCCESS;
}
