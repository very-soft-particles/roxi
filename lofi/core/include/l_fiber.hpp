// =====================================================================================
//
//       Filename:  fiber2.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-03-05 7:45:04 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include <atomic>
#include <emmintrin.h>
#include "l_vocab.hpp"
#include "l_sync.hpp"

#if COMPILER_CL
#define FORCENOINLINE __declspec(noinline)
#define DEFINE_FIBER_FUNC(name) __declspec(allocate(".text")) static unsigned char name[] = 
#elif (COMPILER_GCC)
#define FORCENOINLINE __attribute__((noinline))
#define DEFINE_FIBER_FUNC(name) __attribute__((section(".text#"))) static unsigned char name[] = 
#elif (COMPILER_CLANG)
#define FORCENOINLINE __attribute__((noinline))
#define DEFINE_FIBER_FUNC(name) __attribute__((section(".text#"))) static unsigned char name[] = 
#else
#define DEFINE_FIBER_FUNC(name) static unsigned char name[] = 

#endif

namespace lofi {
  namespace fiber {
    DEFINE_FIBER_FUNC(get_context_code) {
      0x4c, 0x8b, 0x04, 0x24,
      0x4c, 0x89, 0x07,
      0x4c, 0x8d, 0x44, 0x24, 0x08,
      0x4c, 0x89, 0x47, 0x08,
      0x48, 0x89, 0x5f, 0x10,
      0x48, 0x89, 0x6f, 0x18,
      0x4c, 0x89, 0x67, 0x20,
      0x4c, 0x89, 0x6f, 0x28,
      0x4c, 0x89, 0x77, 0x30,
      0x4c, 0x89, 0x7f, 0x38,
      0x32, 0xc0,
      0xc3 };

    DEFINE_FIBER_FUNC(set_context_code) {
      0x48, 0x8b, 0x67, 0x08,
      0x48, 0x8b, 0x5f, 0x10,
      0x48, 0x8b, 0x6f, 0x18,
      0x4c, 0x8b, 0x67, 0x20,
      0x4c, 0x8b, 0x6f, 0x28,
      0x4c, 0x8b, 0x77, 0x30,
      0x4c, 0x8b, 0x7f, 0x38,
      0x48, 0x8b, 0x7f, 0x40,
      0xff, 0x17 };

    DEFINE_FIBER_FUNC(swap_context_code) {
      0x4c, 0x8b, 0x04, 0x24,
      0x4c, 0x89, 0x07,
      0x4c, 0x8d, 0x44, 0x24, 0x08,     
      0x4c, 0x89, 0x47, 0x08,        
      0x48, 0x89, 0x5f, 0x10,        
      0x48, 0x89, 0x6f, 0x18,        
      0x4c, 0x89, 0x67, 0x20,        
      0x4c, 0x89, 0x6f, 0x28,        
      0x4c, 0x89, 0x77, 0x30,        
      0x4c, 0x89, 0x7f, 0x38,        
      0x48, 0x8b, 0x66, 0x08,        
      0x48, 0x8b, 0x5e, 0x10,        
      0x48, 0x8b, 0x6e, 0x18,        
      0x4c, 0x8b, 0x66, 0x20,        
      0x4c, 0x8b, 0x6e, 0x28,        
      0x4c, 0x8b, 0x76, 0x30,        
      0x4c, 0x8b, 0x7e, 0x38,        
      0x48, 0x8b, 0x7e, 0x40,        
      0xff, 0x17 };

    DEFINE_FIBER_FUNC(get_context_w_code) {
      0x4c, 0x8b, 0x04, 0x24,          
      0x4c, 0x89, 0x01,             
      0x4c, 0x8d, 0x44, 0x24, 0x08,       
      0x4c, 0x89, 0x41, 0x08,          
      0x48, 0x89, 0x59, 0x10,          
      0x48, 0x89, 0x69, 0x18,          
      0x4c, 0x89, 0x61, 0x20,          
      0x4c, 0x89, 0x69, 0x28,          
      0x4c, 0x89, 0x71, 0x30,          
      0x4c, 0x89, 0x79, 0x38,          
      0x48, 0x89, 0x79, 0x40,          
      0x48, 0x89, 0x71, 0x48,          
      0x0f, 0x11, 0x71, 0x50,          
      0x0f, 0x11, 0x79, 0x60,          
      0x44, 0x0f, 0x11, 0x41, 0x70,       
      0x44, 0x0f, 0x11, 0x89, 0x80, 0x00, 0x00, 
      0x00,
      0x44, 0x0f, 0x11, 0x91, 0x90, 0x00, 0x00, 
      0x00,
      0x44, 0x0f, 0x11, 0x99, 0xa0, 0x00, 0x00, 
      0x00,
      0x44, 0x0f, 0x11, 0xa1, 0xb0, 0x00, 0x00, 
      0x00,
      0x44, 0x0f, 0x11, 0xa9, 0xc0, 0x00, 0x00, 
      0x00,
      0x44, 0x0f, 0x11, 0xb1, 0xd0, 0x00, 0x00, 
      0x00,
      0x44, 0x0f, 0x11, 0xb9, 0xe0, 0x00, 0x00, 
      0x00,
      0x31, 0xc0,
      0xc3 };

