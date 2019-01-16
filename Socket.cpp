#include "Socket.hpp"

#define _UNIT_TEST false
  
using namespace lio;

// ===== Exception Implementation =====
const char* const
SocketException::exceptionMessages[] = {
   SOCKET_EXCEPTION_MESSAGES
};
#undef SOCKET_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


SocketException::SocketException(SocketExceptionType exceptionType) {
    this->exceptionType = exceptionType;
}

const char*
SocketException::what() const noexcept {
    return this->exceptionMessages[(int) this->exceptionType];
}

const SocketExceptionType
SocketException::type() const noexcept {
    return this->exceptionType;
}
// ===== Exception Implementation End =====


Socket::Socket (SocketFamily sockFamily, SocketType sockType)
  : sockStatus_(SocketStatus::INIT),
    sockMode_(SocketMode::UNDEF),
    sockFamily_(sockFamily),
    sockType_(sockType),
    socketFd_(-1)
{
  DEBUG_FUNC_START;
}

Socket::Socket (const int existingFd,
                SocketMode mode,
                SocketFamily sockFamily, SocketType sockType)
  : sockStatus_(SocketStatus::INIT),
    sockMode_(mode),
    sockFamily_(sockFamily),
    sockType_(sockType),
    socketFd_(existingFd)
{
  DEBUG_FUNC_START;
  //socketFd = 0; // Initial Value. If it's 0, it means it's not initialized.
  if (mode == SocketMode::LISTEN) {
    this->sockStatus_ = SocketStatus::LISTENING;

  } else if (mode == SocketMode::CONNECT) {
    this->sockStatus_ = SocketStatus::CONNECTED;

  } else {
    static_assert(true, "Invalid SocketMode");

  }
}

Socket::~Socket() {
  DEBUG_FUNC_START;
  if (this->socketFd_ > 0) {
    close(this->socketFd_);
  }
}


void Socket::Close() {
  if (this->sockStatus_ == SocketStatus::LISTENING ||
      this->sockStatus_ == SocketStatus::CONNECTED) {
    if (this->socketFd_ >= 0) {
      close(this->socketFd_);
    }

  } else if (this->sockStatus_ == SocketStatus::OPENING) {
    LOG_info << "Close socket is called while opening." << endl;
    
  } else {
    LOG_warn << "Tried to close socket that is not open." << endl;

  }

  this->sockStatus_ = SocketStatus::CLOSED;
  DEBUG_result << "Socket is closed now." << endl;
}



