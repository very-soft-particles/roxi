// =====================================================================================
//
//       Filename:  string.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-19 10:59:22 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_vocab.hpp"
#include <cstdarg>
#include <vadefs.h>

namespace lofi {
  
  struct String {
    u8* str = nullptr;
    u64 size = 0;

    String() {}

    String(u8* str, u64 size) : str(str), size(size) {}

    String(const char* c_string) : str((u8*)c_string), size(0) {
      while(c_string[size++] != '\0') {}
    }

    template<typename ArenaT>
    static String create_explicit(ArenaT* arena, u8* str, size_t size) {

      void* ptr = arena->push(size);
      MEM_COPY(ptr, str, size);

      return {(u8*)ptr, size};
    }

    template<typename ArenaT>
    static String null_at(ArenaT* arena) {
      String result = {};
      result.str = (u8*)arena->push(1);
      *result.str = (u8)'\0';
      return result;
    }

    String& operator=(const char* rhs) {
      String temp{rhs};
      str = temp.str;
      size = temp.size;
      return *this;
    }

  };

  struct StringNode {
    StringNode* next = nullptr;
    String string;
  };

#define PRINT_STRING(string) \
  { u64 str_iter = 0;\
    while(str_iter < string.size)\
    { PRINT_C(string.str[str_iter++]); }\
  }

  struct StringList {
    StringNode* first = nullptr;
    StringNode* last = nullptr;
    u64 node_count = 0;
    u64 total_size = 0;
  };

  struct StringJoin {
    String pre;
    String mid;
    String post;
  };

  static String str(u8* str, size_t size) {
    return {str, size};
  }

  static String str_range(u8* first, u8* opl = nullptr) {
    size_t diff = sizeof(first) - 1;
    if(opl != nullptr) {
      diff = PTR2INT(opl - first);
    }
    return {first, diff};
  }

  // for keeping null terminators in Strings from non string literal c_strings
  static String nt_str_cstring(const char* cstr) {
    String result = cstr;
    return cstr;
  }

  static u64 str_find_next(char c, u64 offset, String str) {
    const u64 string_length = str.size;
    L_ASSERT(offset < string_length);
    for(u64 i = offset; i < string_length; i++) {
      if(str.str[i] == c) {
        return i;
      }
    }
    return MAX_u64;
  }

  static u64 str_find_first(char c, String str) {
    const u64 string_length = str.size;
    for(u64 i = 0; i < string_length; i++) {
      if(str.str[i] == c) {
        return i;
      }
    }
    return MAX_u64;
  }

  static u64 str_find_last(char c, String str) {
    for(u64 i = str.size - 1; i >= 0; i--) {
      if(str.str[i] == c) {
        return i;
      }
    }
    return MAX_u64;
  }

  static u64 str_find_next_last(char c, u64 offset, String str) {
    L_ASSERT(offset < str.size);
    for(u64 i = offset; i >= 0; i--) {
      if(str.str[i] == c) {
        return i;
      }
    }
    return MAX_u64;
  }

  static b8 str_copy(const String src, String* dst, u64 opt_clamp = MAX_u64) {
    if(dst->str == nullptr) {
      return false;
    }
    const u64 size_to_copy = CLAMP_TOP(opt_clamp, src.size);
    MEM_COPY(dst->str, src.str, size_to_copy);
    dst->size = size_to_copy;
    return true;
  }

  static b8 str_copy_nt(const String src, String* dst, u64 opt_clamp = MAX_u64) {
    if(dst->str == nullptr) {
      return false;
    }
    const u64 size_to_copy = CLAMP_TOP(opt_clamp, src.size);
    MEM_COPY(dst->str, src.str, size_to_copy);
    dst->size = size_to_copy;
    dst->str[dst->size] = '\0';
    return true;
  }

  static String str_cstring(const char* cstr) {
    size_t i = 0;
    while(cstr[i] != '\0') {
      i++;
    }
    return {(u8*)cstr, i};
  }

  static b8 str_cstring_copy(const char* src, String* dst, u64 opt_clamp = MAX_u64) {
    String result = str_cstring(src);
    return str_copy(result, dst, opt_clamp);
  }

  static b8 str_cstring_copy_nt(const char* src, String* dst, u64 opt_clamp = MAX_u64) {
    String result = nt_str_cstring(src);
    return str_copy_nt(result, dst, opt_clamp);
  }

  // always includes null terminator
  static b8 str_cstring_copy(const char* src, char* dst, u64* size) {
    if(*size == 0) {
      String result = nt_str_cstring(src);
      *size = result.size;
      return true;
    }
    MEM_COPY((void*)dst, (void*)src, sizeof(char) * (*size));
    return true;
  }

  static void cstr_copy(const char* src, char* dst, const u32 max_size) {
    u32 i = 0;
    while(src[i] != '\0') {
      dst[i] = src[i];
      if(i == max_size) {
        dst[max_size - 1] = '\0';
        break;
      }
    }
    if(i != max_size) {
      dst[i] = src[i];
    }
  }

  static b8 str_copy(String src, char* dst, u64 opt_clamp = MAX_u64) {
    String result = str((u8*)dst, 0);
    str_copy(src, &result, opt_clamp);
    return true;
  }

  static b8 str_copy_nt(String src, char* dst, u64 opt_clamp = MAX_u64) {
    String result = str((u8*)dst, 0);
    str_copy_nt(src, &result, opt_clamp);
    return true;
  }

#define str_lit(s) str((u8*)(s), sizeof(s)-1)

