#ifndef _MEMORYPOOLMANAGER_HPP_
#define _MEMORYPOOLMANAGER_HPP_
/*
  Name
    MemoryPoolManager

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    February 27, 2014
  
  History
    February 27, 2014
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

//#include "Debug.hpp"
#include "liolib/Debug.hpp"
#include "liolib/MemoryPool.hpp"

#include <string> // std::string
#include <vector> // std::vector


namespace lio {

using std::vector;
using std::string;

class MemoryPoolManager {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define MEMORYPOOLMANAGER_EXCEPTION_MESSAGES \
  "MemoryPoolManager Exception has been thrown."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********

struct Config {
  Config() 
    : numPoolMax(4),
      poolSizeMax(1024 * 1024 * 4) // 4MB
      { } 

  int numPoolMax;
  int poolSizeMax;
  MemoryPool::Config defaultMpConfig;


};


  MemoryPoolManager(Config& config);
  ~MemoryPoolManager();

  void* const Mpalloc(size_t allocSize);
  size_t Mpfree (const void* freePtr);

protected:
  
private:
  Config config_;
  vector<MemoryPool*> poolList_;
  void addPool (MemoryPool::Config& poolConfig);
  
};

}

#endif

