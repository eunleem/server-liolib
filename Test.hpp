#ifndef _TEST_HPP_
#define _TEST_HPP_

#include "liolib/Debug.hpp"
#include "liolib/Log.hpp"
#include "liolib/Colors.hpp"

#include <chrono>
#include <string>
#include <map>



#ifndef _PERFTEST
  #define _PERFTEST false
#endif


#if _PERFTEST
  #define PERFTEST if (1)
#else
  #define PERFTEST if (0)
#endif



namespace lio {

typedef std::chrono::high_resolution_clock::time_point highrestime;

namespace Test {

  class Perf {
  public:
    static
    highrestime TimerStart(const std::string& testName) { 
      const highrestime now = std::chrono::high_resolution_clock::now();
      Perf::Timers[testName] = std::pair<highrestime, highrestime> (now, now);
      return now;
    }
    
    highrestime TimerEnd(const std::string& testName, bool showResult = false) { 
      highrestime now = std::chrono::high_resolution_clock::now();
      Perf::Timers[testName].second = now;

      if (showResult == true) {
        Perf::GetResult(testName);
      }

      return now;
    }

    std::chrono::microseconds GetResult(const std::string& testName) {
      auto testItr = Perf::Timers.find(testName);
      if (testItr == Perf::Timers.end()) {
        std::cerr << "Performance Test not found. name: " << testName << std::endl;
        return std::chrono::microseconds(0);
      }

      auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(testItr->second.second- testItr->second.first);
      std::cout << "Performance Test Result for " << testName << ": " << std::endl << TCOLOR::BROWN <<
        std::setw(35) << std::right << total_us.count() << " us" << TCOLOR::END << std::endl;

      return total_us;
    }


    static std::map<std::string, std::pair<highrestime, highrestime>> Timers;
  };
}

}

#else

#endif
