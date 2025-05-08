// =====================================================================================
//
//       Filename:  rx_parser.hpp
//
//    Description:  
//
//    Current types in rlsl
//
//    
//    vec3
//
//        Version:  1.0
//        Created:  2024-11-25 12:21:25 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_lexer.hpp"

#define PARSER_PRIMARY_EXPR_TYPES(X) X(Number) X(Indentifier) X(String)
#define PARSER_RELATIONAL_EXPR_TYPES(X) X(Less) X(Greater) X(LessEquals) X(GreaterEquals) X(Equals) X(NotEquals)
#define PARSER_LOGICAL_EXPR_TYPES(X) X(And) X(Or)
#define PARSER_ARITHMETIC_EXPR_TYPES(X) X(Addition) X(Subtraction) X(Multiplication) X(Division) X(Modulo)

#define BINDING_POWERS(X) X(Default) X(Comma) X(Assignment) X(Logical) X(Relational) X(Additive) X(Multiplicative) X(Unary) X(Call) X(Member) X(Primary) X(Unknown)

namespace roxi {
  namespace rlsl {
    
      namespace parser {

        enum class PrimaryExprType {
#define ENUM(X) X,
          PARSER_PRIMARY_EXPR_TYPES(ENUM)
#undef ENUM
        };

        enum class RelationalExprType {
#define ENUM(X) X,
          PARSER_RELATIONAL_EXPR_TYPES(ENUM)
#undef ENUM
        };

        enum class LogicalExprTypes {
#define ENUM(X) X,
          PARSER_LOGICAL_EXPR_TYPES(ENUM)
#undef ENUM
        };

        enum class ArithmeticExprTypes {
#define ENUM(X) X,
          PARSER_ARITHMETIC_EXPR_TYPES(ENUM)
#undef ENUM
        };

#define PARSER_TYPE_KINDS(X) X(Int64) X(Int32) X(Int16) X(Int8) X(UInt64) X(UInt32) X(UInt16) X(UInt8) X(Float64) X(Float32) X(Bool)

        struct Type {
          enum class Kind {
#define ENUM(X) X,
            PARSER_TYPE_KINDS(ENUM)
#undef ENUM
          };
          Kind kind;
          u32 array_size;
        };

#define OP_TYPES(X) X(Plus) X(Minus) X(Multiply) X(Divide) X(Equals) X(Not) X(PlusEquals) X(MinusEquals) X(PlusPlus) X(MinusMinus) X(Max)

        enum class OpType {
#define ENUM(X) X,
          OP_TYPES(ENUM)
#undef ENUM
        };

        static const char* get_parser_type_kind_string(Type::Kind kind) {
          switch(kind) {
#define STR_FIND(X) case Type::Kind::X: return STRINGIFY(parser::Type::Kind::X);
            PARSER_TYPE_KINDS(STR_FIND);
#undef STR_FIND
            default:
            return nullptr;
          }
        }

        static const char* get_parser_op_type_string(OpType op_type) {
          switch(op_type) {
#define STR_FIND(X) case OpType::X: return STRINGIFY(parser::OpType::X);
            OP_TYPES(STR_FIND);
#undef STR_FIND
            default:
            return nullptr;
          }
        }

        static constexpr u32 DefaultParserArraySize = 1024;
      }		// -----  end of namespace parser  ----- 

    struct StringExpression {
      String value;
      u32 line;
    };

    struct BoolExpression {
      b8 value;
      u32 line;
    };

    struct NumberExpression {
      using value_t = Variant<i64, f64>;
      value_t value;
      u32 line;
    };

    struct SymbolExpression {
      String name;
      u32 line;
    };


    struct BinaryExpression;
    struct PrefixExpression;
    struct AssignmentExpression;

    using expr_t = Variant<PrefixExpression, AssignmentExpression, StringExpression, NumberExpression, SymbolExpression, BoolExpression, BinaryExpression>;

    struct PrefixExpression {
      parser::OpType op;
      expr_t* right;
      u32 line;
    };

    struct BinaryExpression {
      expr_t* left;
      parser::OpType op;
      expr_t* right;
      u32 line;
    };

    struct AssignmentExpression {
      expr_t* left;
      parser::OpType op;
      expr_t* right;
      u32 line;
    };


    struct BlockStatement;
    struct VarDeclarationStatement;
    struct StructDeclarationStatement;
    struct TaskDeclarationStatement;
    struct ArchetypeDeclarationStatement;
    struct SystemDeclarationStatement;
    struct ComponentDeclarationStatement;
    struct IfStatement;
    struct ForStatement;
    struct WhileStatement;
    struct EnumStatement;

    struct ExpressionStatement {
      expr_t* value;
      u32 line;
    };

    using stmt_t = Variant<ExpressionStatement, BlockStatement, VarDeclarationStatement, StructDeclarationStatement, TaskDeclarationStatement, ArchetypeDeclarationStatement, ComponentDeclarationStatement, SystemDeclarationStatement, IfStatement, ForStatement, WhileStatement, EnumStatement>;

