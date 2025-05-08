// =====================================================================================
//
//       Filename:  rx_thread_pool.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:50:30 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_thread_pool.hpp"

template class lofi::ThreadPool<roxi::RoxiNumThreads, roxi::RoxiNumFibers>;
