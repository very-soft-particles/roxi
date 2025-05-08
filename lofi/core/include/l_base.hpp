// =====================================================================================
//
//       Filename:  base.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-06 4:32:49 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "l_definition.hpp"
// typedefs
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef void void_func(void);

// helper macros

// asserting currently that there must be assertions!! until safe
#ifndef LOFI_ASSERT
#define LOFI_ASSERT
#endif

#define STMNT(s) do { s } while(0)
#if defined(COMPILER_CL)
#define L_ASSERT_BREAK() __debugbreak()
#else
#define L_ASSERT_BREAK() raise(SIGTERM)
#endif

#if defined(LOFI_ASSERT)
# define L_ASSERT(cond) STMNT( if (!(cond)) { L_ASSERT_BREAK(); } )
#else
# define L_ASSERT(cond)
#endif

#define STRINGIFY_(s) #s
#define STRINGIFY(string) STRINGIFY_(string)

#define GLUE_(a, b) a##b
#define GLUE(a, b) GLUE_(a, b)

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(*(array)))

#define PTR2INT(ptr) (unsigned long long)((char*)(ptr) - (char*)0)
#define INT2PTR(n) (void*)((char*)0 + (n))

#define MEMBER(type, member) (((type*)0)->member)
#define MEMBER_OFFSET(type, member) PTR2INT(&MEMBER(type, member))

#define MAX(a, b) (((a)>(b))?(a):(b))
#define MIN(a, b) (((a)<(b))?(a):(b))

#define CLAMP(a, x, b) (((x)<(a))?(a):\
                        ((b)<(x))?(b):(x))

#define CLAMP_TOP(a, b) MIN(a, b)
#define CLAMP_BOT(a, b) MAX(a, b)

#define ALIGN_POW2(ptr, pow) (((ptr)+(pow)-1)&~((pow)-1))
#define ALIGN_POW2_DOWN(ptr, pow) ((ptr)&~((pow)-1))

#define LERP(a, t, b) ((a)+((b)-(a))*(t))
#define UNLERP(a, x, b) (((a)==(b))?0.f:\
                         (((x)-(a))/((b)-(a))))

#define KB(n) ((n)<<10)
#define MB(n) ((n)<<20)
#define GB(n) ((n)<<30)
#define TB(n) ((n)<<40)

#define BIT(n) (1<<(n))

#define PRINT(fstr, ...) printf(fstr, __VA_ARGS__)

//#define local static
//#define global static
//#define function static

#define C_EXTERNAL_BEGIN extern "C" {
#define C_EXTERNAL_END }
#define C_EXTERNAL extern "C"

#define MEM_ZERO(ptr, size) memset((ptr), 0, (size))
#define MEM_ZERO_STRUCT(ptr) MEM_ZERO((ptr), sizeof(*(ptr)))
#define MEM_ZERO_ARRAY(ptr) MEM_ZERO((ptr), sizeof(ptr))
#define MEM_ZERO_COUNT(ptr, count) MEM_ZERO((ptr), sizeof(*(ptr))*(count))

#define MEM_FILL_ONES(ptr, size) memset((ptr), 0xff, (size))
#define MEM_FILL_ONES_STRUCT(ptr) MEM_FILL_ONES((ptr), sizeof(*(ptr)))
#define MEM_FILL_ONES_ARRAY(ptr) MEM_FILL_ONES((ptr), sizeof(ptr))
#define MEM_FILL_ONES_COUNT(ptr, count) MEM_FILL_ONES((ptr), sizeof(*(ptr))*(count))

#define MEM_MATCH(dst, src, size) (memcmp((dst),(src),(size)) == 0)

#define MEM_COPY(dst, src, size) memmove((dst), (src), (size))

#define MEM_COPY_STRUCT(dst, src) MEM_COPY((dst), (src), \
                                          MIN(sizeof(*(dst)), sizeof(*(src))))
