#ifndef _HTTPCONNECTION_HPP_
#define _HTTPCONNECTION_HPP_
/*
  Name
    HttpConnection

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 16, 2014
  
  History
    April 03, 2014
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

#include <chrono>

#include "liolib/Consts.hpp"
#include "liolib/HttpWork.hpp"
#include "liolib/MemoryPool.hpp"
#include "liolib/DataBlock.hpp"

#include "liolib/Util.hpp"

namespace lio {

using std::chrono::system_clock;
using consts::ERROR;
using consts::STRING_NOT_FOUND;


class HttpConnection final {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  WORK_NOT_READY
};
#define HTTPCONNECTION_EXCEPTION_MESSAGES \
  "HttpConnection Exception has been thrown.", \
  "Popping incomplete work."

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

  enum class Status : uint8_t {
    NEW,
    READY,
    READING,
    READ_HEADER,
    READING_BODY,
    DONE_READING,
    CLOSED
  };

  HttpConnection(MemoryPool* mp);
  ~HttpConnection();

  int GetFd() const;
  void SetFd(const int fd);
  Status ReadRequest();

  HttpWork* PopWork();

  void Close();

  static
  size_t MAX_BUFFER_SIZE;

  Status status;
  bool isKeepAlive;

  HttpWork* currentWork;

  system_clock::time_point openedTime;
  system_clock::time_point expirationTime;

private:
  int fd;

  MemoryPool* mpBuffer_;



  
};

}

#endif