    DEFINE_FIBER_FUNC(set_context_w_code) {
      0x48, 0x8b, 0x61, 0x08,
      0x48, 0x8b, 0x59, 0x10,
      0x48, 0x8b, 0x69, 0x18,
      0x4c, 0x8b, 0x61, 0x20,
      0x4c, 0x8b, 0x69, 0x28,
      0x4c, 0x8b, 0x71, 0x30,
      0x4c, 0x8b, 0x79, 0x38,
      0x48, 0x8b, 0x79, 0x40,
      0x48, 0x8b, 0x71, 0x48,
      0x0f, 0x10, 0x71, 0x58,
      0x0f, 0x10, 0x79, 0x68,
      0x44, 0x0f, 0x10, 0x41, 0x78,
      0x44, 0x0f, 0x10, 0x89, 0x88, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0x91, 0x98, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0x99, 0xa8, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xa1, 0xb8, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xa9, 0xc8, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xb1, 0xd8, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xb9, 0xe8, 0x00, 0x00,
      0x00,
      0x48, 0x8b, 0x49, 0x50,         
      0xff, 17 };

    //DEFINE_FIBER_FUNC(swap_context_w_code) {
    //  0x4c, 0x8b, 0x04, 0x24,         
    //  0x4c, 0x89, 0x01,            
    //  0x4c, 0x8d, 0x44, 0x24, 0x08,      
    //  0x4c, 0x89, 0x41, 0x08,         
    //  0x48, 0x89, 0x59, 0x10,         
    //  0x48, 0x89, 0x69, 0x18,         
    //  0x4c, 0x89, 0x61, 0x20,         
    //  0x4c, 0x89, 0x69, 0x28,         
    //  0x4c, 0x89, 0x71, 0x30,         
    //  0x4c, 0x89, 0x79, 0x38,         
    //  0x48, 0x89, 0x79, 0x40,         
    //  0x48, 0x89, 0x71, 0x48,         
    //  0x0f, 0x11, 0x71, 0x58,         
    //  0x0f, 0x11, 0x79, 0x68,         
    //  0x44, 0x0f, 0x11, 0x41, 0x78,      
    //  0x44, 0x0f, 0x11, 0x89, 0x88, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0x91, 0x98, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0x99, 0xa8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xa1, 0xb8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xa9, 0xc8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xb1, 0xd8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xb9, 0xe8, 0x00, 0x00,
    //  0x00,
    //  0x48, 0x8b, 0x62, 0x08,         
    //  0x48, 0x8b, 0x5a, 0x10,         
    //  0x48, 0x8b, 0x6a, 0x18,         
    //  0x4c, 0x8b, 0x62, 0x20,         
    //  0x4c, 0x8b, 0x6a, 0x28,         
    //  0x4c, 0x8b, 0x72, 0x30,         
    //  0x4c, 0x8b, 0x7a, 0x38,         
    //  0x48, 0x8b, 0x7a, 0x40,         
    //  0x48, 0x8b, 0x72, 0x48,         
    //  0x0f, 0x10, 0x72, 0x58,         
    //  0x0f, 0x10, 0x7a, 0x68,         
    //  0x44, 0x0f, 0x10, 0x42, 0x78,      
    //  0x44, 0x0f, 0x10, 0x8a, 0x88, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0x92, 0x98, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0x9a, 0xa8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xa2, 0xb8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xaa, 0xc8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xb2, 0xd8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xba, 0xe8, 0x00, 0x00,
    //  0x00,
    //  0x48, 0x8b, 0x4a, 0x50,         
    //  0x48, 0x21, 0x24, 0x25, 0xf0, 0xff, 0xff,
    //  0xff,
    //  0x48, 0x29, 0x24, 0x25, 0x08, 0x00, 0x00,
    //  0x00,
    //  0xff, 0x12,               
    //  0x48, 0x01, 0x24, 0x25, 0x08, 0x00, 0x00,
    //  0x00,
    //  0xc3
    //  };

    // swap with call and alignment
    DEFINE_FIBER_FUNC(swap_context_w_code) {
      0x4c, 0x8b, 0x04, 0x24,         
      0x4c, 0x89, 0x01,            
      0x4c, 0x8d, 0x44, 0x24, 0x08,  
      0x4c, 0x89, 0x41, 0x08,         
      0x48, 0x89, 0x59, 0x10,         
      0x48, 0x89, 0x69, 0x18,         
      0x4c, 0x89, 0x61, 0x20,         
      0x4c, 0x89, 0x69, 0x28,         
      0x4c, 0x89, 0x71, 0x30,         
      0x4c, 0x89, 0x79, 0x38,         
      0x48, 0x89, 0x79, 0x40,         
      0x48, 0x89, 0x71, 0x48,         
      0x0f, 0x11, 0x71, 0x50,         
      0x0f, 0x11, 0x79, 0x60,         
      0x44, 0x0f, 0x11, 0x41, 0x70,      
      0x44, 0x0f, 0x11, 0x89, 0x80, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0x91, 0x90, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0x99, 0xa0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xa1, 0xb0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xa9, 0xc0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xb1, 0xd0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xb9, 0xe0, 0x00, 0x00,
      0x00,
      0x48, 0x8b, 0x62, 0x08,         
      0x48, 0x8b, 0x5a, 0x10,         
      0x48, 0x8b, 0x6a, 0x18,         
      0x4c, 0x8b, 0x62, 0x20,         
      0x4c, 0x8b, 0x6a, 0x28,         
      0x4c, 0x8b, 0x72, 0x30,         
      0x4c, 0x8b, 0x7a, 0x38,         
      0x48, 0x8b, 0x7a, 0x40,         
      0x48, 0x8b, 0x72, 0x48,         
      0x0f, 0x10, 0x72, 0x50,         
      0x0f, 0x10, 0x7a, 0x60,         
      0x44, 0x0f, 0x10, 0x42, 0x70,      
      0x44, 0x0f, 0x10, 0x8a, 0x80, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0x92, 0x90, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0x9a, 0xa0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xa2, 0xb0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xaa, 0xc0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xb2, 0xd0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xba, 0xe0, 0x00, 0x00,
      0x00,
      0x48, 0x8b, 0x8a, 0xf0, 0x00, 0x00, 0x00,
      0xff, 0x22
      };

