// =====================================================================================
//
//       Filename:  rx_time.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-30 4:52:31 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"

#include <time.h>


namespace roxi {
  class Time {
  private:
#if COMPILER_CL
    static LARGE_INTEGER s_frequency;
#else
    static clock_t s_frequency;
#endif
  public:
    static void                     init() {                // Needs to be called once at startup.

#if COMPILER_CL
      // Cache this value - by Microsoft Docs it will not change during process lifetime.
      QueryPerformanceFrequency(&s_frequency);
#else
      s_frequency = clock();
#endif
    };
    
    static void                     terminate() {           // Needs to be called at shutdown.
      // Nothing to do.
    }

    static i64                      now() {                 // Get current time ticks.

#if COMPILER_CL
      // Get current time
      LARGE_INTEGER time;
      QueryPerformanceCounter( &time );

      // Convert to microseconds
      // const i64 microseconds_per_second = 1000000LL;
      const i64 microseconds = int64_mul_div( time.QuadPart, 1000000LL, s_frequency.QuadPart );
#else
      static const i64 micro_mult = 1000000 / CLOCKS_PER_SEC;
      const clock_t now = clock() - s_frequency;
      const i64 microseconds = now * micro_mult;
#endif
      return microseconds;
    }

    static double                   microseconds( i64 time ) { 
      return (double)time;
    }

    static double                   milliseconds( i64 time ) {
      return (double)time / 1000.0;
    }

    static double                   seconds( i64 time ) {
      return (double)time / 1000000.0;
    }

    static i64                      from( i64 starting_time ) { // Get time difference from start to current time.
      return now() - starting_time;
    }

    static double                   from_microseconds( i64 starting_time ) {
      return microseconds( from( starting_time ) );
    }

    static double                   from_milliseconds( i64 starting_time ) {
      return milliseconds( from( starting_time ) );
    }

    static double                   from_seconds( i64 starting_time ) {
      return seconds( from( starting_time ) );
    }

    static double                   delta_seconds( i64 starting_time, i64 ending_time ) {
      return seconds( ending_time - starting_time );
    }

    static double                   delta_milliseconds( i64 starting_time, i64 ending_time ) {
      return milliseconds( ending_time - starting_time );
    }

    static tm                       localtime() {
      time_t cur_time = time(0);
      tm timestamp;
      localtime_s(&timestamp, &cur_time);
      return timestamp;
    }


    static const char*              localtime_to_string() {
      static char buffer[80];
      tm timestamp = localtime();
      strftime(buffer, 80, "%c", &timestamp);
      return buffer;
    }

  };
}
