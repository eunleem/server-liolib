#include "Socket.hpp"


#define _UNIT_TEST false
#include "liolib/Test.hpp"
  
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


uint8_t Socket::numMaxRetry_ = 10;
uint16_t Socket::retryInterval_ = 10; //seconds


Socket::Socket (SocketFamily sockFamily, SocketType sockType)
  : sockFamily_(sockFamily),
    sockType_(sockType),
    sockStatus_(SocketStatus::INIT),
    socketFd_(0)
{
  DEBUG_FUNC_START;
}

Socket::Socket (const int existingFd,
                SocketMode mode,
                SocketFamily sockFamily, SocketType sockType)
  : sockMode_(mode),
    sockFamily_(sockFamily),
    sockType_(sockType),
    sockStatus_(SocketStatus::INIT),
    socketFd_(existingFd)
{
  DEBUG_FUNC_START;
  //socketFd = 0; // Initial Value. If it's 0, it means it's not initialized.
  switch (mode) {
   case SocketMode::LISTEN:
    this->sockStatus_ = SocketStatus::LISTENING;
    break;
   case SocketMode::CONNECT:
    this->sockStatus_ = SocketStatus::CONNECTED;
    break;
   default:
    throw SocketException(SocketExceptionType::INIT_FAILED);
    break;
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

    if (this->socketFd_ > 0) {
      close(this->socketFd_);
    }
  } else {
    // #TODO: LOG instead of show message.
    DEBUG_clog << "Tried to close socket that is not open.\n";
  }
  this->sockStatus_ = SocketStatus::CLOSED;
}



