// =====================================================================================
//
//       Filename:  rx_compiler.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-14 7:27:57 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "include/rx_parser.hpp"
#include "rx_parser.hpp"

namespace roxi {
  
  namespace rlsl {

    static constexpr u64 RLSLCompilerStringBufferSize = KB(16);

    class Compiler {
    private:
      using arena_t = SizedStackArena<RLSLCompilerStringBufferSize>;
      using error_stack_t = StackArray<Error>;
      arena_t _arena;
      error_stack_t _errors;

      struct MemberData {
        // b8 is_struct = true; // maybe not required
        String type_name;
        String var_name;
      };

      struct StructDefinition {
        Array<MemberData> members;
      };

      using struct_def_list_t = Array<StructDefinition>;
      using struct_handle_t = typename struct_def_list_t::index_t;

      struct_def_list_t _struct_defs;

      struct ComponentDefinition {
        String name;
        Array<MemberData> members;
      };

      using component_def_list_t = Array<ComponentDefinition>;
      using component_handle_t = typename component_def_list_t::index_t; 

      struct TaskDefinition {
        SizedStackArray<component_handle_t, MaxRLSLInputComponentsPerTask> input_component_ids;
        SizedStackArray<component_handle_t, MaxRLSLOutputComponentsPerTask> output_component_ids;
        StringList compiled_task;
      };

      using task_def_list_t = Array<TaskDefinition>;
      using task_handle_t = typename task_def_list_t::index_t;


      struct SystemDefinition {
        Array<String> sub_tasks;
        Array<u32> input_component_ids;
        Array<u32> output_component_ids;
      };

      using system_def_list_t = Array<SystemDefinition>;
      using system_handle_t = typename system_def_list_t::index_t; 

      struct ArchetypeDefinition {
        Array<component_handle_t> component_ids;
      };

      using archetype_def_list_t = Array<ArchetypeDefinition>;
      using archetype_handle_t = typename archetype_def_list_t::index_t; 

      using struct_map_t = StackHashMap<String, struct_handle_t>;
      using component_map_t = StackHashMap<String, component_handle_t>;
      using system_map_t = StackHashMap<String, system_handle_t>;
      using archetype_map_t = StackHashMap<String, archetype_handle_t>;
      using task_map_t = StackHashMap<String, task_handle_t>;


      // this is the intermediate format
      // since the language is declarative, all the operation ordering is done using graphs
      struct CompilerEnvironment {
        struct_map_t _struct_map;
        component_map_t _component_map;
        system_map_t _system_map;
        archetype_map_t _archetype_map;
        task_map_t _task_map;

        struct_def_list_t _structs;
        component_def_list_t _components;
        system_def_list_t _systems;
        archetype_def_list_t _archetypes;
        task_def_list_t _tasks;
      } _compiler_env;

      Parser _parser;
      lofi::File output_header_file;

      void post_error(const u32 line, const char* file, const char* message, ...) {
        va_list args;
        va_start(args, message);

        String string = lofi::str_pushfv(&_arena, message, args);
        *(_errors.push(1)) = Error(line, file, string);
        va_end(args);
      }

      void task_aggregate(const String str) {
        TaskDefinition& task = _compiler_env._tasks[_compiler_env._tasks.get_size() - 1];
        lofi::str_list_push(&_arena, &task.compiled_task, str);
      }

      void task_aggregate(const char* c_str, ...) {
        va_list args;
        va_start(args, c_str);

        String string = lofi::str_pushfv(&_arena, c_str, args);
        TaskDefinition& task = _compiler_env._tasks[_compiler_env._tasks.get_size() - 1];

        lofi::str_list_push(&_arena, &task.compiled_task, string);
        va_end(args);
      }

    public:
      void operator()(StringExpression string_expr) {
        PRINT("String = ");
        PRINT_STRING(string_expr.value);
        PRINT("\n");
      }
      void operator()(BoolExpression bool_expr) {
        PRINT("Boolean = ");
        PRINT_C(bool_expr.value);
        PRINT("\n");
      }
      void operator()(i64 integer) {
        task_aggregate("%lli", integer);
      }

      void operator()(f64 floating_point) {
        task_aggregate("%f", floating_point);
      }

      void operator()(NumberExpression number_expr) {
        number_expr.value.visit(*this);
      }

      void operator()(SymbolExpression symbol_expr) {
        task_aggregate(symbol_expr.name);
        PRINT_S("\n");
      }

      void operator()(PrefixExpression prefix_expr) {
        PRINT("\n\t\t");
        task_aggregate(parser::get_parser_op_type_string(prefix_expr.op));
        PRINT_S("\n");
        prefix_expr.right->visit(*this);
      }

      void operator()(AssignmentExpression assignment_expr) {
        assignment_expr.left->visit(*this);
        task_aggregate(parser::get_parser_op_type_string(assignment_expr.op));
        //file_ref << ' ' << parser::get_parser_op_type_string(assignment_expr.op) << ' ';
        assignment_expr.right->visit(*this);

      }

      void operator()(BinaryExpression binary_expr) {
        binary_expr.left->visit(*this);
        task_aggregate(parser::get_parser_op_type_string(binary_expr.op));
        binary_expr.right->visit(*this);
      }

