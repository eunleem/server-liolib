#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_

/*
  Name
    Socket

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description
    Basic Socket Socket class.
    Each instance can bind to a single port for Listen.
    
  History
    July 13, 2013
      Major Refactoring

  Last Modified Date
    Dec 02, 2014

  Learning Resources
    NonBlocking Concept
      http://www.kegel.com/dkftpbench/nonblocking.html
    Epoll Definition
      http://linux.die.net/man/4/epoll

  TODO Notes
    [ETL] Consider moving portNumber and sockType setting to Constructor.
  
  Copyright (c) All Rights reserved to LIFEINO.
*/



/*
 * HEADER ODER
 *
 *  DEBUG.hpp first
 *
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
#define _DEBUG false
#define DEVEL if(1)

#include "liolib/Debug.hpp"

#include "liolib/Consts.hpp"

#include <iostream>
#include <string>
//#include <vector> // vector

#include <cassert> // assert();
#include <cstdio> // sprintf
#include <cstdint> // uint8_t, uint16_t
#include <cstring> // memset(), strncpy(), strerror()

#include <sys/types.h> // getaddrinfo(), gai_strerror()
#include <sys/socket.h> // getaddrinfo(), gai_strerror()
#include <netdb.h> // getaddrinfo(), gai_strerror()
#include <sys/un.h> // sockaddr_un, UNIX_PATH_MAX // I copied UNIX_PATH_MAX from linux/un.h to sys/un.h

#ifndef UNIX_PATH_MAX
  #define UNIX_PATH_MAX 108
#endif

#include <unistd.h> // close()
#include <fcntl.h> // fcntl()

#include "liolib/Util.hpp" // String::ToUInt()

namespace lio
{
using std::string;
using std::cout;
using std::endl;
using std::cerr;

using consts::OK;
using consts::ERROR;

// ******** Exception Declaration *********
enum class SocketExceptionType : std::uint8_t {
  GENERAL,
  INIT_FAILED,
  NOT_INITIALIZED,
  SOCKET_FAIL,
  BIND_FAIL,
  LISTEN_FAIL,
  CONNECT_FAIL,
  WRITE_FAIL,
  READ_FAIL
};
#define SOCKET_EXCEPTION_MESSAGES \
  "Socket Exception has been thrown.", \
  "Failed to initialize Socket." , \
  "Socket Not Initialized.", \
  "System call socket() has failed.", \
  "System call bind() has failed.", \
  "System call listen() has failed.", \
  "System call connect() has failed.", \
  "Socket Write failed.", \
  "Socket Read failed."

class SocketException : public std::exception {
public:
  SocketException (SocketExceptionType exceptionType = SocketExceptionType::GENERAL);

  virtual const
  char*                 what() const noexcept;
  virtual const
  SocketExceptionType   type() const noexcept;
  
private:
  SocketExceptionType   exceptionType;
  static const
  char* const           exceptionMessages[];
};

// ******** Exception Declaration END*********


class Socket {
 public:
  enum class SocketMode : std::uint8_t {
    UNDEF,
    LISTEN,
    CONNECT
  };
  // Implement SocketStatus Later when it's really needed.
  enum class SocketStatus : std::uint8_t {
    INIT,
    LISTENING,
    CONNECTED,
    ERROR,
    CLOSED
  };
  // Const Reference
  //  /usr/include/bits/socket.h
  enum class SocketFamily {
    UNDEF = -1,
    LOCAL = PF_UNIX, // PF_LOCAL
    IP = PF_UNSPEC, // Automatically decides between IPv4 and IPv6
    IPv4 = PF_INET,
    IPv6 = PF_INET6
  };

  enum class SocketType {
    UNDEF = -1,
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
  };

  // For creating new socket.
  Socket(const SocketFamily sockFamily = SocketFamily::IP,
         const SocketType sockType = SocketType::TCP);

  // For importing already existing socket.
  Socket(const int existingFd,
         SocketMode mode,
         const SocketFamily sockFamily = SocketFamily::IP,
         const SocketType sockType = SocketType::TCP);

  virtual
  ~Socket();

  // Simple Get/Set
  int           GetSocketFd() const;
  SocketFamily  GetSocketFamily() const;
  SocketType    GetSocketType() const;
  SocketMode    GetSocketMode() const;

  void          ReportStatus() const;


  // LOCAL Listen and Connect
  bool        Listen (const string& sockName); // socket path
  bool        Connect (const string& sockName, int retryInterval = 3); // socket path
  // IP Listen and Connect
  bool        Listen (const uint16_t portNumber);
  bool        Connect (const uint16_t portNumber, const string& destIpAddr);


  int         Write(const void* dataLocation, const size_t length);
  int         Write(const string& content);
  ssize_t     Read(string* content);
  

  void        Close();


 protected:
    
 private:
  // Config Section
  SocketMode    sockMode_;
  const
  SocketFamily  sockFamily_;
  const
  SocketType    sockType_;

  string        sockName_; // For Local Socket.
  string        portNumber_;
  string        destIpAddr_;

  static
  uint8_t       numMaxRetry_;
  static
  uint16_t      retryInterval_;
  
  // Data Section
  SocketStatus  sockStatus_; 
  int           socketFd_;
  // #MAYBE: Add Time Stamp to report how long the socket was open for.


  // Function Section
  bool          bindSocket();
  bool          localBindSocket();
  inline
  bool          listenSocket();

  bool          connectSocket();
  bool          localConnectSocket();
  
  //void          closeSocket();
};


}
#endif