bool Socket::Listen(const std::string& sockName) {
  //DEBUG_cout << "socketFd: " << this->socketFd << endl;
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");
  assert (this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert (sockName.empty() == false && "sockName cannot be empty.");

  this->sockName_ = sockName;

  DEBUG_args << "Listening on Local Socket Name: " << sockName << endl;

  const unsigned int NumRetry = 5, RetryIntervalSecs = 3;

  bool result =
      Util::Retry([this] {
                    bool isSuccessful =
                        (this->localBindSocket() == Result::SUCCESSFUL) &&
                        (this->listenSocket() == Result::SUCCESSFUL);
                    if (isSuccessful == true) {
                      return Result::SUCCESSFUL;
                    }

                    if (this->sockStatus_ == SocketStatus::CLOSING ||
                        this->sockStatus_ == SocketStatus::CLOSED) {
                      return Result::INTERRUPT;
                    }

                    return Result::RETRY;
                  },
                  NumRetry, RetryIntervalSecs);

  if (result == true) {
    DEBUG_subresult << "  Bind, Listen Successful. SocketFd: " << this->socketFd_ << endl;
    this->sockStatus_ = SocketStatus::LISTENING;

  } else {
    LOG_err << "  Bind, Listen Failed." << endl;
  }

  return result;
}


bool Socket::Connect(const std::string& sockName, int retryInterval) {
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");
  assert (this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert (this->sockType_ == SocketType::TCP && "UDP not supported for Connect().");
  assert (sockName.empty() == false && "sockName cannot be empty.");

  this->sockName_ = sockName;


  DEBUG_args << "Connecting to Local Socket Name: " << sockName << endl;

  const unsigned int NumRetry = 5, RetryIntervalSecs = 3;
  bool result = Util::Retry([this] { return this->localConnectSocket(); },
                            NumRetry, RetryIntervalSecs);

  if (result == true) {
    this->sockStatus_ = SocketStatus::CONNECTED;

  } else {
    LOG_err << "Failed to connect to Domain Socket named " << this->sockName_
            << endl;
  }
  return result;
}

bool Socket::Listen(const uint16_t portNumber) {
  //DEBUG_cout << "socketFd: " << this->socketFd << endl;
  assert(this->sockFamily_ != SocketFamily::LOCAL &&
         "SocketFamily must be NON-LOCAL");
  assert(this->socketFd_ == 0 &&
         "This Socket instance is already initialized.");

  this->portNumber_ = std::to_string(portNumber);

  const unsigned int NumRetry = 5, RetryIntervalSecs = 3;
  bool result = Util::Retry([this] {
                              bool isSuccessful =
                                  (this->bindSocket() == Result::SUCCESSFUL) &&
                                  (this->listenSocket() == Result::SUCCESSFUL);
                              if (isSuccessful == true) {
                                return Result::SUCCESSFUL;
                              }

                              if (this->sockStatus_ == SocketStatus::CLOSING ||
                                  this->sockStatus_ == SocketStatus::CLOSED) {
                                return Result::INTERRUPT;
                              }

                              return Result::RETRY;
                            },
                            NumRetry, RetryIntervalSecs);

  if (result == true) {
    DEBUG_result << "  Now Listening on port " << (int) portNumber << endl;
    this->sockStatus_ = SocketStatus::LISTENING;

  } else {
    LOG_err << "  Bind, Listen Failed. Port: " << this->portNumber_ << endl;
    this->sockStatus_ = SocketStatus::ERROR;
  }

  return result;
}

bool Socket::Connect(const uint16_t portNumber, const std::string& destIpAddr) {
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");
  assert (this->sockFamily_ != SocketFamily::LOCAL && "Socket Family must be NON-LOCAL");
  assert (this->sockType_ == SocketType::TCP && "UDP not supported for Connect().");

  this->portNumber_ = std::to_string(portNumber);
  this->destIpAddr_ = destIpAddr;
  
  if (this->destIpAddr_.empty()) {
    throw SocketException(SocketExceptionType::CONNECT_FAIL);
    return false;
  }

  const unsigned int NumRetry = 5, RetryIntervalSecs = 3;
  bool result = Util::Retry([this] { return this->connectSocket(); },
      NumRetry, RetryIntervalSecs);

  if (result == true) {
    this->sockStatus_ = SocketStatus::CONNECTED;

  } else {
    LOG_err << "Failed to connect to " << this->destIpAddr_ << ":" << this->portNumber_ << endl;
    this->sockStatus_ = SocketStatus::ERROR;
  }
  
  return result;
}


void Socket::ReportStatus() const {
  std::cout << "=== Status Report ===\n";


  std::cout << "SocketFd: " << this->GetSocketFd() << endl;

  std::cout << "Socket Family: ";
  switch (this->sockFamily_) {
    case SocketFamily::LOCAL:
      std::cout << "LOCAL\n";
      break;
    case SocketFamily::IP:
      std::cout << "IP Auto\n";
      break;
    case SocketFamily::IPv4:
      std::cout << "IPv4\n";
      break;
    case SocketFamily::IPv6:
      std::cout << "IPv6\n";
      break;
    default:
      std::cout << "Unknown (Something wrong!)\n";
      break;
  }

  std::cout << "Socket Type: ";
  switch (this->sockType_) {
   case SocketType::TCP:
    std::cout << "TCP\n";
    break;
   case SocketType::UDP:
    std::cout << "UDP\n";
    break;
   default:
    std::cout << "Unknown (Something wrong!)\n";
    break;
  }

  std::cout << "Port: " << this->portNumber_ << endl;
  std::cout << "DestIpAddr: " << this->destIpAddr_ << endl << endl;

  std::cout << "=== End of Status Report ===\n";
}


int Socket::Write(const void* dataLocation, size_t length) {
  DEPRECATED_FUNC("unspecified");
  if (this->sockStatus_ != SocketStatus::CONNECTED) {
    LOG_err << "Socket is not connected and tried to write.";
    return -1;
  }

  int numBytesWritten = -1;
  numBytesWritten = write(this->socketFd_, dataLocation, length);
  if (numBytesWritten == -1) {
    LOG_warn << "Write failed. errno: " << errno << endl;
    return -1;
  }

  return numBytesWritten;
}

int Socket::Write(const std::string& content) {
  DEPRECATED_FUNC("unspecified");
  if (this->sockStatus_ != SocketStatus::CONNECTED) {
    LOG_err << "Socket is not connected and tried to write.";
    return -1;
  }

  int numBytesWritten = -1;
  numBytesWritten = write(this->socketFd_, content.c_str(), content.length());
  if (numBytesWritten == -1) {
    LOG_warn << "Write failed. errno: " << errno << endl;
    return -1;
  }

  return numBytesWritten;
}

ssize_t Socket::Read(std::string* content) {
  DEPRECATED_FUNC("unspecified");
  if (this->sockStatus_ != SocketStatus::CONNECTED) {
    LOG_err << "Socket is not connected and tried to read.";
    return -1;
  }

  char readBuf[1024 * 8];

  ssize_t totalBytesRead = 0;
  ssize_t numBytesRead = 0;
  
  while (true) {
    std::memset(readBuf, 0, sizeof(readBuf));
    numBytesRead = read (this->socketFd_, readBuf, sizeof(readBuf));
    if (numBytesRead > 0) {
      totalBytesRead += numBytesRead;
      content->append(readBuf);
    } else if (numBytesRead == -1) {
      if (errno == EINTR) {
        continue;
      } 
      
    } else if (numBytesRead == 0) {
      break;
    } else {
      LOG_err << "Read Error from Socket. readCount: " << numBytesRead << endl;
      //DEBUG_cout << "ReadContent: " << std::string(readBuf) << endl; 
      return -1;
    }
  }
  
  return totalBytesRead;
}


int Socket::GetSocketFd() const {
  if (this->socketFd_ <= 0) {
    throw SocketException (SocketExceptionType::NOT_INITIALIZED);
  }
  return this->socketFd_;
}

SocketFamily Socket::GetSocketFamily() const {
  return this->sockFamily_;
}

SocketType Socket::GetSocketType() const {
  return this->sockType_;
}

SocketMode Socket::GetSocketMode() const {
  return this->sockMode_;
}


// ************************* PRIVATE Functions *************************

Result Socket::bindSocket() {
  assert(this->sockFamily_ != SocketFamily::LOCAL &&
         "SocketFamily must be NON-LOCAL");

  struct addrinfo addrHints;
  std::memset (&addrHints, 0, sizeof (struct addrinfo));

  addrHints.ai_flags = AI_PASSIVE; // All Interfaces. Automatically sets IP Address
  addrHints.ai_family = static_cast<int> (this->sockFamily_);
  addrHints.ai_socktype = static_cast<int> (this->sockType_);
  addrHints.ai_protocol = 0;
  addrHints.ai_canonname = nullptr;
  addrHints.ai_addr = nullptr;
  addrHints.ai_next = nullptr;
  

  int result = ERROR; 
  struct addrinfo *addrResult;

// #REF: http://linux.die.net/man/3/getaddrinfo
  result = getaddrinfo (NULL, this->portNumber_.c_str(), &addrHints, &addrResult);
  if (result == ERROR) {
    LOG_err << "getaddrinfo() Failed. Err: " << gai_strerror(result) << endl;
    return Result::ERROR;
  }

  int newSocketFd = ERROR;

  while (addrResult != NULL) {
    newSocketFd = socket (addrResult->ai_family,
                          addrResult->ai_socktype,
                          addrResult->ai_protocol);
    if (newSocketFd == ERROR)
      continue;

    if (this->sockType_ == SocketType::TCP) {
      const int enable = 1;
      const int setoptResult = setsockopt(newSocketFd, SOL_SOCKET, SO_REUSEADDR,
                                          &enable, sizeof(int));
      if (setoptResult <= -1) {
        LOG_err << "Failed to SetSockOpt" << endl;
        continue;
      }
    }
    
    result = bind (newSocketFd, addrResult->ai_addr, addrResult->ai_addrlen);
    if (result == OK) {
      // Bind or connect Successful.
      DEBUG_result << "Bind Successful." << endl;
      break;
    }
    close (newSocketFd);
    addrResult = addrResult->ai_next;
  }

  if (addrResult == NULL) {
    LOG_err << "Could not bind to a socket. Port: " << this->portNumber_ << endl;
    return Result::ERROR;
  }

  this->socketFd_ = newSocketFd;
  freeaddrinfo (addrResult);
  return Result::SUCCESSFUL;
}

Result Socket::localBindSocket() {
  assert(this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert(this->sockName_.empty() == false && "SocketName is required.");

  const int SOCKET_FAMILY_LOCAL = static_cast<int>(this->sockFamily_);

  int newSocketFd =
      socket(SOCKET_FAMILY_LOCAL, static_cast<int>(this->sockType_), 0);

  if (newSocketFd < 0) {
    LOG_err << "Failed to Open a Local Socket. Errno: " << errno << endl;
    throw SocketException(SocketExceptionType::SOCKET_FAIL);
    return Result::ERROR;
  }

  struct sockaddr_un address;
  std::memset(&address, 0, sizeof(struct sockaddr_un));
  unlink(this->sockName_.c_str());

  address.sun_family = SOCKET_FAMILY_LOCAL;
  this->sockName_.copy (address.sun_path, UNIX_PATH_MAX);
  
  int result = ERROR;
  result = bind (newSocketFd,
                 (struct sockaddr *) &address,
                 sizeof(struct sockaddr_un));

  if (result != OK) {
    LOG_err << "Failed to Bind to Local Socket. Errno: " << errno << endl;
    throw SocketException(SocketExceptionType::BIND_FAIL);
    return Result::ERROR;
  }

  this->socketFd_ = newSocketFd;

  return Result::SUCCESSFUL;
}

Result Socket::listenSocket() {

  int result = ERROR;
  
  if (this->sockType_ != SocketType::TCP) {
    //Error. Listen() is only used for SOCK_STREAM and SOCK_SEQPACKET socket type.
    LOG_err << "Listen Failed." << endl;
    return Result::ERROR;
  }
  
  result = listen (this->socketFd_, SOMAXCONN);
  if (result == ERROR) {
    //ERROR 
    LOG_err << "Listen Failed. Errno: " << errno << endl;
    return Result::ERROR;
  }

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(this->socketFd_, (struct sockaddr *)&sin, &len) == -1) {
    LOG_warn << "getsockname for listening socket has failed. fd: " << this->socketFd_ << endl;
    
  } else {
    LOG_info << "getsockname returned portNumber: " << ntohs(sin.sin_port) << endl;
  }

  LOG_info << "Socket is Now Listening. this->portNumber: " << this->portNumber_ << endl;

  this->sockMode_ = SocketMode::LISTEN;
  return Result::SUCCESSFUL;
}


Result Socket::localConnectSocket() {
  DEBUG_FUNC_START;
  assert(this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert(this->sockName_.empty() == false && "SocketName is required.");

  const int SOCKET_FAMILY_LOCAL = static_cast<int>(this->sockFamily_);

  struct sockaddr_un address;
  std::memset (&address, 0, sizeof (struct sockaddr_un));
  
  const int ERROR = -1;
  int newSocketFd = ERROR;

  newSocketFd = socket (SOCKET_FAMILY_LOCAL,
                        static_cast<int>(this->sockType_), 0);
  if (newSocketFd == ERROR) {
    LOG_err << "  Local Socket Open Failed. Errno: " << errno
            << " ErrMsg: " << strerror(errno) << endl;
    return Result::ERROR;
  }

  //DEBUG_cout << "  Connect SocketFd: " << newSocketFd << endl;

  this->sockName_.copy (address.sun_path, this->sockName_.length());
  address.sun_family = SOCKET_FAMILY_LOCAL;


  int result = ERROR;
  result = connect (newSocketFd,
                    (struct sockaddr *) &address,
                    sizeof(struct sockaddr_un));
  if (result == ERROR) {
    LOG_err << "  Local Connect Failed. Errno: " << static_cast<int>(errno) << ". " << strerror(errno) << endl;
    return Result::ERROR;
  }

  this->socketFd_ = newSocketFd;
  this->sockMode_ = SocketMode::CONNECT;
  return Result::SUCCESSFUL;
}

Result Socket::connectSocket() {
  DEBUG_FUNC_START;
  assert(this->sockFamily_ != SocketFamily::LOCAL && "Must not be Local Socket");
  assert(this->portNumber_.empty() == false && "Port Number is required.");

  struct addrinfo addrHints;
  std::memset (&addrHints, 0, sizeof (struct addrinfo));

  addrHints.ai_family = static_cast<int>(SocketFamily::IP); //AF_UNSPEC. Automatically decides between IPv4 an IPv6
  addrHints.ai_socktype = static_cast<int> (this->sockType_);
  addrHints.ai_flags = AI_PASSIVE; // All Interfaces. Automatically sets IP Address
  
  int result = ERROR; 
  struct addrinfo *addrResult = nullptr;

  result = getaddrinfo (this->destIpAddr_.c_str(), this->portNumber_.c_str(),
                        &addrHints, &addrResult);
  if (result != OK) {
    LOG_err << "Failed to connect to " << this->destIpAddr_
            << " ErrorDetail: " << gai_strerror(result) << endl;
    return Result::ERROR;
  }

  int newSocketFd = ERROR;
  
  while (addrResult != nullptr) {
    
    newSocketFd = socket (addrResult->ai_family,
                          addrResult->ai_socktype,
                          addrResult->ai_protocol);
    if (newSocketFd == ERROR)
      continue;
    
    if (this->sockType_ == SocketType::TCP) {
      result = connect (newSocketFd, addrResult->ai_addr, addrResult->ai_addrlen);
    }

    if (result == OK) {
      // Bind or connect Successful.
      break;
    }
    close (newSocketFd);
    addrResult = addrResult->ai_next;
  }

  if (addrResult == nullptr) {
    LOG_err << "Could not connect to server. portNumber: " << this->portNumber_ << endl;
    return Result::ERROR;
  }

  freeaddrinfo (addrResult);

  this->socketFd_ = newSocketFd;
  this->sockMode_ = SocketMode::CONNECT;
  LOG_info << "Connected to " << this->destIpAddr_ << ":" << this->portNumber_ << "!" << endl;
  return Result::SUCCESSFUL; 
}

#if _UNIT_TEST
int main () {
  return 0;
}
#endif
#undef _UNIT_TEST