    // yield context without and alignment
    DEFINE_FIBER_FUNC(yield_context_w_code) {
      0x4c, 0x8b, 0x04, 0x24,         
      0x4c, 0x89, 0x01,            
      0x4c, 0x8d, 0x44, 0x24, 0x08,      
      0x4c, 0x89, 0x41, 0x08,         
      0x48, 0x89, 0x59, 0x10,         
      0x48, 0x89, 0x69, 0x18,         
      0x4c, 0x89, 0x61, 0x20,         
      0x4c, 0x89, 0x69, 0x28,         
      0x4c, 0x89, 0x71, 0x30,         
      0x4c, 0x89, 0x79, 0x38,         
      0x48, 0x89, 0x79, 0x40,         
      0x48, 0x89, 0x71, 0x48,         
      0x0f, 0x11, 0x71, 0x50,         
      0x0f, 0x11, 0x79, 0x60,         
      0x44, 0x0f, 0x11, 0x41, 0x70,      
      0x44, 0x0f, 0x11, 0x89, 0x80, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0x91, 0x90, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0x99, 0xa0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xa1, 0xb0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xa9, 0xc0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xb1, 0xd0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x11, 0xb9, 0xe0, 0x00, 0x00,
      0x00,
      0x4c, 0x8b, 0x02,            
      0x48, 0x8b, 0x62, 0x08,         
      0x48, 0x8b, 0x5a, 0x10,         
      0x48, 0x8b, 0x6a, 0x18,         
      0x4c, 0x8b, 0x62, 0x20,         
      0x4c, 0x8b, 0x6a, 0x28,         
      0x4c, 0x8b, 0x72, 0x30,         
      0x4c, 0x8b, 0x7a, 0x38,         
      0x48, 0x8b, 0x7a, 0x40,         
      0x48, 0x8b, 0x72, 0x48,         
      0x0f, 0x10, 0x72, 0x50,         
      0x0f, 0x10, 0x7a, 0x60,         
      0x44, 0x0f, 0x10, 0x42, 0x70,      
      0x44, 0x0f, 0x10, 0x8a, 0x80, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0x92, 0x90, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0x9a, 0xa0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xa2, 0xb0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xaa, 0xc0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xb2, 0xd0, 0x00, 0x00,
      0x00,
      0x44, 0x0f, 0x10, 0xba, 0xe0, 0x00, 0x00,
      0x00,
      0x48, 0x8b, 0x8a, 0xf0, 0x00, 0x00, 0x00,         
      0x41, 0x50,               
      0xc3                  
    };

    // return function with and alignment
    //DEFINE_FIBER_FUNC(swap_context_w_code) {
    //  0x4c, 0x8b, 0x04, 0x24,         
    //  0x4c, 0x89, 0x01,            
    //  0x4c, 0x8d, 0x44, 0x24, 0x08,      
    //  0x4c, 0x89, 0x41, 0x08,         
    //  0x48, 0x89, 0x59, 0x10,         
    //  0x48, 0x89, 0x69, 0x18,         
    //  0x4c, 0x89, 0x61, 0x20,         
    //  0x4c, 0x89, 0x69, 0x28,         
    //  0x4c, 0x89, 0x71, 0x30,         
    //  0x4c, 0x89, 0x79, 0x38,         
    //  0x48, 0x89, 0x79, 0x40,         
    //  0x48, 0x89, 0x71, 0x48,         
    //  0x0f, 0x11, 0x71, 0x58,         
    //  0x0f, 0x11, 0x79, 0x68,         
    //  0x44, 0x0f, 0x11, 0x41, 0x78,      
    //  0x44, 0x0f, 0x11, 0x89, 0x88, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0x91, 0x98, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0x99, 0xa8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xa1, 0xb8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xa9, 0xc8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xb1, 0xd8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x11, 0xb9, 0xe8, 0x00, 0x00,
    //  0x00,
    //  0x4c, 0x8b, 0x02,            
    //  0x48, 0x8b, 0x62, 0x08,         
    //  0x48, 0x8b, 0x5a, 0x10,         
    //  0x48, 0x8b, 0x6a, 0x18,         
    //  0x4c, 0x8b, 0x62, 0x20,         
    //  0x4c, 0x8b, 0x6a, 0x28,         
    //  0x4c, 0x8b, 0x72, 0x30,         
    //  0x4c, 0x8b, 0x7a, 0x38,         
    //  0x48, 0x8b, 0x7a, 0x40,         
    //  0x48, 0x8b, 0x72, 0x48,         
    //  0x0f, 0x10, 0x72, 0x58,         
    //  0x0f, 0x10, 0x7a, 0x68,         
    //  0x44, 0x0f, 0x10, 0x42, 0x78,      
    //  0x44, 0x0f, 0x10, 0x8a, 0x88, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0x92, 0x98, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0x9a, 0xa8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xa2, 0xb8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xaa, 0xc8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xb2, 0xd8, 0x00, 0x00,
    //  0x00,
    //  0x44, 0x0f, 0x10, 0xba, 0xe8, 0x00, 0x00,
    //  0x00,
    //  0x48, 0x8b, 0x4a, 0x50,         
    //  0x48, 0x21, 0x24, 0x25, 0xf0, 0xff, 0xff,
    //  0xff,
    //  0x41, 0x50,               
    //  0xc3                  
    //};

