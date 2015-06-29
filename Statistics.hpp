#ifndef _STATISTICS_HPP_
#define _STATISTICS_HPP_
/*
  Name
    Statistics

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    November 14, 2014
  
  History
    November 14, 2014
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


//#include "liolib/DataBlock.hpp"

namespace lio {


class Statistics {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define STATISTICS_EXCEPTION_MESSAGES \
  "Statistics Exception has been thrown."

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


  Statistics();
  ~Statistics();
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

