#ifndef _ASYNCIO_HPP_
#define _ASYNCID_HPP_

/*
  Name
    AsyncIo

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description
    Provides Asynchronous (event-based) input output.

  Last Modified Date
    Mar 26, 2014

  History
    July 17, 2013
      Decoupling with Socket class.
      Socket class is used as composition rather than inheritance.
        (Declared a variable that has type Socket)


  Learning Resources
    NonBlocking Concept
      http://www.kegel.com/dkftpbench/nonblocking.html
    Epoll Definition
      http://linux.die.net/man/4/epoll
    Epoll Implementation Example
      #TODO: Add Examples

    Function Pointers for Event-based programming.

  Copyright (c) All Rights reserved to LIFEINO.
*/


/*
 * HEADER ORDER
 *
 *  DEBUG.hpp first
 
 *  Std C++ Library
 *  Ported C library
 *
 *  Classic C Library
 *  System Library
 *
 *  My Library
 */

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp" // DEBUG, DEBUG_cout, DEBUG_cerr

#include <string> // std::string
#include <map> // std::map

#include <cstdio> // NULL
#include <cstdlib> // calloc(), free(), exit()
#include <cstdint> // uint16_t

#include <unistd.h> // fcntl(), read(), write()
#include <fcntl.h> // fcntl()

#include <sys/epoll.h>

#include "liolib/Consts.hpp"

#include "liolib/Util.hpp"


namespace lio {

using std::string;
using std::map;


class AsyncIo {
public:
// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  NOT_INITIALIZED,
  EPOLL_ERROR,
  NON_BLOCK_ERROR
};
#define ASYNCIO_EXCEPTION_MESSAGES \
  "AsyncIo Exception has been thrown.", \
  "Socket has not been initialized.", \
  "Failed to created Epoll.", \
  "Failed to make the socket NonBlocking."
 
class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  ExceptionType               exceptionType_;
  static const char* const    exceptionMessages_[];
};

// ******** Exception Declaration END*********

  enum class Status {
    INIT,
    WAITING,
    STOPPING,
    STOPPED
  };

  struct FdEventArgs {
    enum class EventType : uint8_t {
      EPOLLIN,
      EPOLLOUT
    };
    FdEventArgs(int eventFd, EventType eventType) :
      fd(eventFd),
      type(eventType)
    { }
    const int fd;
    const EventType type;
  };

  
  // For SERVER.
  AsyncIo (const int fd);
  virtual
  ~AsyncIo ();

  int           GetEpollFd() const; // Not really needed anymore...?

  void          StartWaiting();

  void          Stop();
  void          StopGracefully();

  static
  int           createEpoll();
  static
  bool          addFdToEpoll (const int epollFd, const int targetFd, const uint32_t events = 0);
  static
  void          removeFdFromEpoll (const int fd);
  static
  void          setNonBlocking(const int fd);

protected:
  void          waitForEvent();

  virtual
  void          OnFdEvent(const FdEventArgs& event);
  
  const
  uint16_t      numMaxEvent_;

  int           epollFd_;


  void          setToStop();

private:
  int fd_;
  Status status_;
  bool isSetToStop_;
  struct epoll_event* events_;

};

}

#endif