    struct BlockStatement {
      stmt_t* body;
      u32 line;
    };

    struct VarDeclarationStatement {
      String name;
      expr_t* value;
      u32 line;

      parser::Type type;
      b8 is_const;
    };

    static constexpr u16 MaxRLSLStructMembers = 32;
    static constexpr u16 MaxRLSLComponentsInArchetype = 128;
    static constexpr u16 MaxRLSLInputComponentsPerTask = 32;
    static constexpr u16 MaxRLSLOutputComponentsPerTask = 32;
    static constexpr u16 MaxRLSLTasksPerSystem = 32;

    struct StructProperty {
      String type_name;
      String variable_name;
      u32 size = 1;
    };

    struct SystemMember {
      String task_name;
    };

    struct StructDeclarationStatement {
      String name;
      Array<StructProperty> members;
      u32 line;
    };

    struct ComponentDeclarationStatement {
      String name;
      Array<StructProperty> members;
      u32 line;
    };

    struct ComponentParam {
      String component_name;
    };

    struct ArchetypeDeclarationStatement {
      String name;
      Array<ComponentParam> components;
      u32 line;
    };

    struct TaskDeclarationStatement {
      String name;
      Array<ComponentParam> input_components;
      Array<ComponentParam> output_components;
      stmt_t* body;
      u32 line;
    };

    struct SystemDeclarationStatement {
      String name;
      Array<SystemMember> tasks;
      u32 line;
    };

    struct IfStatement {
      expr_t* condition;
      stmt_t* body;

    };

    struct ForStatement {

    };


    struct WhileStatement {

    };


    struct EnumStatement {

    };


    class Interpreter;
    class Compiler;

#define EXPECT(token, expected_type, message, ...)\
    if(token.type != lex::Token::Type::expected_type)\
    { post_error(token, message, __VA_ARGS__); return Node{}; }

    class Parser {
      private:
        friend class Interpreter;
        friend class Compiler;
        static parser::OpType get_op_type(lex::Token::Type type) {
          switch(type) 
          { 
            case lex::Token::Type::Plus:
              return parser::OpType::Plus;
            case lex::Token::Type::Minus:
              return parser::OpType::Minus;
            case lex::Token::Type::Asterisk:
              return parser::OpType::Multiply;
            case lex::Token::Type::Slash:
              return parser::OpType::Divide;
            case lex::Token::Type::Equals:
              return parser::OpType::Equals;
            case lex::Token::Type::Bang:
              return parser::OpType::Not;
            case lex::Token::Type::PlusEquals:
              return parser::OpType::PlusEquals;
            case lex::Token::Type::MinusEquals:
              return parser::OpType::MinusEquals;
            case lex::Token::Type::PlusPlus:
              return parser::OpType::PlusPlus;
            case lex::Token::Type::MinusMinus:
              return parser::OpType::MinusMinus;
            default:
              return parser::OpType::Max;
          };
        }

        enum class BindingPower {
#define ENUM(X) X,
          BINDING_POWERS(ENUM)
#undef ENUM
        };

        // enum class Type {
        //     String,
        //     Symbol,
        //     Number,
        //     Boolean,
        //     Prefix,
        //     Binary,
        //     Expr,
        //     Stmt,
        //     VarDecl,
        //   };


        using Node = Variant<expr_t, stmt_t>;

        using arena_t = SizedStackArena<KB(16)>;
        arena_t _arena;
        Lexer _lexer;
        StackArray<Error> _errors;
        StackArray<Node> _ast_nodes;
        const char* _current_src_file = nullptr;

        // FIND BINDING POWER TABLES
        BindingPower get_led_binding_power(const lex::Token::Type type) {
          static const BindingPower _s_b_pwrs[] = //
          { BindingPower::Additive                // +
            , BindingPower::Additive                // -
            , BindingPower::Multiplicative          // /
            , BindingPower::Call                    // (
            , BindingPower::Unknown                 // )
            , BindingPower::Unknown                 // :
            , BindingPower::Unknown                 // ;
            , BindingPower::Multiplicative          // *
            , BindingPower::Member                  // {
            , BindingPower::Unknown                 // }
            , BindingPower::Member                  // [
            , BindingPower::Unknown                 // ]
            , BindingPower::Relational              // <
            , BindingPower::Relational              // >
            , BindingPower::Assignment              // =
            , BindingPower::Unknown                 // !
            , BindingPower::Relational              // ==
            , BindingPower::Relational              // >=
            , BindingPower::Relational              // <=
            , BindingPower::Relational              // !=
            , BindingPower::Unknown                 // #
            , BindingPower::Comma                   // ,
            , BindingPower::Unknown                 // system
            , BindingPower::Unknown                 // task
            , BindingPower::Unknown                 // component
            , BindingPower::Unknown                 // archetype
            , BindingPower::Unknown                 // struct
            , BindingPower::Unknown                 // i64
            , BindingPower::Unknown                 // u64
            , BindingPower::Unknown                 // i32
            , BindingPower::Unknown                 // u32
            , BindingPower::Unknown                 // i16
            , BindingPower::Unknown                 // u16
            , BindingPower::Unknown                 // i8
            , BindingPower::Unknown                 // u8
            , BindingPower::Unknown                 // f32
            , BindingPower::Unknown                 // f64
            , BindingPower::Unknown                 // bool
            , BindingPower::Unknown                 // readonly
            , BindingPower::Unknown                 // writeonly
            , BindingPower::Unknown                 // readwrite
            , BindingPower::Unknown                 // enum
            , BindingPower::Logical                 // and
            , BindingPower::Logical                 // or
            , BindingPower::Unknown                 // if
            , BindingPower::Unknown                 // else
            , BindingPower::Unknown                 // for
            , BindingPower::Unknown                 // true
            , BindingPower::Unknown                 // false
            , BindingPower::Unknown                 // while
            , BindingPower::Unknown                 // "
            , BindingPower::Unknown                 // Identifier
            , BindingPower::Unknown                 // Number
            , BindingPower::Assignment              // +=
            , BindingPower::Assignment              // -=
            , BindingPower::Assignment              // *=
            , BindingPower::Assignment              // /=
            , BindingPower::Assignment              // ++
            , BindingPower::Assignment              // --
          };                                      //
          return _s_b_pwrs[(u8)type];             //
        }

