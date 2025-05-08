// =====================================================================================
//
//       Filename:  rx_lua.hpp
//
//    Description:  lua environment and functionality
//
//        Version:  1.0
//        Created:  2025-01-06 11:31:10 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "lua.h"
#include "lua.hpp"
#include "rx_allocator.hpp"
#include "rx_log.hpp"
#include "rx_dynamic_ecs.hpp"


namespace roxi {
 
  namespace lua {

    class Environment {
    private:
      lua_State* _env;
      lofi::File _script;
      static ECS _ecs;

      static int create_system(lua_State* l) {
        if(!lua_isstring(l, -4)) {
          LOG("System.new() first argument must be of type string", Error);
          return -1;
        }
        if(!lua_isfunction(l, -3)) {
          LOG("System.new() second argument must be of type function", Error);
          return -1;
        }
        if(!lua_istable(l, -2)) {
          LOG("System.new() third argument must be of type table", Error);
          return -1;
        }
        if(!lua_istable(l, -1)) {
          LOG("System.new() fourth argument must be of type table", Error);
          return -1;
        }
        const u64 num_input_args = lua_rawlen(l, -2);
        const u64 num_output_args = lua_rawlen(l, -1);
        const char* name = lua_tostring(l, -4);

        Array<u32> input_components{ALLOCATE(sizeof(u32) * num_input_args)};
        Array<u32> output_components{ALLOCATE(sizeof(u32) * num_output_args)};

        const u32 iter_count = MAX(num_input_args, num_output_args);

        for(u32 i = 0; i <= iter_count; i++) {
          // input component
          if(i < num_input_args) {
            lua_pushinteger(l, i + 1);
            lua_gettable(l, -3);
            lua_pushstring(l, "id");
            lua_gettable(l, -2);
            input_components[i] = lua_tonumber(l, -1);
            lua_pop(l, 1);
          }
          if(i < num_output_args) {
            lua_pushinteger(l, i + 1);
            lua_gettable(l, -2);
            lua_pushstring(l, "id");
            lua_gettable(l, -2);
            output_components[i] = lua_tonumber(l, -1);
            lua_pop(l, 1);
          }
        }

        const u32 system_id = _ecs.create_system
          ( name
          , input_components.get_size()
          , output_components.get_size()
          , input_components.get_buffer()
          , output_components.get_buffer()
          );
        if(system_id == MAX_u32) {
          LOGF(Error, "failed to create ecs system %s", name);
          return -1;
        }
        
        lua_newtable(l);

        lua_pushstring(l, "name");
        lua_pushvalue(l, -6);
        lua_settable(l, -3);

        lua_pushstring(l, "task");
        lua_pushvalue(l, -5);
        lua_settable(l, -3);

        lua_pushstring(l, "id");
        lua_pushinteger(l, system_id);
        lua_settable(l, -3);

        lua_pushstring(l, "inputs");
        lua_pushvalue(l, -4);
        lua_settable(l, -3);

        lua_pushstring(l, "outputs");
        lua_pushvalue(l, -3);
        lua_settable(l, -3);

        return 1;
      }

      static int create_component(lua_State* l) {



        return 1;
      }

      void create_lua_system_interface() {
        lua_newtable(_env);
        lua_pushstring(_env, "new");
        lua_pushcfunction(_env, &Environment::create_system);
        lua_settable(_env, -3);

      }

      void create_lua_component_interface() {
        lua_newtable(_env);
        lua_pushstring(_env, "new");
        lua_pushcfunction(_env, &Environment::create_component);
      }

    public:
      b8 init(const char* file_path) {

        _ecs.init(1024, 64, 256);

        _env = luaL_newstate();
        luaL_openlibs(_env);

        create_lua_system_interface();
        create_lua_component_interface();

        lofi::File _script = lofi::File::create<lofi::FileType::ReadOnly>(file_path);

        if(!_script.is_open()) {
          LOGF(Error
              , "failed to open lua script file %s"
              , file_path);

          return false;
        }


        return true;
      }

      b8 update() {

        return true;
      }


      b8 terminate() {

        lua_close(_env);

        if(!_script.close()) {
          LOG("failed to close lua script file", Error);
          return false;
        }

        if(!_ecs.terminate()) {
          LOG("failed to terminate lua environment static ECS", Error);
          return false;
        }
        return true;
      }


    };

  }		// -----  end of namespace lua  ----- 

}		// -----  end of namespace roxi  ----- 
