// =====================================================================================
//
//       Filename:  rx_load.h
//
//    Description:  file streaming and loading 
//
//        Version:  1.0
//        Created:  2025-01-12 4:21:51 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "pods.hpp"
#include "rx_allocator.hpp"
#include "data/data.hpp"

#define ROXI_FILE_TYPES(X) X(GLTF) X(Obj) X(PNG) X(WAV)

namespace roxi {

  enum class FileType {
#define ENUM(X) X,
    ROXI_FILE_TYPES(ENUM)
#undef ENUM
  };

  struct GLTFData {
    StackArray<BoneInfo>

  };

  struct PNGData {

  };


  struct WAVData {

  };
  static const char* get_file_type_string(const FileType type) {
    static const char* const arr[] = {
#define STRING(X) STRINGIFY(X),
      ROXI_FILE_TYPES(STRING)
#undef STRING
    };
    return arr[(u8)type];
  }

  class FileStream {
  private:
    lofi::File file;

  public:

    template<FileType Type>
    b8 open_read_only(const char* file_path) {
      if(file.is_open()) {
        LOGF(Error, "file stream could not open file %s, because the file is already open elsewhere", file_path);
        return false;
      }
      file = lofi::File::create<::lofi::FileType::ReadOnly>(file_path);
      if(!file.is_open()) {
        LOGF(Error, "failed to open file %s as read only file", file_path);
        return false;
      }
      return true;
    }

    // functions for reading in specific files and return data to the application

    b8 close() {
      return file.close();
    }






  };
 


}		// -----  end of namespace roxi  ----- 
