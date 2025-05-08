// =====================================================================================
//
//       Filename:  rx_lexer.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-14 7:24:13 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"
#include "rx_allocator.hpp"
#include "rx_log.hpp"

#define TOKEN_TYPES(X) X(Plus)\
  X(Minus)\
  X(Slash)\
  X(OpenParen)\
  X(CloseParen)\
  X(Colon)\
  X(SemiColon)\
  X(Asterisk)\
  X(OpenBracket)\
  X(CloseBracket)\
  X(OpenSquareBracket)\
  X(CloseSquareBracket)\
  X(OpenAngleBracket)\
  X(CloseAngleBracket)\
  X(Equals)\
  X(Bang)\
  X(EqualsEquals)\
  X(CloseAngleEquals)\
  X(OpenAngleEquals)\
  X(BangEquals)\
  X(Hash)\
  X(Comma)\
  X(System)\
  X(Task)\
  X(Component)\
  X(Archetype)\
  X(Struct)\
  X(Int64)\
  X(Int32)\
  X(Int16)\
  X(Int8)\
  X(UInt64)\
  X(UInt32)\
  X(UInt16)\
  X(UInt8)\
  X(Float64)\
  X(Float32)\
  X(Bool)\
  X(Const)\
  X(ReadOnly)\
  X(WriteOnly)\
  X(ReadWrite)\
  X(Enum)\
  X(And)\
  X(Or)\
  X(If)\
  X(Else)\
  X(For)\
  X(True)\
  X(False)\
  X(While)\
  X(Quote)\
  X(Identifier)\
  X(Number)\
  X(PlusEquals)\
  X(MinusEquals)\
  X(AsteriskEquals)\
  X(SlashEquals)\
  X(PlusPlus)\
  X(MinusMinus)\
  X(EOS)\
  X(Unknown)\
  X(Max)

namespace roxi {
  namespace rlsl {

    namespace lex {
      static b8 IsAlpha(const char c) {
        return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
      }
 
      static b8 IsEndOfLine(const char c) {
        return ((c == lofi::NewLine) || (c == lofi::LineReturn));
      }
      
      static b8 IsWhitespace(const char c) {
        return ((c == ' ') || (c <= '\t') || (c == '\v') || (c == '\f') || IsEndOfLine(c));
      }
      
      static b8 IsNumber(const char c) {
        return ((c >= '0') && (c <= '9'));
      }


      struct Token {
        enum class Type {
#define ENUM(X) X,
    TOKEN_TYPES(ENUM)
#undef ENUM
        };

        b8 operator!=(const Token rhs) {
          return rhs.type != type;
        }

        b8 operator!=(const Type rhs) {
          return rhs != type;
        }


        b8 operator==(const Token rhs) {
          return rhs.type == type;
        }

        b8 operator==(const Type rhs) {
          return rhs == type;
        }

        Type type = Type::Max;
        u32 line;
        String text;
      };

      const char* get_token_type_name(const Token::Type type) {
#define IF_STRING(X) if(type == Token::Type::X) return STRINGIFY(X);
        TOKEN_TYPES(IF_STRING)
#undef IF_STRING
          return "";
      }
    
    }		// -----  end of namespace lex  ----- 

    class Lexer {
    private:
      struct Entry {
        u32 offset : 30;
        u32 type : 2;
      };
      const char* _position = nullptr;
      u32 _line = 0;
      u32 _column = 0;

      b8 _error = false;
      u32 _error_line = MAX_u32;
      Array<Entry> _entries;
      Arena _arena;

      void skip_whitespace() {
        for(;;) {
          // check if whitespace
          if(lex::IsWhitespace(_position[0])) {
            if(lex::IsEndOfLine(_position[0])) {
              _line++;
            }
            _position++;
          }
          // check if single line comment ie: double slash
          else if((_position[0] == '/') && (_position[1] == '/')) {
            _position += 2;
            while(_position[0] && !lex::IsEndOfLine(_position[0])) {
              _position++;
            }
          }
          // check if multi line comment ie: slash asterisk -> asterisk slash
          else if((_position[0] == '/') && (_position[1] == '*')) {
            _position += 2;

          // Advance until the string is closed. Remember to check if line is changed.
            while(!((_position[0] == '*') && (_position[1] == '/'))) {
              // Handle change of line
              if(lex::IsEndOfLine( _position[0]))
                  _line++;
              // Advance to next character
              _position++;
            }
  
            if(_position[0] == '*' ) {
              _position += 2;
            }
          }
          else {
            break;
          }
        }
      }