  static String str8_prefix(String str, u64 size) {
    u64 size_clamped = CLAMP_TOP(size, str.size);
    return {str.str, size_clamped};
  }

  static String str8_chop(String str, u64 amount) {
    u64 amount_clamped = CLAMP_TOP(amount, str.size);
    u64 remaining_size = str.size - amount_clamped;
    return {str.str, remaining_size};
  }

  static String str8_postfix(String str, u64 size) {
    u64 size_clamped = CLAMP_TOP(size, str.size);
    u64 remaining_size = str.size - size_clamped;
    return {str.str + remaining_size, size_clamped};
  }

  static String str8_skip(String str, u64 amount) {
    u64 amount_clamped = CLAMP_TOP(amount, str.size);
    u64 remaining_size = str.size = amount_clamped;
    return {str.str + amount_clamped, remaining_size};
  }

  static b32 str_compare(String str1, String str2) {
    const u64 str1_size = str1.size;
    if(str1_size == str2.size) {
      for(size_t i = 0; 
          i <= str1_size; 
          i++) {
        if(str1.str[i] == str2.str[i])
          continue;
        return false;
      }
      return true;
    }
    return false;
  }

  static b32 cstr_compare(const char* str1, const char* str2) {
    u64 index = 0;
    b32 result = true;
    while(true) {
      const char c1 = str1[index];
      const char c2 = str2[index];
      if(c1 != c2) {
        result = false;
        break;
      }
      if(c1 == '\0') {
        break;
      }
      index++;
    }
    return result;
  }

  static void str_list_push_explicit(StringList *list, String str, StringNode *node_memory) {
    node_memory->string = str;
    SLL_PUSH(list->first, list->last, node_memory);
    list->node_count += 1;
    list->total_size += str.size;
  }

#define str_expand(s) ((s).str), (u32)((s).size)

  template<typename ArenaT>
  void str_list_push(ArenaT* arena, StringList* list, String string) {
    StringNode* node = (StringNode*)arena->push(sizeof(StringNode));
    str_list_push_explicit(list, string, node);
  }

  template<typename ArenaT>
  String str_join(ArenaT* arena, StringList* list, StringJoin* opt_join) {
    static StringJoin dummy_join = {};
    if(opt_join == nullptr) {
      opt_join = &dummy_join;
    }
    u64 size = (opt_join->pre.size +
        opt_join->post.size +
        opt_join->mid.size*(list->node_count?list->node_count-1:0) +
        list->total_size);
    u8* str = (u8*)arena->push(size + 1);
    u8* ptr = str;
    MEM_COPY(ptr, opt_join->pre.str, opt_join->pre.size);
    ptr += opt_join->pre.size;
    b8 is_mid = false;

    for(StringNode* node = list->first;
        node != nullptr;
        node = node->next) {
      if(is_mid) {
        MEM_COPY(ptr, opt_join->mid.str, opt_join->mid.size);
        ptr += opt_join->mid.size;
      }
      MEM_COPY(ptr, node->string.str, node->string.size);
      ptr += node->string.size;
      if(opt_join->mid.size) {
        is_mid = true;
      }
    }
    MEM_COPY(ptr, opt_join->post.str, opt_join->post.size);
    ptr += opt_join->post.size;
    L_ASSERT(ptr == str + size);
    return {str, size};
  }

  template<typename ArenaT>
  StringList str_split(ArenaT* arena, String string, u8* split_chars, u32 count) {
    StringList result = {};
    u8* ptr = string.str;
    u8* word_first = ptr;
    u8* opl = string.str + string.size;
    for(;ptr < opl; ptr++) {
      u8 byte = *ptr;
      b8 is_split = false;
      for(u32 i = 0; i < count; i++) {
        if(byte == split_chars[i]) {
          is_split = true;
          break;
        }
      }
      if(is_split) {
        if(word_first < ptr) {
          str_list_push(arena, &result, str_range(word_first, ptr));
        }
        word_first = ptr + 1;
      }
    }
    if(word_first < ptr) {
      str_list_push(arena, &result, str_range(word_first, ptr));
    }
    return (result);
  }

  template<typename ArenaT>
  const char* cstr_str(ArenaT* arena, String string) {
    StringNode node;
    node.string = string;
    StringList list;
    list.first = &node;

    StringJoin join;
    join.post = String::null_at(arena);
    String result = str_join(arena, &list,&join);

    return (char*)result.str;
  }

  template<typename ArenaT>
  String str_pushfv(ArenaT* arena, const char* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);

    size_t buffer_size = 1024;
    u8* buffer = (u8*)arena->push(buffer_size);
    size_t actual_size = vsnprintf((char*)buffer, buffer_size, fmt, args);
    String result = {};

    if(actual_size < buffer_size) {
      arena->pop_amount(buffer_size - actual_size - 1);
      result = str(buffer, actual_size);
    } else {
      arena->pop_amount(buffer_size);
      u8* fixed_buffer = (u8*)arena->push(actual_size + 1);
      size_t final_size = vsnprintf((char*)fixed_buffer, actual_size + 1, fmt, args2);
      result = str(fixed_buffer, final_size);
    }

    va_end(args2);
    return (result);
  }

  template<typename ArenaT>
  String str_pushf(ArenaT* arena, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String result = str_pushfv(arena, fmt, args);
    va_end(args);
    return (result);
  }

  template<typename ArenaT>
  void str_list_pushf(ArenaT* arena, StringList* list, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String string = str_pushfv(arena, fmt, args);
    va_end(args);
    str_list_push(arena, list, string);
  }
}		// -----  end of namespace lofi  ----- 