        BindingPower get_nud_binding_power(const lex::Token::Type type) {
          static const BindingPower _s_b_pwrs[] = //
          { BindingPower::Unknown                 // +
            , BindingPower::Unary                   // -
              , BindingPower::Unknown                 // /
              , BindingPower::Unknown                 // (
              , BindingPower::Unknown                 // )
              , BindingPower::Unknown                 // :
              , BindingPower::Unknown                 // ;
            , BindingPower::Unknown                 // *
              , BindingPower::Primary                 // {
              , BindingPower::Unknown                 // }
          , BindingPower::Unknown                 // [
            , BindingPower::Unknown                 // ]
              , BindingPower::Unknown                 // <
              , BindingPower::Unknown                 // >
              , BindingPower::Unknown                 // =
              , BindingPower::Unary                   // !
              , BindingPower::Unknown                 // ==
              , BindingPower::Unknown                 // >=
              , BindingPower::Unknown                 // <=
              , BindingPower::Unknown                 // !=
              , BindingPower::Unknown                 // #
              , BindingPower::Unknown                 // ,
            , BindingPower::Unknown                 // system
              , BindingPower::Unknown                 // task
              , BindingPower::Unknown                 // component
              , BindingPower::Unknown                 // archetype
              , BindingPower::Unknown                 // struct
              , BindingPower::Unknown                 // i64
              , BindingPower::Unknown                 // u64
              , BindingPower::Unknown                 // i32
              , BindingPower::Unknown                 // u32
              , BindingPower::Unknown                 // i16
              , BindingPower::Unknown                 // u16
              , BindingPower::Unknown                 // i8
              , BindingPower::Unknown                 // u8
              , BindingPower::Unknown                 // f32
              , BindingPower::Unknown                 // f64
              , BindingPower::Unknown                 // bool
              , BindingPower::Unknown                 // readonly
              , BindingPower::Unknown                 // writeonly
              , BindingPower::Unknown                 // readwrite
              , BindingPower::Unknown                 // enum
              , BindingPower::Unknown                 // and
              , BindingPower::Unknown                 // or
              , BindingPower::Unknown                 // if
              , BindingPower::Unknown                 // else
              , BindingPower::Unknown                 // for
              , BindingPower::Unknown                 // true
              , BindingPower::Unknown                 // false
              , BindingPower::Unknown                 // while
              , BindingPower::Primary                 // "
              , BindingPower::Primary                 // Identifier
              , BindingPower::Primary                 // Number
              , BindingPower::Unknown                 // +=
              , BindingPower::Unknown                 // -=
              , BindingPower::Unknown                 // *=
              , BindingPower::Unknown                 // /=
              , BindingPower::Unknown                 // ++
              , BindingPower::Unknown                 // --
          };                                      // 
          return _s_b_pwrs[(u8)type];             // 
        }

        void post_error(const lex::Token& token, const char* message, ...) {
          va_list args;
          va_start(args, message);

          String string = lofi::str_pushfv(&_arena, message, args);
          *(_errors.push(1)) = Error(token.line, _current_src_file, string);
          va_end(args);
        }

        Node string(const lex::Token& token) {
          return Node(expr_t(StringExpression{token.text, token.line}));
        }                                         

        Node identifier(const lex::Token& token) {
          return Node(expr_t(SymbolExpression{token.text, token.line}));
        }

        Node number(const lex::Token& token) {
          i64 integer;
          f64 floating_point;

          if(sscanf((char*)token.text.str, "%llu", &integer)) { // NOLINT
            return Node(expr_t(NumberExpression{(i64)integer}));
          } else if (sscanf((char*)token.text.str, "%lf", &floating_point)) {    // NOLINT
            return Node(expr_t(NumberExpression{(f64)floating_point, token.line}));
          } else {
            post_error(token, "failed to parse number literal");
          }
          return Node{};
        }

