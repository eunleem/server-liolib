#ifndef _FILE_HPP_
#define _FILE_HPP_
/*
  Name
    File

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    March 16, 2014
  
  History
    March 16, 2014
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
#define _DEBUG false

#include "liolib/Debug.hpp"


//#inclue "liolib/DataBlock.hpp"

namespace lio {


class File {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define FILE_EXCEPTION_MESSAGES \
  "File Exception has been thrown."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType       exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********


  File();
  virtual
  ~File();

  void Test();
  bool RetBool(bool val);
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

