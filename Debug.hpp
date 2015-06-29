#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

/**
 * #REF: http://www.drdobbs.com/cc-tip-5-a-cc-comment-macro/184401344 
 * #REF: http://www.drdobbs.com/more-on-the-cc-comment-macro-for-debug-s/184401612 
 */

#include <iostream>
#include <iomanip>
#include <string>

#include <cassert>

#include <sys/types.h> // pid_t
#include <unistd.h> // getpid();

//#include <errno.h>

//#include "liolib/Util.hpp" // Timestamp();
#include "liolib/Macro.hpp"

using std::cout;
using std::cerr;
using std::clog;
using std::endl;
using std::string;

#ifndef _DEBUG
  #define _DEBUG true
#endif

#if _DEBUG
  //#define _GNU_SOURCE
  
 // #TODO: getpid() get rid of it. 
  #define DEBUG_FUNC_START cout << COLOR::BLUE <<  "DBG " << \
    std::setw(5) << std::right << getpid() << " " << \
    std::setw(30) << std::left << __FILE__ << ":" << \
    std::setw(5) << std::right <<  __LINE__ << " : " << COLOR::END << \
    COLOR::BROWN << __func__ << " called." << COLOR::END << endl;
  
  #define DEBUG_POINT cout << COLOR::BLUE <<  "DBG " << \
    std::setw(5) << std::right << getpid() << " " << \
    std::setw(30) << std::left << __FILE__ << ":" << \
    std::setw(5) << std::right <<  __LINE__ << " : " << COLOR::END << \
    COLOR::GREEN << "POINT REACHED in " << __func__ << COLOR::END << endl;


  #define DEBUG_cout0 cout << COLOR::BLUE <<  "DBG " << \
    std::setw(5) << std::right << getpid() << " " << \
    std::setw(30) << std::left << __FILE__ << ":" << \
    std::setw(5) << std::right <<  __LINE__ << " : " << COLOR::END

  #define DEBUG_cout cout << COLOR::BLUE <<  "DBG " << \
    std::setw(5) << std::right << getpid() << " " << \
    std::setw(30) << std::left << __FILE__ << ":" << \
    std::setw(5) << std::right <<  __LINE__ << " : " << COLOR::END
 
  #define DEBUG_cerr cerr << COLOR::RED << "ERR " << \
    std::setw(5) << std::right << getpid() << " " << \
    std::setw(30) << std::left << __FILE__ << ":" << \
    std::setw(5) << std::right << __LINE__ << " : " << COLOR::END 

  #define DEBUG_clog clog << COLOR::BLUE << "LOG " << \
    std::setw(5) << std::right << getpid() << " " << \
    std::setw(30) << std::left << __FILE__ << ":" << \
    std::setw(5) << std::right << __LINE__ << " : " << COLOR::END 


  #define DEBUG if(1) 

  #define PROD if(0) 

#else
  #define DEBUG_FUNC_START
  #define DEBUG_POINT

  #define DEBUG_cout0 if(0) cerr
  #define DEBUG_cout if(0) cerr
  #define DEBUG_cerr cerr << "ERR " << \
    getpid() << " " << \
    __FILE__ << ": " << \
    __func__ << " : " << \
    __LINE__ << " : " 

  #define DEBUG_clog clog << "LOG " << \
    getpid() << " " << \
    __FILE__ << ":" << \
    __func__ << " : " << \
    __LINE__ << " : "

  #define DEBUG if(0)

  #define PROD if(1) 
#endif


#endif
