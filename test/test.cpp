// =====================================================================================
//
//       Filename:  main.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-11-29 4:22:38 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "Application.h"

int main(int argc, char** argv) {
  PRINT_LINE("beginning program");
  roxi::Application application;
  if(!application.run())
    PRINT_LINE("application failed!!\n");
  else
    PRINT_LINE("application succeeded!!\n");
  PRINT_LINE("halting program");
  return 0;
}

