// =====================================================================================
//
//       Filename:  error.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-25 9:16:29 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_log.hpp"

#ifndef NRXDEBUG 
#define RX_CHECK(c, statement) STMNT(if(!(c))\
                                       { LOG_ERROR(Error(__LINE__, __FILE__, statement),Debug);\
                                         return false;                                                      \
                                       })

#define RX_ASSERT(c, statement) STMNT(if(!(c))\
                                       { LOG_ERROR(Error(__LINE__, __FILE__, statement),Fatal);\
                                         DUMP_LOGS(); \
                                         PRINT_LINE("[Fatal] fatal error encountered... crashing program...");\
                                         L_ASSERT_BREAK();\
                                       })

#define RX_RETURN(c, statement, on_fail_return) STMNT(if(!(c))\
                                                      { LOG_ERROR(Error(__LINE__, __FILE__, statement),Debug);\
                                                        return (on_fail_return);                              \
                                                      })

#define RX_CHECKF(c, statement, ...) STMNT(if(!(c))\
                                       { LOG_ERROR(Error(__LINE__, __FILE__, statement, __VA_ARGS__),Debug);\
                                         return false;                                                      \
                                       })

#define RX_ASSERTF(c, statement, ...) STMNT(if(!(c))\
                                       { LOG_ERROR(Error(__LINE__, __FILE__, statement, __VA_ARGS__),Fatal);\
                                         DUMP_LOGS(); \
                                         PRINT_LINE("[Fatal] fatal error encountered... crashing program...");\
                                         L_ASSERT_BREAK();\
                                       })

#define RX_RETURNF(c, on_fail_return, statement, ...) STMNT(if(!(c))\
                                                      { LOG_ERROR(Error(__LINE__, __FILE__, statement, __VA_ARGS__),Debug);\
                                                        return (on_fail_return);                              \
                                                      })
#define RX_DEBUG_CREATE(obj) obj

#else
#define RX_CHECK(c, statement) (c)
#define RX_ASSERT(c, statement) (c)
#define RX_RETURN(c, statement, return_value) (c)
#define RX_DEBUG_CREATE(obj)

#define RX_CHECKF(c, statement) (c)
#define RX_ASSERTF(c, statement) (c)
#define RX_RETURNF(c, statement, return_value) (c)
#endif

namespace roxi {
  
  class Error {
  public:
    Error() noexcept;
    Error( u32 line, const char* file, String message) noexcept;
    Error( u32 line, const char* file, const char* fmt_string, ...) noexcept;
    virtual const char* what();
    virtual const char* get_type() const noexcept;
    u32 get_line() noexcept;
    String get_file() noexcept;
    String get_origin_string();
    String get_message();
    SizedStackArena<KB(4)>& get_arena() {
      return arena;
    }
  private:
    u32 line;
    String file;
    String message;
  protected:
    SizedStackArena<KB(4)> arena;
  };

}		// -----  end of namespace roxi  ----- 
