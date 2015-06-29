#ifndef _ASYNCINOTIFY_HPP_
#define _ASYNCINOTIFY_HPP_
/*
  Name
    AsyncInotify

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    March 25, 2014
  
  History
    March 25, 2014
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


//#inclue "liolib/DataBlock.hpp"

namespace lio {


class AsyncInotify {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define ASYNCINOTIFY_EXCEPTION_MESSAGES \
  "AsyncInotify Exception has been thrown."

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


  AsyncInotify();
  ~AsyncInotify();
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