      void operator()(ExpressionStatement expression_stmt) {
        expression_stmt.value->visit(*this);
        task_aggregate(";");
      }

      void operator()(BlockStatement block_stmt) {
        task_aggregate("\n{\n");
        block_stmt.body->visit(*this);
        task_aggregate("\n}\n");
      }

      void operator()(VarDeclarationStatement variable_stmt) {
        PRINT("\n\t");
        if(variable_stmt.is_const) {
          PRINT_S("const ");
          //file_ref << "const ";
        }
        //file_ref << parser::get_parser_type_kind_string(variable_stmt.type.kind) << ' ';
        PRINT_S(parser::get_parser_type_kind_string(variable_stmt.type.kind));
        PRINT(" ");
        //file_ref << variable_stmt.name << ' ';
        PRINT_STRING(variable_stmt.name);
        variable_stmt.value->visit(*this);
        PRINT(";");
      }

      void operator()(ArchetypeDeclarationStatement archetype_stmt) {
        if(!_compiler_env._archetype_map.has(archetype_stmt.name)) {
          ArchetypeDefinition& definition = *(_compiler_env._archetypes.push(1));
          const auto component_count = archetype_stmt.components.get_size();
          definition.component_ids.move_ptr(_arena.push(sizeof(u32) * component_count));
          definition.component_ids.push(component_count);
          for(u32 i = 0; i < component_count; i++) {
            const auto total_components = _compiler_env._components.get_size();
            for(u32 j = 0; j < total_components; j++) {
              if(lofi::str_compare(archetype_stmt.components[i].component_name, _compiler_env._components[j].name)) {
                definition.component_ids[i] = j;
                break;
              }
            }
          }
        }

      }

      void operator()(ComponentDeclarationStatement component_stmt) {

      }

      void operator()(TaskDeclarationStatement task_stmt) {
        if(!_compiler_env._task_map.has(task_stmt.name)) {
          const auto idx = _compiler_env._tasks.get_size();
          _compiler_env._task_map.insert(task_stmt.name, idx);
          TaskDefinition& task = *(_compiler_env._tasks.push(1));
          const auto input_count = task_stmt.input_components.get_size();
          for(u32 i = 0; i < input_count; i++) {
            String component_name = task_stmt.input_components[i].component_name;
            if(_compiler_env._component_map.has(component_name)) {
              *(task.input_component_ids.push(1)) = _compiler_env._component_map[component_name];
              continue;
            }
            post_error(task_stmt.line, _parser.get_current_source_file()
                , "input component %s in task %s not yet defined!", component_name, task_stmt.name);
          }
          const auto output_count = task_stmt.output_components.get_size();
          for(u32 i = 0; i < output_count; i++) {
            String component_name = task_stmt.output_components[i].component_name;
            if(_compiler_env._component_map.has(component_name)) {
              *(task.output_component_ids.push(1)) = _compiler_env._component_map[component_name];
              continue;
            }
            post_error(task_stmt.line, _parser.get_current_source_file()
                , "output component %s in task %s not yet defined!", component_name, task_stmt.name);
          }
          task_stmt.task.body->visit(*this);
        }
      }

      void operator()(StructDeclarationStatement struct_stmt) {
        if(!_compiler_env._struct_map.has(struct_stmt.name)) {
          const auto idx = _struct_defs.get_size();
          StructDefinition& def = *(_struct_defs.push(1));
          const auto member_count = struct_stmt.members.get_size();
          def.members.move_ptr(_arena.push(sizeof(MemberData) * member_count));
          def.members.push(member_count);
          for(u32 i = 0; i < member_count; i++) {
            def.members[i].type_name = String::create_explicit(&_arena, str_expand(struct_stmt.members[i].type_name));
            def.members[i].var_name = String::create_explicit(&_arena, str_expand(struct_stmt.members[i].variable_name));
          }
          _compiler_env._struct_map.insert(struct_stmt.name, idx);
          return;
        }
        post_error(struct_stmt.line, _parser.get_current_source_file(), "struct %s already defined elsewhere!", (char*)struct_stmt.name.str);
      }
      void operator()(SystemDeclarationStatement system_stmt) {

      }

      void operator()(expr_t expr) {
        PRINT_LINE("Expression");
        expr.visit(*this);
      }
      void operator()(stmt_t stmt) {
        PRINT_LINE("Statement");
        stmt.visit(*this);
      }

      void walk(Parser::Node& node) {
        node.visit(*this);
      }

      b8 init(const char* input_file_path, const char* output_file_path) {
        RX_CHECKF(_parser.init(input_file_path), "failed to initialize parser with %s", input_file_path);

        Parser::Node ast;

        RX_CHECKF(_parser.parse(ast), "failed to parse RLSL file %s", input_file_path);

        walk(ast);


        return true;
      }

      b8 terminate() {
        RX_CHECK(_parser.terminate()
            , "failed to terminate parser in RLSL Compiler");
        return true;
      }

    };



  }		// -----  end of namespace rlsl  ----- 

}		// -----  end of namespace roxi  ----- 