    struct context {
#if (OS_WINDOWS)
      void *rip = nullptr, *rsp = nullptr;
      void *rbx = nullptr, *rbp = nullptr, *r12 = nullptr, *r13 = nullptr, *r14 = nullptr, *r15 = nullptr, *rdi = nullptr, *rsi = nullptr;
      __m128i xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      void* arg = nullptr;
#elif (OS_LINUX || OS_MAC)
      void *rip = nullptr, *rsp = nullptr;
      void *rbx = nullptr, *rbp = nullptr, *r12 = nullptr, *r13 = nullptr, *r14 = nullptr, *r15 = nullptr;
      void *arg = nullptr;
#else
      void *rip = nullptr, *rsp = nullptr, *arg = nullptr;
#endif
    };

#if OS_WINDOWS
    static void (*get_context)(context*) = (void(*)(context*))get_context_w_code;
#elif (OS_LINUX || OS_MAC)
    static void (*get_context)(context*) = (void(*)(context*))get_context_code;
#endif

#if OS_WINDOWS
    static void (*set_context)(context*) = (void(*)(context*))set_context_w_code;
#elif (OS_LINUX || OS_MAC)
    static void (*set_context)(context*) = (void(*)(context*))set_context_code;
#endif

#if OS_WINDOWS
    static void (*swap_context)(context* from_context, context* to_context) = (void(*)(context*, context*))swap_context_w_code;
    static void (*yield_context)(context* from_context, context* to_context) = (void(*)(context*, context*))yield_context_w_code;
#elif (OS_LINUX || OS_MAC)
    static void (*swap_context)(context* from_context, context* to_context) = (void(*)(context*, context*))swap_context_code;
#endif

    template<size_t StackSize>
    static void* do_align(void* stack) {
      u8* temp = (u8*)stack + StackSize;
      temp = (u8*)INT2PTR(ALIGN_POW2_DOWN(PTR2INT(temp), 16));
    
      return (void*)temp;
    }

    template<size_t StackSize>
    static void create_fiber_context(void(*start_func)(void*), void* arg, void* stack, context* context) {
      u8* temp = (u8*)stack + StackSize;
      temp = (u8*)INT2PTR(ALIGN_POW2_DOWN(PTR2INT(temp), 16));
      temp -= 128;
 

      context->rip = (void*)start_func;
      context->rsp = temp;
      context->arg = arg;
    }

    static void convert_thread_to_fiber(context* _context) {
      get_context(_context);
    }

    static void run(context* _context) {
      set_context(_context);
    }

    static void FORCENOINLINE swap_fiber(context* switch_from_fiber, context* switch_to_fiber) {
      swap_context(switch_from_fiber, switch_to_fiber);
    }

    static void FORCENOINLINE yield_fiber(context* switch_from_fiber, context* switch_to_fiber) {
      yield_context(switch_from_fiber, switch_to_fiber);
    }
  }		// -----  end of namespace fiber  ----- 


  template<u64 StackSize>
  struct Fiber;

  template<u64 StackSize>
  using FiberHandle = Fiber<StackSize>*;

  template<u64 StackSize>
  class Fiber {
  private:
    using FiberHandle = FiberHandle<StackSize>;
    FiberHandle prev = nullptr;
    fiber::context _context;
    void* stack = nullptr;
    std::atomic_flag waiting = ATOMIC_FLAG_INIT;
  public:
    Fiber() {
      fiber::get_context(&_context);
    }
    Fiber(void(*main_func)(void*), void* arg) {
      _context.rip = (void*)main_func;
      _context.arg = arg;
    }
 
    Fiber(const Fiber&) = delete;
    Fiber(Fiber&&) = delete;

    Fiber(fiber::context from_context) : _context{from_context} {};
    Fiber(void(*main_func)(void*), void* arg, void* new_stack) {
      stack = new_stack;
      fiber::create_fiber_context<StackSize>(main_func, arg, new_stack, &_context);
    }

    b8 operator==(FiberHandle other) {
      if(other->_context.rsp == nullptr) {
        //PRINT_S("RSP IS NULL... RETURNING FALSE\n");
        return false;
      }
      void* stack_ptr = _context.rsp;
      if(stack == nullptr || stack_ptr == nullptr) {
        //PRINT("STACK IS %llu... OTHER STACK IS %llu RETURNING FALSE\n", stack, stack_ptr);
        return false;
      }
      void* other_stack_ptr = other->_context.rsp;

      //PRINT("CHECKING FIBER EQUALITY... FIBER %llu, OTHER FIBER %llu\n", stack, other_stack_ptr);
      if(other_stack_ptr <= ((u8*)stack + StackSize) && other_stack_ptr > stack) {
        //PRINT("FIBER %llu, OWNS FIBER %llu... RETURNING TRUE\n", stack, other_stack_ptr);
        return true;
      }
      //PRINT("FIBER %llu, DOES NOT OWN FIBER %llu\n", stack, other_stack_ptr);
      return false;
    }

    FiberHandle set_argument(void* new_arg) {
      _context.arg = new_arg;
      return this;
    }

    FiberHandle run(void* arg) {
      _context.arg = arg;
      fiber::run(&_context);
      return this;
    }