        Node prefix(const lex::Token& token) {
          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = expr(_lexer.next_token(), BindingPower::Default);

          return Node(expr_t(PrefixExpression{get_op_type(token.type), parse_expr_node(_ast_nodes[idx]), token.line}));
        }

        template<typename T>
          expr_t* parse_expr_node(T& expression) {
            return (expr_t*)&expression;
          }

        template<typename T>
          stmt_t* parse_stmt_node(T& statement) {
            return (stmt_t*)&statement;
          }


        Node block_stmt(const lex::Token& token) {
          EXPECT(token, OpenBracket, "expected block statement must begin with an open bracket ie: {");

          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = stmt(_lexer.next_token());

          lex::Token close_bracket_token = _lexer.next_token();
          EXPECT(close_bracket_token, CloseBracket, "expected closing bracket ie: } after block statement");

          return Node{stmt_t(BlockStatement{parse_stmt_node(_ast_nodes[idx]), token.line})};
        }

        static const b8 is_type(lex::Token::Type token_type) {
          return token_type == lex::Token::Type::Int64
            || token_type == lex::Token::Type::Int32
            || token_type == lex::Token::Type::Int16
            || token_type == lex::Token::Type::Int8
            || token_type == lex::Token::Type::UInt64
            || token_type == lex::Token::Type::UInt32
            || token_type == lex::Token::Type::UInt16
            || token_type == lex::Token::Type::UInt8
            || token_type == lex::Token::Type::Float64
            || token_type == lex::Token::Type::Float32
            || token_type == lex::Token::Type::Bool ? true : false;
        }

        static const parser::Type::Kind convert_lexer_type_to_parser_type_kind(const lex::Token::Type lex_type) {
          static const parser::Type::Kind _s_kinds[] = 
          { parser::Type::Kind::Int64
            , parser::Type::Kind::Int32
              , parser::Type::Kind::Int16
              , parser::Type::Kind::Int8
              , parser::Type::Kind::UInt64
              , parser::Type::Kind::UInt32
              , parser::Type::Kind::UInt16
              , parser::Type::Kind::UInt8
              , parser::Type::Kind::Float64
              , parser::Type::Kind::Float32
              , parser::Type::Kind::Bool
          };

          return _s_kinds[(u8)lex_type - (u8)parser::Type::Kind::Int64];
        }

        Node variable(const lex::Token& token) {

          lex::Token current_token = token;
          b8 is_const = false;
          if(current_token.type == lex::Token::Type::Const) {
            current_token = _lexer.next_token();
            is_const = true;

          }
          if(!is_type(current_token.type)) {
            post_error(current_token, "const variable declaration must be followed by a type name");
            return Node{};
          }

          const parser::Type::Kind kind = convert_lexer_type_to_parser_type_kind(current_token.type);

          lex::Token identifier_token = _lexer.next_token();
          EXPECT(identifier_token, Identifier, "expected identifier token after typename");

          String name = identifier_token.text;

          u64 size = 1;
          lex::Token next_token = _lexer.next_token();
          if(next_token.type == lex::Token::Type::OpenSquareBracket) {
            lex::Token array_count_token = _lexer.next_token();
            EXPECT(array_count_token, Number, "size for array type not found, array size must be static", parser::DefaultParserArraySize);
            if((char)array_count_token.text.str[0] == '-') {
              post_error(array_count_token, "array count must be an unsigned integer");
              return Node{};
            }
            if(!sscanf((char*)array_count_token.text.str, "%llu", &size)) {     // NOLINT
              post_error(array_count_token, "array count must be an unsigned integer type");
              return Node{};
            }
            if(size >= MAX_u32) {
              post_error(array_count_token, "array size too large, will be narrowed to lower than %llu", MAX_u32);
            }
          }
          lex::Token close_square_bracket_token = _lexer.next_token();
          EXPECT(close_square_bracket_token, CloseSquareBracket, "array type variable declaration requires a closing square bracket");

          const auto expr_idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = expr(_lexer.next_token(), BindingPower::Assignment);

          return Node(stmt_t(VarDeclarationStatement{name, parse_expr_node(_ast_nodes[expr_idx]), token.line, parser::Type{kind, (u32)size}, is_const}));
        }

        Node assignment(const Node& lhs, const lex::Token& token) {
          parser::OpType op = get_op_type(token.type);
          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = expr(_lexer.next_token(), BindingPower::Assignment);
          return Node(expr_t(AssignmentExpression{parse_expr_node(lhs), op, parse_expr_node(_ast_nodes[idx]), token.line}));
        }

        Node grouping(const lex::Token& token) {
          lex::Token next_token = _lexer.next_token();
          Node expression = expr(next_token, BindingPower::Default);
          next_token = _lexer.next_token();
          EXPECT(next_token, CloseParen, "could not find a close parenthesis at the end of a grouping");
          return expression;
        }