bool Socket::Listen(const string& sockName) {
  //DEBUG_cout << "socketFd: " << this->socketFd << endl;
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");
  assert (this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert (sockName.empty() == false && "sockName cannot be empty.");

  this->sockName_ = sockName;

  bool result = false;
  int retryCount = 0;
  
  while (result == false && this->numMaxRetry_ > retryCount) {
    ++retryCount;

    result = this->localBindSocket() && this->listenSocket();
    if (result) {
      DEBUG_cout << "  Bind, Listen Successful." << endl;
      DEBUG_cout << "  SocketFd: " << this->socketFd_ << endl;
      break;
    }

    cout << "Trying again after " << static_cast<int>(this->retryInterval_) << " seconds... " <<
      "(" << retryCount << "/" << static_cast<int>(this->numMaxRetry_) << ")" << endl;
    sleep( static_cast<int> (this->retryInterval_));
  }

  if (result == true) {
    this->sockStatus_ = SocketStatus::LISTENING;
  } else {
    DEBUG_cerr << "  Bind, Listen Failed." << endl;
  }

  return result;
}

bool Socket::Connect(const string& sockName, int retryInterval) {
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");
  assert (this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert (this->sockType_ == SocketType::TCP && "UDP not supported for Connect().");
  assert (sockName.empty() == false && "sockName cannot be empty.");

  this->sockName_ = sockName;

  DEBUG_cout << "Local Socket Name: " << sockName << endl;
  
  bool result = false;
  uint8_t retryCount = 0;


  while (result == false && this->numMaxRetry_ > retryCount) {
    retryCount += 1;
    try {
      result = this->localConnectSocket ();
    } catch (SocketException& ex) {
      DEBUG_cerr << "SocketError: " << ex.what() << endl;
      if (ex.type() != SocketExceptionType::CONNECT_FAIL) {
        throw SocketException(ex.type());
      }
    }
    if (result == false && this->numMaxRetry_ > retryCount) {
      cout << "Trying again after " << retryInterval << " seconds... " <<
          "(" << static_cast<int>(retryCount) << "/" <<
          static_cast<int>(this->numMaxRetry_) << ")" << endl;
      sleep(retryInterval);
    }
  }
  
  if (result == true) {
    this->sockStatus_ = SocketStatus::CONNECTED;
  } else {
    DEBUG_cerr << "Failed to connect to Domain Socket named " << this->sockName_ << endl;
  }
  return result;
}

bool Socket::Listen(const uint16_t portNumber) {
  //DEBUG_cout << "socketFd: " << this->socketFd << endl;
  assert (this->sockFamily_ != SocketFamily::LOCAL &&
         "SocketFamily must be NON-LOCAL");
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");


  this->portNumber_ = std::to_string(portNumber);

  bool result = false;
  int retryCount = 0;
  
  while (result == false && this->numMaxRetry_ > retryCount) {
    ++retryCount;

    result = this->bindSocket() && this->listenSocket();
    if (result) {
      DEBUG_cout << "  Bind, Listen Successful. SocketFd: " << this->socketFd_ << endl;
      break;
    }

    DEVEL {
      DEVEL cout << "*** DEV MODE! Increasing Port Number!\n";
      this->portNumber_ = std::to_string(Util::String::ToUInt(this->portNumber_) + 1);
      DEVEL cout << "*** New Port Number: " << this->portNumber_ << endl;
      continue;
    }

    cout << "Trying again after " << this->retryInterval_ << " seconds... " <<
      "(" << retryCount << "/" << static_cast<int>(this->numMaxRetry_) << ")" << endl;
    sleep( static_cast<int> (this->retryInterval_));
  }

  if (result == true) {
    this->sockStatus_ = SocketStatus::LISTENING;
  } else {
    DEBUG_cerr << "  Bind, Listen Failed. Port: " << this->portNumber_ << endl;
    this->sockStatus_ = SocketStatus::ERROR;
  }

  return result;
}

bool Socket::Connect(const uint16_t portNumber, const string& destIpAddr) {
  assert (this->socketFd_ == 0 && "This Socket instance is already initialized.");
  assert (this->sockFamily_ != SocketFamily::LOCAL && "Socket Family must be NON-LOCAL");
  assert (this->sockType_ == SocketType::TCP && "UDP not supported for Connect().");

  this->portNumber_ = std::to_string(portNumber);
  this->destIpAddr_ = destIpAddr;
  
  bool result = 0;
  uint8_t retryCount = 0;
  
  if (this->destIpAddr_.empty()) {
    throw SocketException(SocketExceptionType::CONNECT_FAIL);
    return false;
  }
  
  while (result == false && this->numMaxRetry_ > retryCount) {
    ++retryCount;

    result = this->connectSocket ();
    if(result == false && this->numMaxRetry_ > retryCount) {
      cout << "Trying again after " << static_cast<int>(this->retryInterval_) << " seconds... " <<
        "(" << retryCount << "/" << static_cast<int>(this->numMaxRetry_) << ")" << endl;
      sleep( static_cast<int> (this->retryInterval_));
    }
  }

  if (result == true) {
    this->sockStatus_ = SocketStatus::CONNECTED;
  } else {
    DEBUG_cerr << "Failed to connect to " << this->destIpAddr_ << ":" << this->portNumber_ << endl;
    this->sockStatus_ = SocketStatus::ERROR;
  }
  
  return result;
}


void Socket::ReportStatus() const {
  cout << "=== Status Report ===\n";


  cout << "SocketFd: " << this->GetSocketFd() << endl;

  cout << "Socket Family: ";
  switch (this->sockFamily_) {
    case SocketFamily::LOCAL:
      cout << "LOCAL\n";
      break;
    case SocketFamily::IP:
      cout << "IP Auto\n";
      break;
    case SocketFamily::IPv4:
      cout << "IPv4\n";
      break;
    case SocketFamily::IPv6:
      cout << "IPv6\n";
      break;
    default:
      cout << "Unknown (Something wrong!)\n";
      break;
  }

  cout << "Socket Type: ";
  switch (this->sockType_) {
   case SocketType::TCP:
    cout << "TCP\n";
    break;
   case SocketType::UDP:
    cout << "UDP\n";
    break;
   default:
    cout << "Unknown (Something wrong!)\n";
    break;
  }

  cout << "Port: " << this->portNumber_ << endl;
  cout << "DestIpAddr: " << this->destIpAddr_ << endl << endl;

  cout << "=== End of Status Report ===\n";
}


int Socket::Write(const void* dataLocation, size_t length) {
  assert(this->sockStatus_ == SocketStatus::CONNECTED && "Socket Status must be connected.");

  int numBytesWritten = -1;
  numBytesWritten = write (this->socketFd_, dataLocation, length);
  if (numBytesWritten == -1) {
    throw SocketException (SocketExceptionType::WRITE_FAIL);
    return -1;
  }

  return numBytesWritten;
}

int Socket::Write(const string& content) {
  //assert(this->sockStatus_ == SocketStatus::CONNECTED && "Socket Status must be connected.");

  int numBytesWritten = -1;
  numBytesWritten = write (this->socketFd_, content.c_str(), content.length());
  if (numBytesWritten == -1) {
    throw SocketException (SocketExceptionType::WRITE_FAIL);
    return -1;
  }

  return numBytesWritten;
}

ssize_t Socket::Read(string* content) {
  assert(this->sockStatus_ == SocketStatus::CONNECTED && "Socket Status must be connected.");
  ssize_t totalBytesRead = 0;

  char readBuf[1024*8];

  ssize_t numBytesRead = 0;
  
  while (true) {
    memset(readBuf, 0, sizeof(readBuf));
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
      DEBUG_cerr << "Read Error from Socket. readCount: " << numBytesRead << endl;
      //DEBUG_cout << "ReadContent: " << string(readBuf) << endl; 
      return -1;
    }
  }
  
  return totalBytesRead;
}


int Socket::GetSocketFd() const {
  if (this->socketFd_ <= 0) {
    //#think: assert vs throw here?
    throw SocketException (SocketExceptionType::NOT_INITIALIZED);
  }
  return this->socketFd_;
}

Socket::SocketFamily Socket::GetSocketFamily() const {
  return this->sockFamily_;
}

Socket::SocketType Socket::GetSocketType() const {
  return this->sockType_;
}

Socket::SocketMode Socket::GetSocketMode() const {
  return this->sockMode_;
}


// ************************* PRIVATE Functions *************************

bool Socket::bindSocket() {
  assert(this->sockFamily_ != SocketFamily::LOCAL &&
         "SocketFamily must be NON-LOCAL");

  struct addrinfo addrHints;
  memset (&addrHints, 0, sizeof (struct addrinfo));

  addrHints.ai_family = static_cast<int> (this->sockFamily_);
  addrHints.ai_socktype = static_cast<int> (this->sockType_);
  addrHints.ai_flags = AI_PASSIVE; // All Interfaces. Automatically sets IP Address
  

  int result = ERROR; 
  struct addrinfo *addrResult;

  result = getaddrinfo (NULL, this->portNumber_.c_str(), &addrHints, &addrResult);
  
  if (result == ERROR) {
    DEBUG_cout << "getaddrinfo: " << gai_strerror(result) << endl;
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror (s));
    return false;
  }

  int newSocketFd = ERROR;

  while (addrResult != NULL) {
    newSocketFd = socket (addrResult->ai_family,
                          addrResult->ai_socktype,
                          addrResult->ai_protocol);
    if (newSocketFd == ERROR)
      continue;
    
    result = bind (newSocketFd, addrResult->ai_addr, addrResult->ai_addrlen);

    if (result == OK) {
      // Bind or connect Successful.
      DEBUG_cout << "Bind Successful." << endl;
      break;
    }
    close (newSocketFd);
    addrResult = addrResult->ai_next;
  }


  if (addrResult == NULL) {
    DEBUG_cerr << "Could not bind to a socket." << endl;
    return false;
  }

  this->socketFd_ = newSocketFd;
  freeaddrinfo (addrResult);
  return true; 
}