      void parse_number() {
        char c = _position[0];

        // Parse the following literals:
        // 58, -58, 0.003, 4e2, 123.456e-67, 0.1E4f
        // 1. Sign detection
        i32 sign = 1;
        if ( c == '-' ) {
          sign = -1;
          _position++;
        }

        // 2. Heading zeros (00.003)
        if ( *_position == '0' ) {
          _position++;

          while(*_position == '0')
            _position++;
        }
        // 3. Decimal part (until the point)
        //i32 decimal_part = 0;
        if(*_position > '0' && *_position <= '9') {
          //decimal_part = (*_position - '0');
          _position++;

          while(*_position != '.' && lex::IsNumber( *_position ) ) {
            //decimal_part = (decimal_part * 10) + (*_position - '0');
            _position++;
          }
        }
        // 4. Fractional part
        //i32 fractional_part = 0;
        //i32 fractional_divisor = 1;

        if(*_position == '.') {
          _position++;
          while(lex::IsNumber( *_position )) {
            //fractional_part = (fractional_part * 10) + (*_position - '0');
            //fractional_divisor *= 10;
            _position++;
          }
        }

        // 5. Exponent (if present)
        if ( *_position == 'e' || *_position == 'E' ) {
            ++_position;
        }

        //f64 parsed_number = (f64)sign * (decimal_part + ((f64)fractional_part / fractional_divisor));
        //Entry& entry = *(_entries.push(1));
        //entry.offset = _arena.get_size();
        //MEM_COPY(_arena.push(sizeof(f64)), &parsed_number, sizeof(f64));
      }

     public:
      b8 init(const char* text, const u32 max_entries = 256, const u32 data_size = 1024) {
        _arena.move_ptr(ALLOCATE(data_size));
        _entries.move_ptr(ALLOCATE(sizeof(Entry) * max_entries));
        _position = text;
        return true;
      }

      b8 terminate() {
        FREE((void*)_arena.get_buffer());
        FREE(_entries.get_buffer());
        return true;
      }