        Node parse_nud(const lex::Token& token) {
          Node result;
          switch(token.type) {
            case lex::Token::Type::True:{
                                          return Node(expr_t(BoolExpression{true}));
                                        }
            case lex::Token::Type::False:{
                                           return Node(expr_t(BoolExpression{false}));
                                         }
            case lex::Token::Type::Number:
                                         return number(token);
            case lex::Token::Type::Quote:
                                         return string(token);
            case lex::Token::Type::Identifier:
                                         return identifier(token);
            case lex::Token::Type::Bang:
                                         return prefix(token);
            case lex::Token::Type::Minus:
                                         return prefix(token);
            case lex::Token::Type::OpenParen:
                                         return grouping(token);
            default:
                                         post_error(token, "could not properly parse nud expression");
          }
          return Node{};
        }

        Node parse_led(const Node& left, const lex::Token& token, const BindingPower power) {

          lex::Token next_token = _lexer.next_token();
          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = expr(next_token, power);
          return Node(expr_t(BinaryExpression{parse_expr_node(left), get_op_type(token.type), parse_expr_node(_ast_nodes[idx])}));
        }

        Node expr(const lex::Token& token, const BindingPower power) {

          BindingPower current_power = get_nud_binding_power(token.type);

          if(current_power == BindingPower::Unknown) {
            // error, expr should begin with a nud!
            post_error(token, "expression does not begin with a valid nud");
            return Node{};
          }

          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = parse_nud(token);

          lex::Token next_token = _lexer.next_token();
          current_power = get_led_binding_power(next_token.type);
          while(current_power > power) {
            if(current_power == BindingPower::Unknown) {
              // error, expr rhs should be a led!
              post_error(token, "expression rhs is not a valid led");
              return Node{};
            }
            const auto idx_right = _ast_nodes.get_size();
            *(_ast_nodes.push(1)) = parse_led(_ast_nodes[idx], next_token, current_power);
            next_token = _lexer.next_token();
            current_power = get_led_binding_power(next_token.type);
          }
          const auto idx_final = _ast_nodes.get_size() - 1;

          return parse_led(_ast_nodes[idx_final], next_token, power);
        }

        Node archetype(const lex::Token& token) {
          EXPECT(token, Identifier, "expected identifier name after archetype declaration");

          String name = token.text;

          SizedStackArray<ComponentParam, MaxRLSLComponentsInArchetype> members;

          lex::Token next_token = _lexer.next_token();
          EXPECT(next_token, OpenParen, "expected open paren ie: ( after archetype identifier name %s", token.text);
          next_token = _lexer.next_token();
          while(next_token.type != lex::Token::Type::CloseParen) {
            EXPECT(next_token, Identifier, "expected identifier for system component declaration");
            members.push(1)->component_name = next_token.text;
            next_token = _lexer.next_token();
          }
          const auto member_count = members.get_size();
          Array<ComponentParam> components{_arena.push(sizeof(ComponentParam) * member_count)};
          components.push(member_count);
          for(u32 i = 0; i < member_count; i++) {
            components[i] = members[i];
          }

          return Node{stmt_t(ArchetypeDeclarationStatement{name, components, token.line})};
        }

        Node component(const lex::Token& token) {

          EXPECT(token, Identifier, "expected identifier name after component declaration");

          String name = token.text;

          SizedStackArray<StructProperty, MaxRLSLStructMembers> members;

          lex::Token next_token = _lexer.next_token();
          EXPECT(next_token, OpenBracket, "expected open bracket ie: { after struct identifier name %s", token.text);

          next_token = _lexer.next_token();
          lex::Token::Type type = next_token.type;
          while
            ( type != lex::Token::Type::EOS
              && type != lex::Token::Type::CloseBracket
            ) {
              StructProperty& member = *(members.push(1));
              if(is_type(type)) {
                // primitive type
                member.type_name = next_token.text;
                lex::Token var_name = _lexer.next_token();
                if(var_name.type != lex::Token::Type::Identifier) {
                  post_error(var_name, "expected identifier after %s type declaration in struct %s", (char*)next_token.text.str, (char*)token.text.str);
                  return Node{};
                }

                member.variable_name = var_name.text;
              } else if(type == lex::Token::Type::Identifier) {
                // structure type
                member.type_name = next_token.text;
                lex::Token var_name = _lexer.next_token();
                if(var_name.type != lex::Token::Type::Identifier) {
                  post_error(var_name, "expected identifier after %s type declaration in struct %s", (char*)next_token.text.str, (char*)token.text.str);
                  return Node{};
                }
                member.variable_name = var_name.text;
              }

              lex::Token end_token = _lexer.next_token();
              if(end_token.type == lex::Token::Type::OpenSquareBracket) {
                // array type
                end_token = _lexer.next_token();
                EXPECT(end_token, Number, "array declaration inside struct %s must have static size", (char*)token.text.str);
                if(end_token.text.str[0] == '-') {
                  post_error(end_token, "array count cannot be negative");
                }
                if(!sscanf((char*)end_token.text.str, "%llu", &member.size)) {     // NOLINT
                  post_error(end_token, "array count must be an unsigned integer type");
                  return Node{};
                }
                end_token = _lexer.next_token();
                EXPECT(end_token, CloseSquareBracket, "expected closing square bracket ie: ] after array declaration in struct %s", (char*)token.text.str);
              }
            }
          const u32 member_count = members.get_size();
          Array<StructProperty> props{_arena.push(sizeof(StructProperty) * member_count)};
          props.push(member_count);
          for(u32 i = 0; i < member_count; i++) {
            props[i] = members[i];
          }
          return Node{stmt_t(ComponentDeclarationStatement{name, props, token.line})};
        }