#define MEM_COPY_ARRAY(dst, src) MEM_COPY((dst), (src), \
                                          MIN(sizeof(dst), sizeof(src)))
#define MEM_COPY_COUNT(dst, src, count) MEM_COPY((dst), (src), \
                                          MIN(sizeof(*(dst)), sizeof(*(src)))*(count))

#define EVAL_PRINT(s) PRINT("%s = %d\n", STRINGIFY(s), (i32)(s))
#define EVAL_PRINT_I(i) EVAL_PRINT(i)
#define EVAL_PRINT_LL(s) PRINT("%s = %lld\n", STRINGIFY(s), (i64)(s))
#define EVAL_PRINT_U(s) PRINT("%s = %u\n", STRINGIFY(s), (u32)(s))
#define EVAL_PRINT_ULL(s) PRINT("%s = %llu\n", STRINGIFY(s), (u64)(s))
#define EVAL_PRINT_F(s) PRINT("%s = %e [%f]\n", STRINGIFY(s), (f64)(s), (f64)(s))
#define EVAL_PRINT_B(s) PRINT("%s = %s\n", STRINGIFY(s), (char*)((s)?"true":"false"))
#define EVAL_PRINT_S(s) PRINT("%s = %s\n", STRINGIFY(s), (char*)(s))
#define PRINT_LINE(s) PRINT("%s\n", (char*)(s))
#define PRINT_S(s) PRINT("%s", (char*)(s))
#define PRINT_C(c) PRINT("%c", (char)(c))
#define PRINT_ENCLOSED(s, c) PRINT(                  \
    "%c%c%c%c%c%c%c%c%c%c%s%c%c%c%c%c%c%c%c%c%c\n", \
    c, c, c, c, c, c, c, c, c, c,                 \
    s, c, c, c, c, c, c, c, c, c, c)
#define PRINT_TITLE(s) PRINT_ENCLOSED(s, ':')

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
#define MOVE(obj) static_cast<decltype(obj)&&>(obj)

#define DLL_PUSH_BACK_NP(f,l,n,next,prev) ((f)==0?\
                                           (f)=(l)=(n),(n)->next=(n)->prev=0:\
                                          ((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=0))

#define DLL_PUSH_BACK(f,l,n) DLL_PUSH_BACK_NP(f,l,n,next,prev)
#define DLL_PUSH_FRONT(f,l,n) DLL_PUSH_BACK_NP(f,l,n,prev,next)

#define DLL_REMOVE_NP(f,l,n,next,prev) ((f)==(n)?\
                                       ((f)=(f)->next,(f)->prev=0):\
                                        (l)==(n)?\
                                       ((l)=(l)->prev,(l)->next=0):\
                                       ((n)->next->prev=(n)->prev,\
                                        (n)->prev->next=(n)->next))

#define DLL_REMOVE(f,l,n) DLL_REMOVE_NP(f,l,n,next,prev)

// singly linked list, maybe unnecessary, implementation commented out for now

#define SLL_PUSH(f,l,n) ((f)==0?\
                         (f)=(l)=(n),(n)->next=0:\
                        ((l)->next=(n),(l)=(n),(n)->next=0))

#define SLL_POP_FRONT(f,l,n) ((f)==(n)?\
                          ((f)=(f)->next):\
                           (l)==(n)?\
                          ((l)=(l)->prev,(l)->next=0):\
                          ((n)->next->prev=(n)->prev,\
                           (n)->prev->next=(n)->next))

#define FREE_LIST_INIT(l,c,s) for(size_t i=0;i<(c);i++)\
                              { (i!=(c)-1)?            \
                                  (l)[i].next=(l)+i+1:(l)[i].next=nullptr;\
                                ((l)[i].next->offset)=((l)[i].offset+(s));\
                              }
#define FREE_LIST_TAKE(f) ((f)=(f)->next)

#define FREE_LIST_GIVE(f,n) (n)->next=(f);(f)=(n)

