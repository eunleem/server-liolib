#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

/**
 * #REF: http://www.drdobbs.com/cc-tip-5-a-cc-comment-macro/184401344 
 * #REF: http://www.drdobbs.com/more-on-the-cc-comment-macro-for-debug-s/184401612 
 */

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>

#include <cassert>
#include <cstring>

#include <sys/types.h> // pid_t
#include <unistd.h> // getpid();

//#include <errno.h>

#include "liolib/Colors.hpp"

using std::endl;

namespace lio {
  class Debug {
  public:
    enum class Type {
      ERROR = 0, // Probably not really needed.
      WARNING = 1,
      POINT = 2,
      ARGS = 3,
      FUNC_RESULT = 4,
      SUB_RESULT = 5,
      DETAIL = 6,
      TEMP = 7
    };

    static
    std::ostream& Print(Type type) {
      return getStream(type) << getColor(type) << "DBG " << toString(type)
                             << TCOLOR::RESET << TCOLOR::BLUE << std::setw(6)
                             << std::right << getpid() << " ";
    }

  private:
    inline static
    std::ostream& getStream(Type type) {
      return std::cout;
    }

    inline static
    std::string getColor(Type type) {
       switch (type) {
         case Type::ERROR:
           return TCOLOR::REDBG;
         case Type::WARNING:
           return TCOLOR::RED;
         case Type::POINT:
           return TCOLOR::GREEN;
         default:
           return TCOLOR::BLUE;
       }
       return TCOLOR::BLUE;
    }


    inline static
    std::string toString(Type type) {
      switch (type) {
        case Type::ERROR: return "ERR ";
        case Type::WARNING: return "WRN ";
        case Type::POINT: return "PNT ";
        case Type::ARGS: return "ARG ";
        case Type::FUNC_RESULT: return "FRT ";
        case Type::SUB_RESULT: return "SRT ";
        case Type::DETAIL: return "DTL ";
        case Type::TEMP: return "TMP ";
      }
      return "MSG ";
    }
  };
}

#ifndef _DEBUG
  #define _DEBUG true
#endif

#if _DEBUG

#ifndef _FILE_LINE_INFO
#define _FILE_LINE_INFO                                                        \
  std::setw(30) << std::left << __FILE__ << ":" << std::setw(5) << std::right  \
                << __LINE__ << " : " << TCOLOR::END
#endif

#define _DEBUGFUNC(x) lio::Debug::Print(x)

#define DEPRECATED_FUNC(alt)                                                   \
  _DEBUGFUNC(lio::Debug::Type::WARNING)                                        \
      << _FILE_LINE_INFO << TCOLOR::BROWN << __func__ << " is DEPRECTAED! "    \
      << "Use " << alt << " instead!" << TCOLOR::RESET << std::endl;

#define DEBUG_FUNC_START                                                       \
  _DEBUGFUNC(lio::Debug::Type::POINT)                                          \
      << _FILE_LINE_INFO << TCOLOR::END << TCOLOR::BROWN << __func__           \
      << " called." << TCOLOR::END << std::endl;

#define DEBUG_POINT(pointName)                                                 \
  _DEBUGFUNC(lio::Debug::Type::POINT)                                          \
      << _FILE_LINE_INFO << TCOLOR::END << TCOLOR::GREEN                       \
      << "POINT NAME: " << pointName << TCOLOR::END << std::endl;

#define DEBUG_err _DEBUGFUNC(lio::Debug::Type::ERROR) << _FILE_LINE_INFO
#define DEBUG_warn _DEBUGFUNC(lio::Debug::Type::WARNING) << _FILE_LINE_INFO
#define DEBUG_args _DEBUGFUNC(lio::Debug::Type::ARGS) << _FILE_LINE_INFO
#define DEBUG_result _DEBUGFUNC(lio::Debug::Type::FUNC_RESULT) << _FILE_LINE_INFO
#define DEBUG_subresult _DEBUGFUNC(lio::Debug::Type::SUB_RESULT) << _FILE_LINE_INFO
#define DEBUG_detail _DEBUGFUNC(lio::Debug::Type::DETAIL) << _FILE_LINE_INFO
#define DEBUG_temp _DEBUGFUNC(lio::Debug::Type::TEMP) << _FILE_LINE_INFO

#define DEBUG_point(x) DEBUG_POINT(x)

#define DEBUG_cout DEBUG_detail
#define DEBUG_cerr DEBUG_err
#define DEBUG_clog DEBUG_detail

#define DEBUG if (1)
#define PROD if (0)

#else
  #define DEPRECATED_FUNC(x)
  #define DEBUG_FUNC_START
  #define DEBUG_POINT

  #define DEBUG_err if(0) std::cerr
  #define DEBUG_warn if(0) std::cerr
  #define DEBUG_args if(0) std::cerr
  #define DEBUG_result if(0) std::cerr
  #define DEBUG_subresult if(0) std::cerr
  #define DEBUG_detail if(0) std::cerr
  #define DEBUG_temp if(0) std::cerr

  #define DEBUG_cout if(0) std::cerr

  #define DEBUG_cerr                                                             \
    std::cerr << "ERR " << getpid() << " " << __FILE__ << ": " << __func__       \
              << " : " << __LINE__ << " : "

  #define DEBUG_clog std::clog << "LOG " << \
    getpid() << " " << \
    __FILE__ << ":" << \
    __func__ << " : " << \
    __LINE__ << " : "

  #define DEBUG if(0)
  #define PROD if(1) 
#endif


#endif