        Node task(const lex::Token& token) {
          if(token.type != lex::Token::Type::Identifier) {
            post_error(token, "expected identifier token for task name declaration, instead got %s", lex::get_token_type_name(token.type));
            return Node{};
          }
          String name = token.text;

          lex::Token next_token = _lexer.next_token();
          EXPECT(next_token, OpenParen, "expected open paren ie: ( after task %s declaration, instead got %s", (char*)name.str, lex::get_token_type_name(next_token.type));
          next_token = _lexer.next_token();

          SizedStackArray<ComponentParam, MaxRLSLInputComponentsPerTask> inputs;
          SizedStackArray<ComponentParam, MaxRLSLInputComponentsPerTask> outputs;

          lex::Token name_token = _lexer.next_token();
          while(next_token.type != lex::Token::Type::CloseParen) {
            switch(next_token.type) {
              case lex::Token::Type::ReadOnly:{
                                                EXPECT(name_token, Identifier, "expected identifier token, instead got %s", (char*)name_token.text.str);
                                                (inputs.push(1))->component_name = name_token.text;
                                              }break;
              case lex::Token::Type::WriteOnly:{
                                                 EXPECT(name_token, Identifier, "expected identifier token, instead got %s", (char*)name_token.text.str);
                                                 (outputs.push(1))->component_name = name_token.text;
                                               }break;
              case lex::Token::Type::ReadWrite:{
                                                 EXPECT(name_token, Identifier, "expected identifier token, instead got %s", (char*)name_token.text.str);
                                                 (inputs.push(1))->component_name = next_token.text;
                                                 (outputs.push(1))->component_name = next_token.text;
                                               }break;
              default:
                                               post_error(next_token, "expected ReadOnly, WriteOnly, or ReadWrite for component parameter declaration in task %s", (char*)name.str);
                                               return Node{};
            }
            next_token = _lexer.next_token();
            name_token = _lexer.next_token();
          }
          const auto input_count = inputs.get_size();
          const auto output_count = outputs.get_size();
          Array<ComponentParam> input_components{_arena.push(sizeof(ComponentParam) * input_count)};
          input_components.push(input_count);
          Array<ComponentParam> output_components{_arena.push(sizeof(ComponentParam) * output_count)};
          output_components.push(output_count);
          const auto iter_count = MAX(input_count, output_count);
          for(u32 i = 0; i < iter_count; i++) {
            if(i < input_count) {
              input_components[i] = inputs[i];
            }
            if(i < output_count) {
              output_components[i] = outputs[i];
            }
          }
          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = block_stmt(name_token);
          return Node{stmt_t(TaskDeclarationStatement{name, input_components, output_components, parse_stmt_node(_ast_nodes[idx]), token.line})};
        }

        Node system(const lex::Token& token) {
          EXPECT(token, Identifier, "expected identifier token for system name declaration, instead got %s", lex::get_token_type_name(token.type));
          String name = token.text;

          lex::Token next_token = _lexer.next_token();

          EXPECT(next_token, OpenParen, "expected open paren ie: ( after system %s declaration, instead got %s", (char*)name.str, lex::get_token_type_name(next_token.type));

          SizedStackArray<SystemMember, MaxRLSLTasksPerSystem> members;

          while(next_token.type != lex::Token::Type::CloseParen) {
            EXPECT(next_token, Identifier, "expected indentifier for task name, instead got %s", lex::get_token_type_name(next_token.type));
            (members.push(1))->task_name = next_token.text;
            next_token = _lexer.next_token();
            EXPECT(next_token, Comma, "expected comma separator, instead got %s", lex::get_token_type_name(next_token.type));
            next_token = _lexer.next_token();
          }
          next_token = _lexer.next_token();
          EXPECT(next_token, SemiColon, "expected semi-colon at the end of system declaration");
          const auto task_count = members.get_size();
          Array<SystemMember> tasks{_arena.push(sizeof(SystemMember) * task_count)};
          tasks.push(task_count);
          for(u32 i = 0; i < task_count; i++) {
            tasks[i] = members[i];
          }

          return Node{stmt_t(SystemDeclarationStatement{name, tasks, token.line})};
        }