bool Socket::localBindSocket() {
  assert(this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert(this->sockName_.empty() == false && "SocketName is required.");

  const int SOCKET_FAMILY_LOCAL = static_cast<int>(this->sockFamily_);

  int newSocketFd = socket (SOCKET_FAMILY_LOCAL,
                            static_cast<int>(this->sockType_), 0);
  if (newSocketFd < 0) {
    cerr << "Errno: " << static_cast<int>(errno) << endl;
    throw SocketException(SocketExceptionType::SOCKET_FAIL);
    return false;
  }


  struct sockaddr_un address;

  memset(&address, 0, sizeof(struct sockaddr_un));

  unlink(this->sockName_.c_str());

  address.sun_family = SOCKET_FAMILY_LOCAL;
  this->sockName_.copy (address.sun_path, UNIX_PATH_MAX);
  
  
  int result = ERROR;
  result = bind (newSocketFd,
                 (struct sockaddr *) &address,
                 sizeof(struct sockaddr_un));

  if (result != OK) {
    cerr << "Errno: " << static_cast<int>(errno) << endl;

    throw SocketException(SocketExceptionType::BIND_FAIL);
    return false;
  }

  this->socketFd_ = newSocketFd;

  return true;
}

inline
bool Socket::listenSocket() {

  int result = ERROR;
  
  if (this->sockType_ != SocketType::TCP) {
    //Error. Listen() is only used for SOCK_STREAM and SOCK_SEQPACKET socket type.
    DEBUG_cerr << "Listen Failed." << endl;
    return false;
  }
  
  result = listen (this->socketFd_, SOMAXCONN);
  if (result == ERROR) {
    //ERROR 
    DEBUG_cerr << "Listen Failed. Errno: " << errno << endl;
    return false;
  }
  DEBUG_cout << "Listen Successful." << endl;

  this->sockMode_ = SocketMode::LISTEN;
  return true;
}


bool Socket::localConnectSocket() {
  assert(this->sockFamily_ == SocketFamily::LOCAL && "Must be Local Socket");
  assert(this->sockName_.empty() == false && "SocketName is required.");

  const int SOCKET_FAMILY_LOCAL = static_cast<int>(this->sockFamily_);

  struct sockaddr_un address;

  memset (&address, 0, sizeof (struct sockaddr_un));
  
  const int ERROR = -1;
  int newSocketFd = ERROR;

  newSocketFd = socket (SOCKET_FAMILY_LOCAL,
                        static_cast<int>(this->sockType_), 0);
  if (newSocketFd == ERROR) {
    DEBUG_cerr << "  Socket::localConnect Error. Errno: " << errno << endl;
    throw SocketException(SocketExceptionType::SOCKET_FAIL);
    return false;
  }

  //DEBUG_cout << "  Connect SocketFd: " << newSocketFd << endl;

  this->sockName_.copy (address.sun_path, UNIX_PATH_MAX);
  address.sun_family = SOCKET_FAMILY_LOCAL;


  int result = ERROR;
  result = connect (newSocketFd,
                    (struct sockaddr *) &address,
                    sizeof(struct sockaddr_un));
  if (result == ERROR) {
    DEBUG_cerr << "  Errno: " << static_cast<int>(errno) << ". " << strerror(errno) << endl;
    throw SocketException (SocketExceptionType::CONNECT_FAIL);
    return false;
  }

  this->socketFd_ = newSocketFd;
  this->sockMode_ = SocketMode::CONNECT;
  return true;
}

// #TODO: Add Throw maybe?
bool Socket::connectSocket() {
  assert(this->sockFamily_ != SocketFamily::LOCAL && "Must not be Local Socket");

  struct addrinfo addrHints;
  memset (&addrHints, 0, sizeof (struct addrinfo));

  addrHints.ai_family = AF_UNSPEC; // Automatically decides between IPv4 an IPv6
  addrHints.ai_socktype = static_cast<int> (this->sockType_);
  addrHints.ai_flags = AI_PASSIVE; // All Interfaces. Automatically sets IP Address
  
  int result = ERROR; 
  struct addrinfo *addrResult = nullptr;

  result = getaddrinfo (this->destIpAddr_.c_str(), this->portNumber_.c_str(),
                        &addrHints, &addrResult);
  if (result != OK) {
    DEBUG_cerr << "Failed to connect to " << this->destIpAddr_ << " ErrorDetail: " << gai_strerror(result) << endl;
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror (s));
    return false;
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
    cerr << "Could not bind or connect." << endl;
    //fprintf (stderr, "Could not bind\n");
    return false;
  }

  freeaddrinfo (addrResult);

  this->socketFd_ = newSocketFd;
  this->sockMode_ = SocketMode::CONNECT;
  return true; 
}

#if _UNIT_TEST
int main () {
  return 0;
}
#endif
#undef _UNIT_TEST


