// =====================================================================================
//
//       Filename:  error.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-25 9:22:51 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "error.h"
#include "rx_vocab.h"


namespace roxi {

  Error::Error() noexcept {}
    
  Error::Error( u32 line, const char* file, const char* fmt_string, ... ) noexcept
    : line(line)
    , file
    {file}
    {
      va_list args;
      va_start(args, fmt_string);
      message = lofi::str_pushfv(&arena, fmt_string, args);
      va_end(args);
    }
    
  Error::Error( u32 line, const char* file, String message ) noexcept
    : line(line)
    , file
      {file}
    , message(message) {}

  const char* Error::what()
  {
    StringList list;
    str_list_push
      (&arena
      , &list
      , get_origin_string()
      );
    str_list_push
      (&arena
      , &list
      , get_message()
      );
    String result = str_join(&arena, &list, nullptr);
    return (char*)result.str;
  }

  const char* Error::get_type() const noexcept
  {
    return "roxi::Error";
  }

  u32 Error::get_line() noexcept
  {
    return line;
  }

  String Error::get_file() noexcept
  {
    return file;
  }

  String Error::get_origin_string()
  {
    String temp = lofi::str_cstring("\t[File] %s\n\t[Line] %d\n\t");
    String result = str_pushf(&arena, (char*)temp.str, file.str, line);
    return result;
  }

  String Error::get_message() 
  {
    String temp = lofi::str_cstring("::: [Message] :::\n\t%s");
    return str_pushf(&arena, (char*)temp.str, message.str);
  }

}		// -----  end of namespace roxi  ----- 
