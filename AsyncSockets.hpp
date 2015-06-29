#ifndef _ASYNCSOCKET_HPP_
#define _ASYNCSOCKET_HPP_

/*
  Name
    AsyncSocket

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description
    Provides Asynchronous Socket (event-based) input.

  Last Modified Date
    Apr 11, 2015

  History
    July 17, 2013
      Decoupling with Socket class.
      Socket class is used as composition rather than inheritance.
        (Declared a variable that has type Socket)

  To Dos
    Support Multiple Port Listening. Epoll can handle multiple sockets.
      Challenges are modifying this class compatible with existing programs that I made.
      Which... isn't too difficult. But doing it elegant is very... challenging.
      Should I create a new class??

      First, I am creating a MESSY version of supporting Multiple Sockets. Actually only 2.

    Recreate/Refactor AsyncSocket class to support Multiple Sockets in a elegant way.



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
#include <list> // std::list

#include <cstdio> // NULL
#include <cstdlib> // calloc(), free(), exit()
#include <cstdint> // uint16_t

#include <unistd.h> // fcntl(), read(), write()
#include <fcntl.h> // fcntl()

#include <sys/epoll.h>

#include "liolib/Consts.hpp"

#include "liolib/Socket.hpp"
#include "liolib/Util.hpp"



namespace lio {

using std::string;
using std::map;

class AsyncSockets {
public:
// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  NOT_INITIALIZED,
  EPOLL_ERROR,
  NON_BLOCK_ERROR,
  SOCKET_ERROR,
  ACCEPT_ERROR,
  CONNECT_ERROR,
  WRITE_ERROR
};
#define ASYNCSOCKETS_EXCEPTION_MESSAGES \
  "AsyncSocket Exception has been thrown.", \
  "Socket has not been initialized.", \
  "Error occurred while working with Epoll.", \
  "Failed to make the socket NonBlocking.", \
  "Socket Error. Check if Socket is opened.", \
  "Socket Accept has failed.", \
  "Connect Error.", \
  "Write Error."
 
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
    OPEN_READY,
    OPEN,
    CLOSING,
    CLOSED
  };

  struct Connection {
    Connection() :
      in_len(0),
      portNumber(0),
      isSecure(false)
    { }
    Connection(struct sockaddr& in_addr, socklen_t& in_len,
        uint16_t portNumber = 0, bool isSecure = false) :
      in_addr(in_addr),
      in_len(in_len),
      portNumber(portNumber),
      isSecure(isSecure)
    { }
    Socket::SocketFamily GetAddressFamily() const {
      switch (this->in_addr.sa_family) {
        case PF_UNIX:
          return Socket::SocketFamily::LOCAL;
        case PF_INET:
          return Socket::SocketFamily::IPv4;
        case PF_INET6:
          return Socket::SocketFamily::IPv6;
      } 
      return Socket::SocketFamily::UNDEF;
    }
    const char* GetAddressData() const {
      return this->in_addr.sa_data;
    }
    struct sockaddr in_addr;
    socklen_t in_len;
    uint16_t portNumber;
    bool isSecure;
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

  struct SocketEventArgs {
    SocketEventArgs(int sockFd) :
      socketFd(sockFd) { }
    const int socketFd;
  };

  struct AcceptEventArgs {
    AcceptEventArgs(int acptFd, Connection connection) :
      acceptFd(acptFd),
      connection(connection) { }
    const int acceptFd;
    const Connection connection;
  };

  
  AsyncSockets ();

  virtual
  ~AsyncSockets ();

  // For Domain Socket
  Socket*       AddSocket(
                  const string& sockName,
                  const Socket::SocketFamily sockFamily = Socket::SocketFamily::LOCAL,
                  const Socket::SocketType sockType = Socket::SocketType::TCP);

  // For Network Socket
  Socket*       AddSocket(
                  const uint16_t portNumber,
                  const Socket::SocketFamily sockFamily = Socket::SocketFamily::IP,
                  const Socket::SocketType sockType = Socket::SocketType::TCP);

  int           GetEpollFd() const;
  void          SetNumMaxEvent(const int maxEvent);

  int           GetSocketFd(uint16_t portNumber) const;
  int           GetSocketFd(const string& sockName) const;

  void          Listen(const uint16_t portNumber);
  void          Listen(const string& sockName);

  void          Wait();

  //void          Connect(const string& sockName);
  //void          Connect(const uint16_t portNumber, const string& destIpAddr);

  //int           Write(const string& content);
  //int           Write(const void* dataLocation, const size_t length);
  //ssize_t       Read(string* content);

  void          Close(uint16_t portNumber);
  void          Close(const string& sockName);

  static
  int           createEpoll();
  static
  bool          addFdToEpoll (const int epollFd, const int targetFd, const uint32_t events = 0);
  static
  void          removeFdFromEpoll (const int fd);
  static
  void          setNonBlocking(const int fd);

protected:
  void          waitForEvent(ssize_t epollTimeOut = -1);

  virtual
  void          OnFdEvent(const FdEventArgs& event) = 0;
  
  
  std::map<uint16_t, std::pair<Socket*, Socket::SocketMode>> networkSockets_;
  std::map<string, std::pair<Socket*, Socket::SocketMode>> domainSockets_;

  int           numMaxEvent_;

  int           epollFd_;



private:
  static const int defaultNumEventMax_;
  bool isSetToStop_;
  struct epoll_event* events_;


  Status status_;
};

}

#endif
