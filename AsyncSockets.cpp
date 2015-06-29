#include "AsyncSockets.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
AsyncSockets::Exception::exceptionMessages_[] = {
  ASYNCSOCKETS_EXCEPTION_MESSAGES
};
#undef ASYNCSOCKETS_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


AsyncSockets::Exception::Exception(ExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
AsyncSockets::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const AsyncSockets::ExceptionType
AsyncSockets::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====

const int AsyncSockets::defaultNumEventMax_ = 100;

// SERVER MODE

AsyncSockets::AsyncSockets() :
  numMaxEvent_(this->defaultNumEventMax_),
  epollFd_(0),
  isSetToStop_(false),
  events_(nullptr),
  status_(Status::INIT)
{
  DEBUG_FUNC_START;

  this->epollFd_ = this->createEpoll();

  this->events_ = (epoll_event *) calloc (this->numMaxEvent_, sizeof (epoll_event));
}



AsyncSockets::~AsyncSockets() {
  DEBUG_FUNC_START;

  for (auto& socket : this->domainSockets_) {
    socket.second.first->Close();
    delete socket.second.first;
  } 
  this->domainSockets_.clear();
  for (auto& socket : this->networkSockets_) {
    socket.second.first->Close();
    delete socket.second.first;
  } 
  this->networkSockets_.clear();

  delete events_;
}

void AsyncSockets::Close(uint16_t portNumber) {
  DEBUG_FUNC_START;
  // CLOSE SOCKET
  if (this->status_ == Status::OPEN ||
      this->status_ == Status::CLOSING)
  {
    auto it = this->networkSockets_.find(portNumber);
    if (it == this->networkSockets_.end()) {
      DEBUG_cerr << "Cannot close socket. It doesn't exist. PortNumber: " << portNumber << endl; 
      return;
    } 
    Socket* socket = it->second.first;
    socket->Close();
    delete socket;
    this->networkSockets_.erase(it);
    DEBUG_cout << "AsyncSockets is now stopped and socket is closed." << endl; 
  } 

  if (this->networkSockets_.size() + this->domainSockets_.size() == 0) {
    DEBUG_cout << "All Sockets are closed" << endl; 
    this->status_ = Status::CLOSED;
  } 
}

void AsyncSockets::Close(const string& sockName) {
  DEBUG_FUNC_START;
  // CLOSE SOCKET
  if (this->status_ == Status::OPEN ||
      this->status_ == Status::CLOSING)
  {
    auto it = this->domainSockets_.find(sockName);
    if (it == this->domainSockets_.end()) {
      DEBUG_cerr << "Cannot close socket. It doesn't exist. SockName: " << sockName << endl; 
      return;
    } 
    Socket* socket = it->second.first;
    socket->Close();
    delete socket;
    this->domainSockets_.erase(it);
    DEBUG_cout << "AsyncSockets is now stopped and socket is closed." << endl; 
  } 

  if (this->networkSockets_.size() + this->domainSockets_.size() == 0) {
    DEBUG_cout << "All Sockets are closed" << endl; 
    this->status_ = Status::CLOSED;
  } 
}


Socket* AsyncSockets::AddSocket(const string& sockName,
    const Socket::SocketFamily sockFamily,
    const Socket::SocketType sockType) {
  DEBUG_FUNC_START;
  auto sockmapitr = this->domainSockets_.find(sockName);
  if (sockmapitr != this->domainSockets_.end()) {
    DEBUG_cerr << "Socket is already added. SocketName: " << sockName << endl; 
    return nullptr;
  } 

  Socket* socket = new Socket(sockFamily, sockType);
  Socket::SocketMode mode = Socket::SocketMode::UNDEF;

  std::pair<Socket*, Socket::SocketMode> newSocket(socket, mode);

  this->domainSockets_[sockName] = newSocket;

  return socket;
}

Socket* AsyncSockets::AddSocket(const uint16_t portNumber,
    const Socket::SocketFamily sockFamily,
    const Socket::SocketType sockType) {

  DEBUG_FUNC_START;
  DEBUG_cout << "  PortNumber: " << portNumber << endl; 
  auto sockmapitr = this->networkSockets_.find(portNumber);
  if (sockmapitr != this->networkSockets_.end()) {
    DEBUG_cerr << "Socket is already added. PortNumber: " << portNumber << endl; 
    return nullptr;
  } 

  Socket* socket = new Socket(sockFamily, sockType);
  Socket::SocketMode mode = Socket::SocketMode::UNDEF;

  std::pair<Socket*, Socket::SocketMode> newSocket(socket, mode);

  this->networkSockets_[portNumber] = newSocket;

  return socket;
}

int AsyncSockets::GetEpollFd() const {
  return this->epollFd_;
}

void AsyncSockets::SetNumMaxEvent(const int maxEvent) {
  if (maxEvent <= 0) {
    DEBUG_cerr << "MaxEvent cannot be equal to or less than 0." << endl; 
    return;
  } 
  this->numMaxEvent_ = maxEvent;
}

int AsyncSockets::GetSocketFd(const uint16_t portNumber) const {
  DEBUG_FUNC_START;
  auto sockmapitr = this->networkSockets_.find(portNumber);
  if (sockmapitr == this->networkSockets_.end()) {
    DEBUG_cerr << "Socket Not Found. PortNumber: " << portNumber << endl; 
    return -1;
  } 

  Socket* socket = sockmapitr->second.first;

  return socket->GetSocketFd();
}

int AsyncSockets::GetSocketFd(const string& socketName) const {
  DEBUG_FUNC_START;
  auto sockmapitr = this->domainSockets_.find(socketName);
  if (sockmapitr == this->domainSockets_.end()) {
    DEBUG_cerr << "Socket Not Found. SocketName: " << socketName << endl; 
    return -1;
  } 

  Socket* socket = sockmapitr->second.first;
  return socket->GetSocketFd();
}

void AsyncSockets::Listen(const string& socketName) {
  DEBUG_FUNC_START;
  auto sockmapitr = this->domainSockets_.find(socketName);
  if (sockmapitr == this->domainSockets_.end()) {
    DEBUG_cerr << "Socket Not Found. SocketName: " << socketName << endl; 
    return;
  } 

  Socket* socket = sockmapitr->second.first;
  assert (socket->GetSocketFamily() == Socket::SocketFamily::LOCAL &&
          "SocketFamily must be LOCAL");
  
  Socket::SocketMode& sockMode = sockmapitr->second.second;
  sockMode = Socket::SocketMode::LISTEN;

  bool result = socket->Listen(socketName);
  if (result == false) {
    std::cerr << "Could not initialize Socket!" << endl;
    return;
  }
  
  const int socketFd = socket->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::OPEN_READY;
}

#if 0
void AsyncSockets::Connect (const string& sockName) {
  DEBUG_FUNC_START;
  assert (this->socket_->GetSocketFamily() == Socket::SocketFamily::LOCAL &&
          "SocketFamily must be LOCAL");
  
  this->mode_ = Socket::SocketMode::CONNECT;
  
  bool result = false;
  result = this->socket_->Connect(sockName);

  if (result == false) {
    throw exception();
    return;
  }
  this->status_ = Status::CONNECTED;
  return;
}
#endif

void AsyncSockets::Listen(const uint16_t portNumber) {
  DEBUG_FUNC_START;
  auto sockmapitr = this->networkSockets_.find(portNumber);
  if (sockmapitr == this->networkSockets_.end()) {
    DEBUG_cerr << "Socket Not Found. PortNumber: " << portNumber << endl; 
    return;
  } 

  Socket* socket = sockmapitr->second.first;
  assert (socket->GetSocketFamily() == Socket::SocketFamily::IP &&
          "SocketFamily must be IP");

  Socket::SocketMode& sockMode = sockmapitr->second.second;
  sockMode = Socket::SocketMode::LISTEN;

  bool result = socket->Listen(portNumber);
  if (result == false) {
    std::cerr << "Could not initialize Socket!" << endl;
    return;
  }
  
  const int socketFd = socket->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::OPEN_READY;
  //this->waitForEvent();
}


void AsyncSockets::Wait() {
  if (this->status_ == Status::OPEN_READY) {
    this->status_ = Status::OPEN;
    this->waitForEvent();
  } else {
    DEBUG_cerr << "Status is not OPEN_READY. Cannot start WAITING for an event." << endl; 
  }
}

#if 0

void AsyncSockets::Connect (const uint16_t portNumber, const string& destIpAddr) {
  DEBUG_FUNC_START;
  assert (this->socket_->GetSocketFamily() != Socket::SocketFamily::LOCAL &&
          "SocketFamily must be NON-LOCAL");
  
  this->mode_ = Socket::SocketMode::CONNECT;

  bool result = false;
  result = this->socket_->Connect(portNumber, destIpAddr);
  
  if (result == false) {
    throw Exception(ExceptionType::CONNECT_ERROR);
    return;
  }

  this->status_ = Status::CONNECTED;
  return;
}

int AsyncSockets::Write(const void* dataLocation, size_t length) {
  if (this->status_ == Status::LISTENING ||
      this->status_ == Status::CONNECTED) {
    return this->socket_->Write(dataLocation, length);
  } else {
    DEBUG_cerr << "Tried to write to socket that is not listening nor connected" << endl; 
  }
  return -1;
}

int AsyncSockets::Write(const string& content) {
  //assert(this->mode_ == Socket::SocketMode::CONNECT && "Write is supported only for CONNECT");
  // #TODO: Check for max length it can write.
  
  if (this->status_ == Status::LISTENING ||
      this->status_ == Status::CONNECTED) {
    return this->socket_->Write(content);
  } else {
    DEBUG_cerr << "Tried to write to socket that is not listening nor connected" << endl; 
  }
  return -1;
}

ssize_t AsyncSockets::Read(string* content) {
  assert(this->mode_ == Socket::SocketMode::CONNECT && "Read is supported only for CONNECT");

  ssize_t totalBytesRead = 0;
  char readBuf[1024*8];
  ssize_t numBytesRead = 0;
  
  while (true) {
    memset(readBuf, 0, sizeof(readBuf));
    numBytesRead = read (this->GetSocketFd(), readBuf, sizeof(readBuf));
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
#endif




void AsyncSockets::waitForEvent(ssize_t epollTimeout) {
  // epollTimeout is in milliseconds. -1 means no timeout, 0 means return immediately.
  // #TEST: instead of class member, using local var.
  
  //int socketFd = this->GetSocketFd();
  
  while (true) {
    if (this->status_ != Status::OPEN) {
      break;
    } 
    ssize_t numEvents = 0;
    DEBUG_cout << "Waiting..." << endl;
    numEvents = epoll_wait (this->epollFd_, this->events_, this->numMaxEvent_, epollTimeout);
    DEBUG_cout << "  Event Triggered! " << "numEvents: " << numEvents << endl;
    for (ssize_t i = 0; numEvents > i; ++i) {
      if ((this->events_[i].events & EPOLLIN) ||
          (this->events_[i].events & EPOLLOUT))
      {
        FdEventArgs::EventType eventType;

        if (this->events_[i].events & EPOLLIN) {
          eventType = FdEventArgs::EventType::EPOLLIN;
        } else {
          eventType = FdEventArgs::EventType::EPOLLOUT;
        }

        this->OnFdEvent(FdEventArgs(this->events_[i].data.fd, eventType));

      } else if ( (this->events_[i].events & EPOLLERR) ||
                  (this->events_[i].events & EPOLLHUP) )
      {
        // #TODO: LOG ERROR? Research which case it is
        DEBUG_cerr << "EPOLLERR & EPOLLHUP Error!!" << endl;
        close (this->events_[i].data.fd);
        continue;

      } else {
        DEBUG_cerr << "EPOLL Error! Else...?" << endl;
        close (this->events_[i].data.fd);
        continue;
      }
    }
  }
}

int AsyncSockets::createEpoll() {
  int epollFd = epoll_create1(0);
  if (epollFd == consts::ERROR) {
    DEBUG_cerr << "Failed to create epollfd." << endl; 
    throw Exception (ExceptionType::EPOLL_ERROR);
  }
  DEBUG_cout << "Epoll has been created. EpollFd: " << epollFd << endl; 
  return epollFd;
}    


bool AsyncSockets::addFdToEpoll (const int epollFd, const int targetFd, const uint32_t events) {
  if (epollFd < 0) {
    DEBUG_cerr << "Epoll is not initialized." << endl; 
    throw Exception(ExceptionType::EPOLL_ERROR);
  } 
  if (targetFd < 0) {
    DEBUG_cerr << "Adding invalid fd to epoll. Ignored." << endl; 
    return false;
  } 
  
  const int ERROR = -1;
  
  struct epoll_event event;
  
  event.data.fd = targetFd;
  event.events = EPOLLIN | events | EPOLLET;
  
  int result = ERROR;
  result = epoll_ctl (epollFd, EPOLL_CTL_ADD, targetFd, &event);
  if (result == ERROR) {
    DEBUG_cerr << "controlEpoll has failed. errno: " << errno <<
      " epollFd: " << epollFd << " targetFd: " << targetFd << endl; 
    throw Exception(ExceptionType::EPOLL_ERROR);
    return false;
  }

  return true;
}


void AsyncSockets::removeFdFromEpoll (const int fd) {
  assert(!"NOT YET IMPLEMENTED");
}

void AsyncSockets::setNonBlocking (const int fd) {
  if ( fd < 0 ) {
    DEBUG_cerr << "Descriptor cannot be less than 0." << endl; 
    throw Exception(ExceptionType::SOCKET_ERROR); 
  }

  int flags = fcntl (fd, F_GETFL, 0);
  if (flags == consts::ERROR) {
    DEBUG_cerr << "Failed to make it nonblock fd. Invalid fd." << endl; 
    throw Exception(ExceptionType::NON_BLOCK_ERROR); 
  }

  flags |= O_NONBLOCK;
  
  int result = fcntl (fd, F_SETFL, flags);
  if (result == consts::ERROR) {
    DEBUG_cerr << "Failed to make it nonblock fd. Set flag failed." << endl; 
    throw Exception(ExceptionType::NON_BLOCK_ERROR); 
  }
}


}

#if _UNIT_TEST


#include <iostream>
#include <unistd.h>

using namespace lio;


int main() {
  const string sockName = "./.testSock";

  pid_t id = fork();
  if (id == 0) {
    // child (client)
    sleep(1);
    DEBUG_cout << "Child Start!" << endl; 
    AsyncSockets* asock = new AsyncSockets(Socket::SocketFamily::LOCAL);
    asock->Connect(sockName);

    /*
    string userInput;
    while (true) {
      std::getline(std::cin, userInput);

      if (userInput == "q") {
        break;
      }
      
      asock->Write(userInput);
    }
    */
    

    string data = "12345678901234567890123456789012345678901234567890";

    for (int j = 0; 1000 > j; j++) {
      asock->Write(data.c_str(), 50);
      asock->Write(data.c_str(), 50);
      
    } 

    
  } else if (id > 0) {
    // parent (server)
    AsyncSockets* asock = new AsyncSockets(Socket::SocketFamily::LOCAL);
    asock->Listen(sockName);
  }



  return 0;
}

#endif
#undef _UNIT_TEST


