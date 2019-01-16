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
    Sep 29, 2015

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

#include "liolib/Debug.hpp"
#include "liolib/Log.hpp"
#include "liolib/Test.hpp"

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

#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef UNIX_PATH_MAX
  #define UNIX_PATH_MAX 108
#endif

#include <unistd.h> // close()
#include <fcntl.h> // fcntl()

#include "liolib/Util.hpp" // String::ToUInt()

namespace lio {

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

enum class SocketMode : std::uint8_t {
  UNDEF,
  LISTEN,
  CONNECT
};
// Implement SocketStatus Later when it's really needed.
enum class SocketStatus : std::uint8_t {
  INIT,
  OPENING,
  LISTENING,
  CONNECTED,
  CLOSING,
  CLOSED,
  ERROR,
};
// Const Reference
//  /usr/include/bits/socket.h
enum class SocketFamily {
  UNDEF = -1,
  LOCAL = AF_UNIX, // AF_LOCAL
  IP = AF_UNSPEC, // Automatically decides between IPv4 and IPv6
  IPv4 = AF_INET,
  IPv6 = AF_INET6
};

enum class SocketType {
  UNDEF = -1,
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM
};


  class IPv4Connection {
  public:
    IPv4Connection() : address(), serverPortNumber(0), isSecure(false) {
      memset(&this->address, 0, sizeof(this->address));
    }

    IPv4Connection(const struct sockaddr_in& addr, uint16_t serverPortNumber,
               bool isSecure = false)
        : address(addr), serverPortNumber(serverPortNumber),
          isSecure(isSecure) {
      //memcpy(&this->address, &addr, sizeof(addr));
    }

    SocketFamily GetAddressFamily() const {
      switch (this->address.sin_family) {
        case AF_UNIX:
          return SocketFamily::LOCAL;
        case AF_INET:
          return SocketFamily::IPv4;
        case AF_INET6:
          return SocketFamily::IPv6;
      } 
      return SocketFamily::UNDEF;
    }

    uint32_t GetIpAddressInt() const {
      return this->address.sin_addr.s_addr;
    }

    std::string GetIpAddressStr() const {
      return std::string(inet_ntoa(this->address.sin_addr));
    }

    uint16_t GetClientPortNumber() const {
      return ntohs(this->address.sin_port);
    }

    uint16_t GetServerPortNumber() const {
      return this->serverPortNumber;
    }

    bool IsSecure() const {
      return this->isSecure;
    }

  private:
    struct sockaddr_in address;
    uint16_t serverPortNumber;
    bool isSecure;
  };

class Socket {
 public:

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
  bool          Listen (const std::string& sockName); // socket path
  bool          Connect (const std::string& sockName, int retryInterval = 3); // socket path
  // IP Listen and Connect
  bool          Listen (const uint16_t portNumber);
  bool          Connect (const uint16_t portNumber, const std::string& destIpAddr);

  void          Close();

  // DEPRECATED FUNCS
  int           Write(const void* dataLocation, const size_t length);
  int           Write(const std::string& content);
  ssize_t       Read(std::string* content);


private:
  Result        bindSocket();
  Result        localBindSocket();

  Result        listenSocket();

  Result        connectSocket();
  Result        localConnectSocket(); // result 1: successful, 0: retry, -1: stop.


 protected:
    
 private:
  SocketStatus  sockStatus_; 

  SocketMode    sockMode_;
  const
  SocketFamily  sockFamily_;
  const
  SocketType    sockType_;

  std::string   sockName_; // For Local Socket.
  std::string   portNumber_;
  std::string   destIpAddr_;
  
  int           socketFd_;

  // #MAYBE: Add Time Stamp to report how long the socket was open for.
};


}
#endif
