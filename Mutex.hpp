#ifndef _MUTEX_HPP_
#define _MUTEX_HPP_
/*
  Name
    Mutex

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    February 12, 2014
  
  History
    February 12, 2014
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

#include "Debug.hpp"
//#include "liolib/Debug.hpp"

#include <iostream>

namespace lio {


class Mutex {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define MUTEX_EXCEPTION_MESSAGES \
  "Mutex Exception has been thrown."

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


  Mutex();
  ~Mutex();
protected:
  
private:

  /* Name
   *  sampleFunc
   * Description
   *  Description of the function here.
   * Input
   *  description about parameters
   * Output
   *  Returns
        void NOTHING
   *  Changes
   *    variables or things that this function changes 
   *     
   */
  void sampleFunc();

};

}

#endif