        Node structure(const lex::Token& token) {
          EXPECT(token, Identifier, "expected identifier token for struct name declaration, instead got %s", lex::get_token_type_name(token.type));
          String name = token.text;
          SizedStackArray<StructProperty, MaxRLSLStructMembers> members;

          const lex::Token open_bracket = _lexer.next_token();
          EXPECT(open_bracket, OpenBracket, "expected open bracket ie: { after struct identifier name %s", token.text);

          lex::Token next_token = _lexer.next_token();
          lex::Token::Type type = next_token.type;
          while
            ( type != lex::Token::Type::EOS
              && type != lex::Token::Type::CloseBracket
            ) {
              StructProperty& member = *(members.push(1));
              if(is_type(type)) {
                // primitive type
                member.type_name = next_token.text;
                lex::Token var_name = _lexer.next_token();
                EXPECT(var_name, Identifier, "expected identifier after %s type declaration in struct %s", (char*)next_token.text.str, (char*)token.text.str);
                member.variable_name = var_name.text;
              } else if(type == lex::Token::Type::Identifier) {
                // structure type
                member.type_name = next_token.text;
                lex::Token var_name = _lexer.next_token();
                EXPECT(var_name, Identifier, "expected identifier after %s type declaration in struct %s", (char*)next_token.text.str, (char*)token.text.str);
                member.variable_name = var_name.text;
              }
              lex::Token end_token = _lexer.next_token();
              if(end_token.type == lex::Token::Type::OpenSquareBracket) {
                // array type
                end_token = _lexer.next_token();
                EXPECT(end_token, Number, "array declaration inside struct %s must have static size", (char*)token.text.str);
                if(end_token.text.str[0] == '-') {
                  post_error(end_token, "array count cannot be negative");
                }
                if(!sscanf((char*)end_token.text.str, "%llu", &member.size)) {     // NOLINT
                  post_error(end_token, "array count must be an unsigned integer type");
                  return Node{};
                }
                end_token = _lexer.next_token();
                EXPECT(end_token, CloseSquareBracket, "expected closing square bracket ie: ] after array declaration in struct %s", (char*)token.text.str);
              }
              next_token = _lexer.next_token();
              EXPECT(next_token, SemiColon, "expected semi-colon after struct member declaration");
            }
          next_token = _lexer.next_token();
          EXPECT(next_token, SemiColon, "expected semi-colon after struct declaration");
          const auto member_count = members.get_size();
          Array<StructProperty> props{_arena.push(sizeof(StructProperty) * member_count)};
          props.push(member_count);
          for(u32 i = 0; i < member_count; i++) {
            props[i] = members[i];
          }
          return Node{stmt_t(StructDeclarationStatement{name, props, token.line})};
        }

        Node if_stmt(const lex::Token& token) {
          EXPECT(token, OpenParen, "expected open parenthesis after if keyword");
          lex::Token next_token = _lexer.next_token();


          return Node{};
        }

        Node for_stmt(const lex::Token& token) {

          return Node{};
        }

        Node while_stmt(const lex::Token& token) {

          return Node{};
        }

        Node enum_stmt(const lex::Token& token) {

          return Node{};
        }

        Node stmt(const lex::Token& token) {
          const lex::Token::Type type = token.type;
          switch(type) {
            case lex::Token::Type::Const:
            case lex::Token::Type::Int64:
            case lex::Token::Type::Int32:
            case lex::Token::Type::Int16:
            case lex::Token::Type::Int8:
            case lex::Token::Type::UInt64:
            case lex::Token::Type::UInt32:
            case lex::Token::Type::UInt16:
            case lex::Token::Type::UInt8:
            case lex::Token::Type::Float32:
            case lex::Token::Type::Float64:
            case lex::Token::Type::Bool:
              return variable(_lexer.next_token());
            case lex::Token::Type::OpenBracket:
              return block_stmt(_lexer.next_token());
            case lex::Token::Type::Component:
              return component(_lexer.next_token());
            case lex::Token::Type::Task:
              return task(_lexer.next_token());
            case lex::Token::Type::System:
              return system(_lexer.next_token());
            case lex::Token::Type::Struct:
              return structure(_lexer.next_token());
            case lex::Token::Type::Archetype:
              return archetype(_lexer.next_token());
            case lex::Token::Type::If:
               return if_stmt(_lexer.next_token());
            case lex::Token::Type::For:
               return for_stmt(_lexer.next_token());
            case lex::Token::Type::While:
               return while_stmt(_lexer.next_token());
            case lex::Token::Type::Enum:
               return enum_stmt(_lexer.next_token());
            default:
              break;
          }

          const auto idx = _ast_nodes.get_size();
          *(_ast_nodes.push(1)) = expr(token, BindingPower::Default);

          lex::Token semi_colon_token = _lexer.next_token();
          if(semi_colon_token.type != lex::Token::Type::SemiColon) {
            post_error(semi_colon_token, "expected semi-colon at end of statement");
          }

          return Node(stmt_t(ExpressionStatement{parse_expr_node(_ast_nodes[idx])}));
        }