#define FREE_LIST_CLEAR(l,c) for(size_t i=0;i<(c);i++)\
                              { (l)[i].next=nullptr;\
                                ((l)[i].next->offset)=0;\
                              }
// integer constants

static const i8  MIN_i8  = (i8)    0x80;
static const i16 MIN_i16 = (i16)   0x8000;
static const i32 MIN_i32 = (i32)   0x80000000;
static const i64 MIN_i64 = (i64)   0x8000000000000000llu;

static const i8  MAX_i8  = (i8)    0x7f;
static const i16 MAX_i16 = (i16)   0x7fff;
static const i32 MAX_i32 = (i32)   0x7fffffff;
static const i64 MAX_i64 = (i64)   0x7fffffffffffffffllu;

static const u8  MAX_u8  =  0xff;
static const u16 MAX_u16 =  0xffff;
static const u32 MAX_u32 =  0xffffffff;
static const u64 MAX_u64 =  0xffffffffffffffffllu;

static const f32 M_EPSILON_f32 = (f32) 1.1920929e-7f; 
static const f32 PI_f32        = (f32) 3.14159265359f; 
static const f32 TAU_f32       = (f32) 6.28318530718f;
static const f32 e_f32         = (f32) 2.71828182846f;
static const f32 PHI_HI_f32    = (f32) 1.61803398875f;
static const f32 PHI_LO_f32    = (f32) 0.61803398875f;

static const f64 M_EPSILON_f64 = (f64) 2.220446e-16; 
static const f64 PI_f64        = (f64) 3.14159265359; 
static const f64 TAU_f64       = (f64) 6.28318530718;
static const f64 e_f64         = (f64) 2.71828182846;
static const f64 PHI_HI_f64    = (f64) 1.61803398875;
static const f64 PHI_LO_f64    = (f64) 0.61803398875;

// float constant functions

static const f32 INF_f32() {
  union { f32 f; u32 u; } r;
  r.u = 0x7f800000;
  return (r.f);
}

static const f32 NEG_INF_f32() {
  union { f32 f; u32 u; } r;
  r.u = 0xff800000;
  return (r.f);
}

static const f64 INF_f64() {
  union { f64 f; u64 u; } r;
  r.u = 0x7ff0000000000000;
  return (r.f);
}

static const f64 NEG_INF_f64() {
  union { f64 f; u64 u; } r;
  r.u = 0xfff0000000000000;
  return (r.f);
}

static const f32 ABS_f32(const f32 v) {
  union { f32 f; u32 u; } r;
  r.f = v;
  r.u &= 0x7fffffff;
  return (r.f);
}

static const f64 ABS_f64(const f64 v) {
  union { f64 f; u64 u; } r;
  r.f = v;
  r.u &= 0x7fffffffffffffff;
  return (r.f);
}
// meta programming types
// many of these stolen from kvasir and/or from brigand
namespace lofi {


  namespace meta {
   
    struct InternalUseOnly {};

  }		// -----  end of namespace meta  ----- 


  template<typename>
  struct AlwaysFalse {
    enum {
      value = 0
    };
  };

  template<>
  struct AlwaysFalse<meta::InternalUseOnly> {
    enum {
      value = 1
    };
  };

  struct NotAType {};

  template<class T>
  struct Return {
    using type = T;
  };

  template<class T, T I>
  struct Value {
    using type = Value<T, I>;
    static constexpr T value = I;
  };

  // integer type for sequences

  template<auto I>
  using Int = Value<int, I>;

  template<auto I>
  using UInt = Value<unsigned, I>;

  template<auto I>
  using IdxT = Value<size_t, I>;

  template<i32 I>
  constexpr Int<I> IntV;

  template<u32 I>
  constexpr UInt<I> UIntV;

  template<size_t I>
  constexpr IdxT<I> IdxV;

  template<bool B>
  using Bool = Value<bool, B>;
  using TrueType = Bool<true>;
  using FalseType = Bool<false>;

}		// -----  end of namespace lofi  ----- 