      lex::Token next_token() {
        lex::Token result;
        result.text.str = (u8*)_position;
        result.text.size = 1;
        result.line = _line;

        const char c = _position[0];
        _position++;
        switch(c) {
          case lofi::NewLine:
          {
            _line++;
          }break;
          case '\0':
          {
            result.type = lex::Token::Type::EOS;
          }break;

          case '/':
          {
            if(_position[0] == '/') {
              while(!lex::IsEndOfLine(_position[0])) {
                _position++;
              }
              _position++;
            }else if(_position[0] == '=') {
              _position++;
              result.type = lex::Token::Type::SlashEquals;
              break;
            }
            result.type = lex::Token::Type::Slash;
          }break;
          case '-':
          {
            if(_position[0] == '=') {
              result.type = lex::Token::Type::MinusEquals;
              _position++;
              break;
            } else if(_position[0] == '-') {
              result.type = lex::Token::Type::MinusMinus;
              _position++;
              break;
            }
            result.type = lex::Token::Type::Minus;
          }break;
          case '+':
          {
            if(_position[0] == '=') {
              result.type = lex::Token::Type::PlusEquals;
              _position++;
              break;
            } else if(_position[0] == '+') {
              result.type = lex::Token::Type::PlusPlus;
              _position++;
              break;
            }
            result.type = lex::Token::Type::Plus;
          }break;
          case '(':
          {
            result.type = lex::Token::Type::OpenParen;
          }break;
          case ')':
          {
            result.type = lex::Token::Type::CloseParen;
          }break;
          case '{':
          {
            result.type = lex::Token::Type::OpenBracket;
          }break;
          case '}':
          {
            result.type = lex::Token::Type::CloseBracket;
          }break;
          case ';':
          {
            result.type = lex::Token::Type::SemiColon;
          }break;
          case ':':
          {
            result.type = lex::Token::Type::Colon;
          }break;
          case '*':
          {
            if(_position[0] == '=') {
              result.type = lex::Token::Type::AsteriskEquals;
              _position++;
              break;
            }
            result.type = lex::Token::Type::Asterisk;
          }break;
          case '[':
          {
            result.type = lex::Token::Type::OpenSquareBracket;
          } break;
          case ']':
          {
            result.type = lex::Token::Type::CloseSquareBracket;
          } break;
          case '<':
          {
            if(_position[0] == '=') {
              _position++;
              result.type = lex::Token::Type::OpenAngleEquals;
              break;
            }
            result.type = lex::Token::Type::OpenAngleBracket;
          }break;
          case '>':
          {
            if(_position[0] == '=') {
              _position++;
              result.type = lex::Token::Type::CloseAngleEquals;
              break;
            }
            result.type = lex::Token::Type::CloseAngleBracket;
          }break;
          case '=':
          {
            if(_position[0] == '=') {
              _position++;
              result.type = lex::Token::Type::EqualsEquals;
              break;
            }
            result.type = lex::Token::Type::Equals;
          } break;
          case '!':
          {
            if(_position[0] == '=') {
              _position++;
              result.type = lex::Token::Type::BangEquals;
              break;
            }
            result.type = lex::Token::Type::Bang;
          } break;
          case '#':
          {
            result.type = lex::Token::Type::Hash;
          }break;
          case ',':
          {
            result.type = lex::Token::Type::Comma;
          }break;
          case '"':
          {
            result.type = lex::Token::Type::Quote;
            while(_position[0] && _position[0] != '"')
            {
              if(_position[0] == '\\' && _position[1])
                {
                    ++_position;
                }
                ++_position;
            }

            result.text.size = PTR2INT((u8*)_position - result.text.str);
            if ( _position[0] == '"' ) {
                ++_position;
            }
          } break;

          default:
          {
            // Identifier/keywords
            if ( lex::IsAlpha( c ) ) {
              result.type = lex::Token::Type::Identifier;

              while ( lex::IsAlpha( _position[0] ) || lex::IsNumber( _position[0] ) || (_position[0] == '_') ) {
                _position++;
              }

              result.text.size = PTR2INT((u8*)_position - result.text.str);
            } // Numbers: handle also negative ones!
            else if ( lex::IsNumber( c ) || c == '-' ) {
              // Backtrack to start properly parsing the number
              _position--;
              parse_number();
              // Update token and calculate correct length.
              result.type = lex::Token::Type::Number;
              result.text.size = PTR2INT((u8*)_position - result.text.str);
            }
            else {
              result.type = lex::Token::Type::Unknown;
            }
          } break;
        }
        if(result.type == lex::Token::Type::Identifier) {
          static const String _keywords[] = 
          { String{ (u8*)"and"      , 3 }
          , String{ (u8*)"or"       , 2 }
          , String{ (u8*)"if"       , 2 }
          , String{ (u8*)"else"     , 4 }
          , String{ (u8*)"false"    , 5 }
          , String{ (u8*)"true"     , 4 }
          , String{ (u8*)"system"   , 6 }
          , String{ (u8*)"task"     , 4 }
          , String{ (u8*)"component", 9 }
          , String{ (u8*)"struct"   , 6 }
          , String{ (u8*)"for"      , 3 }
          , String{ (u8*)"while"    , 5 }
          , String{ (u8*)"i64"      , 3 }
          , String{ (u8*)"i32"      , 3 }
          , String{ (u8*)"i16"      , 3 }
          , String{ (u8*)"i8"       , 2 }
          , String{ (u8*)"u64"      , 3 }
          , String{ (u8*)"u32"      , 3 }
          , String{ (u8*)"u16"      , 3 }
          , String{ (u8*)"u8"       , 2 }
          , String{ (u8*)"f32"      , 3 }
          , String{ (u8*)"f64"      , 3 }
          , String{ (u8*)"bool"     , 4 }
          , String{ (u8*)"readonly" , 4 }
          , String{ (u8*)"writeonly", 5 }
          , String{ (u8*)"readwrite", 9 }
          , String{ (u8*)"enum"     , 4 }
          , String{ (u8*)"const"    , 5 }
          , String{ (u8*)"archetype", 9 }
          };

          static const lex::Token::Type _types[] = 
          { lex::Token::Type::And
          , lex::Token::Type::Or
          , lex::Token::Type::If
          , lex::Token::Type::Else
          , lex::Token::Type::False
          , lex::Token::Type::True
          , lex::Token::Type::System
          , lex::Token::Type::Task
          , lex::Token::Type::Component
          , lex::Token::Type::Struct
          , lex::Token::Type::For
          , lex::Token::Type::While
          , lex::Token::Type::Int64
          , lex::Token::Type::Int32
          , lex::Token::Type::Int16
          , lex::Token::Type::Int8
          , lex::Token::Type::UInt64
          , lex::Token::Type::UInt32
          , lex::Token::Type::UInt16
          , lex::Token::Type::UInt8
          , lex::Token::Type::Float32
          , lex::Token::Type::Float64
          , lex::Token::Type::Bool
          , lex::Token::Type::ReadOnly
          , lex::Token::Type::WriteOnly
          , lex::Token::Type::ReadWrite
          , lex::Token::Type::Enum
          , lex::Token::Type::Const
          , lex::Token::Type::Archetype
          };

          for(u32 i = 0; i < ARRAY_SIZE(_keywords); i++) {
            if(lofi::str_compare(_keywords[i], result.text)) {
              result.type = _types[i];
              break;
            }
          }
        }
        return result;
      }

    };

  }		// -----  end of namespace rlsl  ----- 
}		// -----  end of namespace roxi  ----- 
