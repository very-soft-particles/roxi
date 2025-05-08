// =====================================================================================
//
//       Filename:  l_file.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-08-05 4:58:25 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_base.hpp"
#include "l_string.hpp"
#include <stdio.h>

namespace lofi {

  enum class FileType {
    ReadOnly,
    WriteOnly,
    ReadWrite,
    Max
  };

  static constexpr char NewLine = '\n';
  static constexpr char LineReturn = '\r';
  static constexpr char Tab = '\t';
  static constexpr char Null = '\0';

  class File {
  private:
    FILE* file = nullptr;
    FileType file_type = FileType::Max;

  public:
    File() {};
    template<FileType FileT>
    static File create(const char* path) {
      File new_file{};
      new_file.file_type = FileT;
      if constexpr(FileT == FileType::ReadOnly) {
        fopen_s(&new_file.file, path, "rb");
      } else if constexpr(FileT == FileType::WriteOnly) {
        fopen_s(&new_file.file, path, "wb");
      } else if constexpr(FileT == FileType::ReadWrite) {
        fopen_s(&new_file.file, path, "rwb");
      }
      if(new_file.file == nullptr) {
        PRINT("[ERROR]: file at path \"%s\" could not be opened\n", path);
      }
      return new_file;
    }

    b8 is_open() {
      return file != nullptr;
    }

    ~File() {
      close();
    }

    b8 close() {
      if(file == nullptr) {
        return false;
      }
      if(fclose(file)) {
        return false;
      }
      file = nullptr;
      return true;
    }

    b8 seek_to_start() {
      if(fseek(file, 0, SEEK_SET)) {
        return false;
      }
      return true;
    }

    b8 seek_to_end() {
      if(fseek(file, 0, SEEK_END)) {
        return false;
      }
      return true;
    }

    b8 seek_to_idx(u64 index) {
      if(fseek(file, 0, index)) {
        return false;
      }
      return true;
    }

    u64 get_size() {
      const auto idx = get_current_offset();
      seek_to_end();
      const auto result = get_current_offset();
      seek_to_idx(idx);
      return result;
    }

    u64 get_current_offset() {
      return ftell(file);
    }

    u8 copy_one_at_current_offset() {
      return fgetc(file);
    }

    b8 set_offset(u64 offset) {
      if(fseek(file, offset, SEEK_SET)) {
        return false;
      }
      return true;
    }

    File& operator<<(String entry) {
      L_ASSERT(file_type != FileType::ReadOnly);
      const u64 string_length = entry.size;
      for(u64 i = 0; i < string_length; i++) {
        fputc(entry.str[i], file);
      }
      return *this;
    }

    File& operator<<(File& file) {
      return file;
    }

    template<class ArrayT>
    File& separate_array_and_copy_to_file(char separator, ArrayT entries) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
        }
        *this << entries[i];
      }
      return *this;
    }

    template<class ArrayT>
    File& separate_array_and_copy_to_file(const char* separator, ArrayT entries) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
        }
        *this << entries[i];
      }
      return *this;
    }

    template<class ArrayT>
    File& separate_array_and_copy_to_file_add_new_line(char separator, u32 new_line_every, ArrayT entries) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
          if(i % new_line_every == 0) {
            *this << lofi::NewLine;
          }
        }
        *this << entries[i];
      }
      return *this;
    }

    template<class ArrayT>
    File& separate_array_and_copy_to_file_add_new_line(const char* separator, u32 new_line_every, ArrayT entries) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
          if(i % new_line_every == 0) {
            *this << lofi::NewLine;
          }
        }
        *this << entries[i];
      }
      return *this;
    }

    template<class ArrayT, class FuncT>
    File& separate_array_and_process_to_file_add_new_line(char separator, u32 new_line_every, ArrayT entries, FuncT&& func) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
          if(i % new_line_every == 0) {
            *this << lofi::NewLine;
          }
        }
        *this << func(entries[i]);
      }
      return *this;
    }

    template<class ArrayT, class FuncT>
    File& separate_array_and_process_to_file_add_new_line(const char* separator, u32 new_line_every, ArrayT entries, FuncT&& func) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
          if(i % new_line_every == 0) {
            *this << lofi::NewLine;
          }
        }
        *this << func(entries[i]);
      }
      return *this;
    }

    template<class ArrayT, class FuncT>
    File& separate_array_and_process_to_file(char separator, ArrayT entries, FuncT&& func) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
        }
        *this << func(entries[i]);
      }
      return *this;
    }

    template<class ArrayT, class FuncT>
    File& separate_array_and_process_to_file(const char* separator, ArrayT entries, FuncT&& func) {
      const auto size = entries.get_size();
      for(u64 i = 0; i < size; i++) {
        if(i != 0) [[likely]] {
          *this << separator;
        }
        *this << func(entries[i]);
      }
      return *this;
    }

    File& operator<<(const char* entry) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%s", entry);
      return *this;
    }

    File& operator<<(char entry) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fputc(entry, file);
      return *this;
    }

    File& operator<<(i32 value) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%i", value);
      return *this;
    }

    File& operator<<(i64 value) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%lli", value);
      return *this;
    }


    File& operator<<(u32 value) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%u", value);
      return *this;
    }

    File& operator<<(u64 value) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%llu", value);
      return *this;
    }

    File& operator<<(f32 value) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%f", value);
      return *this;
    }

    File& operator<<(f64 value) {
      L_ASSERT(file_type != FileType::ReadOnly);
      fprintf(file, "%f", value);
      return *this;
    }


    // make sure buffer is first allocated with get_size() !!
    b8 copy_all_to_buffer(void* buffer) {
      if (file_type == FileType::ReadWrite || file_type == FileType::ReadOnly) {
        fseek(file, 0, SEEK_END);
        const u64 size = ftell(file);
        fseek(file, 0, SEEK_SET);
        if(!(fread(buffer, 1, size, file) == size)) {
          PRINT("[ERROR] copy failed at index %li\n", ftell(file));
          return false;
        }
        return true;
      } else {
        PRINT_LINE("[ERROR] attempting to copy from unexpected file type");
        return false;
      }
    }
  };
}		// -----  end of namespace lofi  ----- 