      public:
        b8 init(const char* src_file_path, const u32 max_entries = 256, const u32 max_data_size = KB(4)) {
          lofi::File file = lofi::File::create<lofi::FileType::ReadOnly>(src_file_path);
          RX_CHECKF(file.is_open(), "could not properly open script file at src %s", src_file_path);

          const u32 file_size = file.get_size();
          char* text = (char*)_arena.push(file_size);
          file.copy_all_to_buffer((void*)text);
          file.close();

          RX_CHECKF(_lexer.init(text, max_entries, max_data_size),
              "failed to initialize rlsl lexer with source file %s", src_file_path);
          return true;
        }

        const char* get_current_source_file() const {
          return _current_src_file;
        }

        b8 parse(Node& statement_out) {

          statement_out = stmt(_lexer.next_token());

          const u32 error_count = _errors.get_size();
          for(u32 i = 0; i < error_count; i++) {
            LOG_ERROR(_errors[i], RLSL);
          }

          return error_count ? false : true;
        }

        b8 terminate() {
          return _lexer.terminate();
        }

      private:

    };

//    class Interpreter {
//      private:
//        Parser _parser;
//        struct V {
//          struct StmtV {
//            struct ExprV {
//              void operator()(StringExpression string_expr) {
//                PRINT("String = ");
//                PRINT_STRING(string_expr.value);
//                PRINT("\n");
//              }
//              void operator()(BoolExpression bool_expr) {
//                PRINT("Boolean = ");
//                PRINT_C(bool_expr.value);
//                PRINT("\n");
//              }
//
//              struct NumV {
//                void operator()(i64 integer) {
//                  PRINT("\n\t\t\t");
//                  PRINT("%lli\n", integer);
//                }
//
//                void operator()(f64 floating_point) {
//                  PRINT("\n\t\t\t");
//                  PRINT("%f\n", floating_point);
//                }
//              } _num_v;
//
//              void operator()(NumberExpression number_expr) {
//                PRINT("\n\t\t");
//                number_expr.value.visit(_num_v);
//              }
//
//              void operator()(SymbolExpression symbol_expr) {
//                PRINT("\n\t\t");
//                PRINT_STRING(symbol_expr.name);
//                PRINT_S("\n");
//              }
//
//              void operator()(PrefixExpression prefix_expr) {
//                PRINT("\n\t\t");
//                PRINT_S(parser::get_parser_op_type_string(prefix_expr.op));
//                PRINT_S("\n");
//                prefix_expr.right->visit(*this);
//              }
//
//              void operator()(AssignmentExpression assignment_expr) {
//                assignment_expr.left->visit(*this);
//                PRINT("\n\t\t");
//                PRINT_S(parser::get_parser_op_type_string(assignment_expr.op));
//                PRINT_S("\n");
//                assignment_expr.right->visit(*this);
//
//              }
//
//              void operator()(BinaryExpression binary_expr) {
//                PRINT("\n\t\t");
//                binary_expr.left->visit(*this);
//                PRINT_S(parser::get_parser_op_type_string(binary_expr.op));
//                PRINT(" ");
//                binary_expr.right->visit(*this);
//              }
//            } _expr_v;
//
//            void operator()(ExpressionStatement expression_stmt) {
//              PRINT("\n\t");
//              expression_stmt.value->visit(_expr_v);
//
//            }
//            void operator()(BlockStatement block_stmt) {
//              PRINT("\n\t");
//              block_stmt.body->visit(*this);
//            }
//            void operator()(VarDeclarationStatement variable_stmt) {
//              PRINT("\n\t");
//              if(variable_stmt.is_const) {
//                PRINT_S("const ");
//              }
//              PRINT_S(parser::get_parser_type_kind_string(variable_stmt
//                    .type.kind));
//              if(variable_stmt.type.array_size != 1) {
//                PRINT("[%u]", variable_stmt.type.array_size);
//              }
//              PRINT(" ");
//              PRINT_STRING(variable_stmt.name);
//              PRINT(" = ");
//              variable_stmt.value->visit(_expr_v);
//              PRINT(";");
//            }
//          } _stmt_v;
//
//          void operator()(expr_t expr) {
//            PRINT_LINE("Expression");
//            expr.visit(_stmt_v._expr_v);
//          }
//          void operator()(stmt_t stmt) {
//            PRINT_LINE("Statement");
//            stmt.visit(_stmt_v);
//          }
//        } _v;
//
//        void walk(Parser::Node& node) {
//          node.visit(_v);
//        }
//
//      public:
//
//        b8 init(const char* file_path) {
//          RX_CHECKF(_parser.init(file_path), "failed to initialize parser with %s", file_path);
//
//          Parser::Node ast;
//
//          RX_CHECKF(_parser.parse(ast), "failed to parse RLSL file %s", file_path);
//
//          walk(ast);
//
//          return true;
//        }
//
//        b8 terminate() {
//          RX_CHECK(_parser.terminate(),
//              "failed to terminate parser in RLSL Interpreter");
//          return true;
//        }
//    };

#undef EXPECT

  }		// -----  end of namespace rlsl  ----- 
}		// -----  end of namespace roxi  ----- 
