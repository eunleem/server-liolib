#ifndef _FILELOADER_HPP_
#define _FILELOADER_HPP_
/*
  Name
    FileLoader

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 10, 2014
  
  History
    April 10, 2014
      Created

  ToDos
    Apr 10, 2014
      LoadFile allocates memory using malloc() and returns pointer in DataBlock<>. 
      The main problem is... user of this class MUST know to free memory by calling free()
      But it isn't so obvious.
      Return Object that is obvious in how to free memory would be nice to add in this class.
    


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

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "liolib/DataBlock.hpp"

namespace lio {

using std::string;
using std::map;
using std::fstream;
using std::ifstream;

class FileLoader {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define FILELOADER_EXCEPTION_MESSAGES \
  "FileLoader Exception has been thrown."

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

private:

public:

  FileLoader();
  ~FileLoader();

  static
  DataBlock<void*> LoadFile(const string& filePath);
  //bool RemoveFileFromCache(const string& filePath);
protected:

private:
  
  
};


}

#endif