    FiberHandle run() {
      fiber::run(&_context);
      return this;
    }

    FiberHandle swap(FiberHandle previous) {
      prev = previous;
      //if(waiting.test(std::memory_order_acquire)) {
      //  //_context.rsp = (u8*)INT2PTR(ALIGN_POW2_DOWN(PTR2INT(_context.rsp), 16) - 8);
      //  //_context.arg = prev->_context.arg;
      //  fiber::yield_fiber(&prev->_context, &_context);
      //  return this;
      //}
      fiber::swap_fiber(&prev->_context, &_context);
      return this;
    }

    void set_previous(const FiberHandle previous) {
      prev = previous;
    }

    const void* get_current_stack_ptr() const {
      return _context.rsp;
    }

    void clear() {
      prev = nullptr;
      _context = fiber::context{};
      stack = nullptr;
      waiting.clear(std::memory_order_release);
    }

    const b8 is_waiting() const {
      return waiting.test(std::memory_order_acquire);
    }

    FiberHandle yield() {
      if(!prev) {
        return nullptr;
      }
      fiber::swap_fiber(&_context, &prev->_context);
      //PRINT_S("SWAPPED FIBER RETURNED FROM********\n");
      return this;
    }

    void* get_stack_base() const {
      return stack;
    }

    FiberHandle wait() {
      waiting.test_and_set(std::memory_order_release);
      yield();
      waiting.clear(std::memory_order_release);
      return this;
    }
  };

}		// -----  end of namespace lofi  ----- 

