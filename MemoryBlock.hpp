#ifndef _MEMORYBLOCK_HPP_
#define _MEMORYBLOCK_HPP_
/*
  Name
    MemoryBlock

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 14, 2014
  
  History
    April 14, 2014
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

#include "liolib/Debug.hpp"


#include "liolib/DataBlock.hpp"

namespace lio {


class MemoryBlock final : public DataBlock<void*>  {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define MEMORYBLOCK_EXCEPTION_MESSAGES \
  "MemoryBlock Exception has been thrown."

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


  MemoryBlock();
  ~MemoryBlock();
protected:
  
private:

/** FUNC_NAME 
 *    DESCRIPTION
 *  Input
 *    DESCRIPTION_ABOUT_PARAMETERS
 *  Returns
 *    void NOTHING
 */
  void sampleFunc();
  
};

}

#endif