//#if ARCH_ARM64
//    #define FIBER_ARM64
//#elif defined(__x86_64__) || defined(_M_X64)
//    #define FIBER_X64
//#else
//    #error "Fibers not supported on target architecture."
//#endif
//
//#if OS_MAC
//    #define FIBER_APPLE
//#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//    #define FIBER_WIN
//#elif defined(__linux__)
//    #define FIBER_LINUX
//#else
//    #error "Fibers not supported on target Platform."
//#endif
//
//#if defined(FIBER_ARM64)
//    #define FIBER_ASM_ARM64_IMPLEMENTATION
//#elif defined(FIBER_X64)
//    #define FIBER_ASM_X64_IMPLEMENTATION
//#endif
//
//#if defined(FIBER_MALLOC) && defined(FIBER_FREE)
//// custom stack allocation is fine
//#elif !defined(FIBER_MALLOC) && !defined(FIBER_FREE)
//// default stack allocation is fine either
//#else
//#error "Do not define either of TINY_FIBER_MALLOC and TINY_FIBER_FREE"
//#endif
//
//#if defined(FIBER_WIN)
//#include <Windows.h>
//#endif
//
//#ifndef FIBER_MALLOC
//#define FIBER_MALLOC   malloc
//#define FIBER_FREE     free
//#endif
//
//namespace lofi {
//// an alias for CPU registers, all need to be 64 bits long
//typedef u64 Register;
//
//// check the register size, making sure it is 8 bytes.
//static_assert(sizeof(Register) == 8, "Incorrect register size");
//
//// helper macro to check the offset of a register in fiber context
//#define CHECK_OFFSET(REGISTER, OFFSET)    \
//    static_assert(offsetof(FiberContext, REGISTER) == OFFSET, \
//                  "Incorrect register offset")
//
//// Both x64 and arm64 require stack memory pointer to be 16 bytes aligned
//#define FIBER_STACK_ALIGNMENT    16
//
//// MacOS requires a _ to be defined at the beginning of a function name implemented in asm
//#if defined(FIBER_APPLE)
//    #define _switch_fiber_internal switch_fiber_internal
//#endif
//
//#ifdef FIBER_ASM_ARM64_IMPLEMENTATION
//
//#define FIBER_ASM_IMPLEMENTATION
//
//// alias for registers that need to be saved during a fiber switch
//#define FIBER_REG_X0    0x00
//#define FIBER_REG_X16   0x08
//#define FIBER_REG_X17   0x10
//#define FIBER_REG_X18   0x18
//#define FIBER_REG_X19   0x20
//#define FIBER_REG_X20   0x28
//#define FIBER_REG_X21   0x30
//#define FIBER_REG_X22   0x38
//#define FIBER_REG_X23   0x40
//#define FIBER_REG_X24   0x48
//#define FIBER_REG_X25   0x50
//#define FIBER_REG_X26   0x58
//#define FIBER_REG_X27   0x60
//#define FIBER_REG_X28   0x68
//#define FIBER_REG_X29   0x70
//#define FIBER_REG_X30   0x78
//#define FIBER_REG_V8    0x80
//#define FIBER_REG_V9    0x88
//#define FIBER_REG_V10   0x90
//#define FIBER_REG_V11   0x98
//#define FIBER_REG_V12   0xa0
//#define FIBER_REG_V13   0xa8
//#define FIBER_REG_V14   0xb0
//#define FIBER_REG_V15   0xb8
//#define FIBER_REG_SP    0xc0
//
//    /**
//     * On Arm 64 architecture, each register should be 64 bits. And following registers need to be
//     * saved during a context switch
//     *   - [X16 - X30]
//     *     https://developer.arm.com/documentation/102374/0101/Procedure-Call-Standard
//     *   - [D16 - D23]
//     *     https://developer.arm.com/documentation/den0024/a/The-ABI-for-ARM-64-bit-Architecture/Register-use-in-the-AArch64-Procedure-Call-Standard/Parameters-in-NEON-and-floating-point-registers
//     */
//    struct FiberContext {
//      // funciton paramters
//      Register    x0;
//
//      // general purpose registers
//      Register    x16;
//      Register    x17;
//      Register    x18;
//      Register    x19;
//      Register    x20;
//      Register    x21;
//      Register    x22;
//      Register    x23;
//      Register    x24;
//      Register    x25;
//      Register    x26;
//      Register    x27;
//      Register    x28;
//      Register    x29;    // frame pointer (FP)
//      Register    x30;    // link register (R30)
//
//      // float-point and neon registers
//      Register    v8;
//      Register    v9;
//      Register    v10;
//      Register    v11;
//      Register    v12;
//      Register    v13;
//      Register    v14;
//      Register    v15;
//
//      // stack pointer
//      Register    sp;     
//    };
//
//  // Making sure our offsets are all correct
//  CHECK_OFFSET(x0,  FIBER_REG_X0);
//  CHECK_OFFSET(x16, FIBER_REG_X16);
//  CHECK_OFFSET(x17, FIBER_REG_X17);
//  CHECK_OFFSET(x18, FIBER_REG_X18);
//  CHECK_OFFSET(x19, FIBER_REG_X19);
//  CHECK_OFFSET(x20, FIBER_REG_X20);
//  CHECK_OFFSET(x21, FIBER_REG_X21);
//  CHECK_OFFSET(x22, FIBER_REG_X22);
//  CHECK_OFFSET(x23, FIBER_REG_X23);
//  CHECK_OFFSET(x24, FIBER_REG_X24);
//  CHECK_OFFSET(x25, FIBER_REG_X25);
//  CHECK_OFFSET(x26, FIBER_REG_X26);
//  CHECK_OFFSET(x27, FIBER_REG_X27);
//  CHECK_OFFSET(x28, FIBER_REG_X28);
//  CHECK_OFFSET(x29, FIBER_REG_X29);
//  CHECK_OFFSET(x30, FIBER_REG_X30);
//  CHECK_OFFSET(v8,  FIBER_REG_V8);
//  CHECK_OFFSET(v9,  FIBER_REG_V9);
//  CHECK_OFFSET(v10, FIBER_REG_V10);
//  CHECK_OFFSET(v11, FIBER_REG_V11);
//  CHECK_OFFSET(v12, FIBER_REG_V12);
//  CHECK_OFFSET(v13, FIBER_REG_V13);
//  CHECK_OFFSET(v14, FIBER_REG_V14);
//  CHECK_OFFSET(v15, FIBER_REG_V15);
//  CHECK_OFFSET(sp,  FIBER_REG_SP);
//
//  // create a new fiber
//  extern "C" inline bool _create_fiber_internal(void* stack, uint32_t stack_size, void (*target)(void*), void* arg, FiberContext* context) {
//    // it is the users responsibility to make sure the stack is 16 bytes aligned, which is required by the Arm64 architecture
//    if((((uintptr_t)stack) & (FIBER_STACK_ALIGNMENT - 1)) != 0)
//      return false;
//
//    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
//    context->x30 = (uintptr_t)target;
//    context->x0 = (uintptr_t)arg;
//    context->sp = (uintptr_t)stack_top;
//
//    return true;
//  }
//
//  // switching to a different fiber
//  asm(
//      R"(
//.text
//.align 4
//_switch_fiber_internal:
//    str x16, [x0, 0x08] /* FIBER_REG_X16 */
//    str x17, [x0, 0x10] /* FIBER_REG_X17 */
//    str x18, [x0, 0x18] /* FIBER_REG_X18 */
//    str x19, [x0, 0x20] /* FIBER_REG_X19 */
//    str x20, [x0, 0x28] /* FIBER_REG_X20 */
//    str x21, [x0, 0x30] /* FIBER_REG_X21 */
//    str x22, [x0, 0x38] /* FIBER_REG_X22 */
//    str x23, [x0, 0x40] /* FIBER_REG_X23 */
//    str x24, [x0, 0x48] /* FIBER_REG_X24 */
//    str x25, [x0, 0x50] /* FIBER_REG_X25 */
//    str x26, [x0, 0x58] /* FIBER_REG_X26 */
//    str x27, [x0, 0x60] /* FIBER_REG_X27 */
//    str x28, [x0, 0x68] /* FIBER_REG_X28 */
//    str x29, [x0, 0x70] /* FIBER_REG_X29 */
//    str d8,  [x0, 0x80] /* FIBER_REG_V8 */
//    str d9,  [x0, 0x88] /* FIBER_REG_V9 */
//    str d10, [x0, 0x90] /* FIBER_REG_V10 */
//    str d11, [x0, 0x98] /* FIBER_REG_V11 */
//    str d12, [x0, 0xa0] /* FIBER_REG_V12 */
//    str d13, [x0, 0xa8] /* FIBER_REG_V13 */
//    str d14, [x0, 0xb0] /* FIBER_REG_V14 */
//    str d15, [x0, 0xb8] /* FIBER_REG_V15 */
//    // store sp and lr
//    mov x2, sp
//    str x2,  [x0, 0xc0] /* FIBER_REG_SP */
//    str x30, [x0, 0x78] /* FIBER_REG_X30 */
//    // load context 'to'
//    mov x7, x1
//    ldr x16, [x7, 0x08] /* FIBER_REG_X16 */
//    ldr x17, [x7, 0x10] /* FIBER_REG_X17 */
//    ldr x18, [x7, 0x18] /* FIBER_REG_X18 */
//    ldr x19, [x7, 0x20] /* FIBER_REG_X19 */
//    ldr x20, [x7, 0x28] /* FIBER_REG_X20 */
//    ldr x21, [x7, 0x30] /* FIBER_REG_X21 */
//    ldr x22, [x7, 0x38] /* FIBER_REG_X22 */
//    ldr x23, [x7, 0x40] /* FIBER_REG_X23 */
//    ldr x24, [x7, 0x48] /* FIBER_REG_X24 */
//    ldr x25, [x7, 0x50] /* FIBER_REG_X25 */
//    ldr x26, [x7, 0x58] /* FIBER_REG_X26 */
//    ldr x27, [x7, 0x60] /* FIBER_REG_X27 */
//    ldr x28, [x7, 0x68] /* FIBER_REG_X28 */
//    ldr x29, [x7, 0x70] /* FIBER_REG_X29 */
//    ldr d8,  [x7, 0x80] /* FIBER_REG_V8 */
//    ldr d9,  [x7, 0x88] /* FIBER_REG_V9 */
//    ldr d10, [x7, 0x90] /* FIBER_REG_V10 */
//    ldr d11, [x7, 0x98] /* FIBER_REG_V11 */
//    ldr d12, [x7, 0xa0] /* FIBER_REG_V12 */
//    ldr d13, [x7, 0xa8] /* FIBER_REG_V13 */
//    ldr d14, [x7, 0xb0] /* FIBER_REG_V14 */
//    ldr d15, [x7, 0xb8] /* FIBER_REG_V15 */
//    // Load first parameter, this is only used for the first time a fiber gains control
//    ldr x0, [x7, 0x00]  /* FIBER_REG_X0 */
//    ldr x30, [x7, 0x78] /* FIBER_REG_X30 */
//    ldr x2,  [x7, 0xc0] /* FIBER_REG_SP */
//    mov sp, x2
//    ret
//)");
//
//#elif defined(FIBER_ASM_X64_IMPLEMENTATION)
//
//#define FIBER_ASM_IMPLEMENTATION
//
//#define FIBER_REG_RBX 0x00
//#define FIBER_REG_RBP 0x08
//#define FIBER_REG_R12 0x10
//#define FIBER_REG_R13 0x18
//#define FIBER_REG_R14 0x20
//#define FIBER_REG_R15 0x28
//#define FIBER_REG_RDI 0x30
//#define FIBER_REG_RSP 0x38
//#define FIBER_REG_RIP 0x40
//
//    //! Fiber context that saves all the callee saved registers
//    /**
//     * The specific set of register is architecture and OS dependent. SORT uses this implementation for
//     * Intel Mac and X64 Ubuntu, which use 'System V AMD64 ABI'.
//     * https://en.wikipedia.org/wiki/X86_calling_conventions#cite_note-AMD-28
//     * 
//     * System V Application Binary Interface AMD64 Architecture Processor Supplement
//     * Page 23, AMD64 ABI Draft 1.0
//     * https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-1.0.pdf
//     */
//    struct FiberContext {
//      // callee-saved registers
//      Register rbx;
//      Register rbp;
//      Register r12;
//      Register r13;
//      Register r14;
//      Register r15;
//
//      // parameter registers
//      Register rdi;
//
//      // stack and instruction registers
//      Register rsp;
//      Register rip;
//    };
//
//  CHECK_OFFSET(rbx, FIBER_REG_RBX);
//  CHECK_OFFSET(rbp, FIBER_REG_RBP);
//  CHECK_OFFSET(r12, FIBER_REG_R12);
//  CHECK_OFFSET(r13, FIBER_REG_R13);
//  CHECK_OFFSET(r14, FIBER_REG_R14);
//  CHECK_OFFSET(r15, FIBER_REG_R15);
//  CHECK_OFFSET(rdi, FIBER_REG_RDI);
//  CHECK_OFFSET(rsp, FIBER_REG_RSP);
//  CHECK_OFFSET(rip, FIBER_REG_RIP);
//
//  // create a new fiber
//  inline bool _create_fiber_internal(void* stack, uint32_t stack_size, void (*target)(void*), void* arg, FiberContext* context) {
//    // it is the users responsibility to make sure the stack is 16 bytes aligned, which is required by the Arm64 architecture
//    if((((uintptr_t)stack) & (FIBER_STACK_ALIGNMENT - 1)) != 0)
//      return false;
//
//    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
//    context->rip = (uintptr_t)target;
//    context->rdi = (uintptr_t)arg;
//    context->rsp = (uintptr_t)&stack_top[-3];
//    stack_top[-2] = 0;
//
//    return true;
//  }
//
//  // switching to a different fiber
//  asm(
//      R"(
//.text
//.align 4
//_switch_fiber_internal:
//    // Save context 'from'
//    // Store callee-preserved registers
//    movq        %rbx, 0x00(%rdi) /* FIBER_REG_RBX */
//    movq        %rbp, 0x08(%rdi) /* FIBER_REG_RBP */
//    movq        %r12, 0x10(%rdi) /* FIBER_REG_R12 */
//    movq        %r13, 0x18(%rdi) /* FIBER_REG_R13 */
//    movq        %r14, 0x20(%rdi) /* FIBER_REG_R14 */
//    movq        %r15, 0x28(%rdi) /* FIBER_REG_R15 */
//    /* call stores the return address on the stack before jumping */
//    movq        (%rsp), %rcx             
//    movq        %rcx, 0x40(%rdi) /* FIBER_REG_RIP */
//
//    /* skip the pushed return address */
//    leaq        8(%rsp), %rcx            
//    movq        %rcx, 0x38(%rdi) /* FIBER_REG_RSP */
//    // Load context 'to'
//    movq        %rsi, %r8
//    // Load callee-preserved registers
//    movq        0x00(%r8), %rbx /* FIBER_REG_RBX */
//    movq        0x08(%r8), %rbp /* FIBER_REG_RBP */
//    movq        0x10(%r8), %r12 /* FIBER_REG_R12 */
//    movq        0x18(%r8), %r13 /* FIBER_REG_R13 */
//    movq        0x20(%r8), %r14 /* FIBER_REG_R14 */
//    movq        0x28(%r8), %r15 /* FIBER_REG_R15 */
//    // Load first parameter, this is only used for the first time a fiber gains control
//    movq        0x30(%r8), %rdi /* FIBER_REG_RDI */
//    // Load stack pointer
//    movq        0x38(%r8), %rsp /* FIBER_REG_RSP */
//    // Load instruction pointer, and jump
//    movq        0x40(%r8), %rcx /* FIBER_REG_RIP */
//    jmp         *%rcx
//)");
//
//#elif defined(FIBER_WIN)
//
//    struct FiberContext {
//      // handle to hold fiber
//      LPVOID  raw_fiber_handle = nullptr;
//    };
//
//#endif
//
//#if defined(FIBER_ASM_IMPLEMENTATION)
//
//#if __has_attribute (optnone)
//#define FORCENOTINLINE     __attribute__((optnone))
//#elif __has_attribute (noinline)
//#define FORCENOTINLINE     __attribute__((noinline))
//#else
//#define FORCENOTINLINE
//#endif
//
//  extern "C" 
//  {
//    extern void FORCENOTINLINE _switch_fiber_internal(FiberContext* from, const FiberContext* to);
//  }
//#endif
//  template<u64 StackSize>
//    struct Fiber;
//
//  template<u64 StackSize>
//    using FiberHandle = Fiber<StackSize>*;
//  //! Abstruction for fiber struct.
//  template<u64 StackSize>
//    struct Fiber {
//      using FiberHandle = FiberHandle<StackSize>;
//      /**< fiber context, this is platform dependent. */
//      FiberContext     context;
//      FiberHandle prev;
//      b8 waiting = false;
//
//      /**< Pointer to stack. */
//      void*                   stack_ptr = nullptr;
//
//      /**< Stack size. */
//      //! Create a new fiber.
//      //!
//      //! Allocate stack memory for the fiber. If there is no valid function pointer provided, it will fail.
//      Fiber(void (*fiber_func)(void*), void* arg, void* stack) {
//        context = {};
//
//        stack_ptr = stack;
//#if defined(FIBER_ASM_IMPLEMENTATION)
//
//        // Make sure the stack meets the alignment requirement
//        uintptr_t aligned_stack_ptr = (uintptr_t)stack_ptr;
//        aligned_stack_ptr += FIBER_STACK_ALIGNMENT - 1;
//        aligned_stack_ptr &= ~(FIBER_STACK_ALIGNMENT - 1);
//
//        _create_fiber_internal((void*)aligned_stack_ptr, StackSize, fiber_func, arg, &context);
//#elif defined(FIBER_WIN)
//        context.raw_fiber_handle = ::CreateFiber(StackSize, fiber_func, arg);
//#endif
//      }
//
//      Fiber() {
//#if defined(FIBER_ASM_IMPLEMENTATION)
//        context = {};
//        stack_ptr = nullptr;
//#elif defined(FIBER_WIN)
//        context.raw_fiber_handle = ConvertThreadToFiber(nullptr);
//#endif
//      }
//
//      b8 is_waiting() {
//        return waiting;
//      }
//
//      b8 operator==(FiberHandle other) {
//        if(other->get_stack_base() == stack_ptr) {
//          return true;
//        }
//        return false;
//      }
//
//      void swap(FiberHandle from_fiber) {
//        prev = from_fiber;
//        if(stack_ptr == nullptr)
//          return;
//#if defined(FIBER_ASM_IMPLEMENTATION)
//        _switch_fiber_internal(&prev->context, &context);
//#elif defined(FIBER_WIN)
//        ::SwitchToFiber(context.raw_fiber_handle);
//#endif
//      }
//
//      void yield() {
//#if defined(FIBER_ASM_IMPLEMENTATION)
//        _switch_fiber_internal(&context, &prev->context);
//#elif defined(FIBER_WIN)
//        ::SwitchToFiber(prev->context.raw_fiber_handle);
//#endif
//      }
//
//      void wait() {
//        waiting = true;
//        yield();
//        waiting = false;
//      }
//
//      void* get_stack_base() {
//        return stack_ptr;
//      }
//
//      void clear() {
//        prev = nullptr;
//        context = {};
//        stack_ptr = nullptr;
//        waiting = false;
//      }
//
//      void end_fiber() {
//#if defined(FIBER_ASM_IMPLEMENTATION)
//        stack_ptr = nullptr;
//#elif defined(FIBER_WIN)
//        ConvertFiberToThread();
//        context.raw_fiber_handle = nullptr;
//#endif
//      }
//
//      void delete_fiber() {
//#if defined(FIBER_ASM_IMPLEMENTATION)
//#elif defined(FIBER_WIN)
//        ::DeleteFiber(context.raw_fiber_handle);
//#endif
//      }
//    };
//}
