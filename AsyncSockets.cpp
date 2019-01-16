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
      LOG_warn << "Cannot close socket. It doesn't exist. PortNumber: " << portNumber << endl; 
      return;
    } 
    Socket* socket = it->second.first;
    socket->Close();
    delete socket;
    this->networkSockets_.erase(it);
    LOG_info << "AsyncSockets is now stopped and socket is closed." << endl; 
  } else {
    LOG_warn << "Closing Socket that is not open. status: "
             << (int)this->status_ << endl;
    return;
  }

  if (this->networkSockets_.size() + this->domainSockets_.size() == 0) {
    LOG_info << "All Sockets are closed" << endl; 
    this->status_ = Status::CLOSED;
  } 
}

void AsyncSockets::Close(const std::string& sockName) {
  DEBUG_FUNC_START;
  // CLOSE SOCKET
  if (this->status_ == Status::OPEN ||
      this->status_ == Status::CLOSING)
  {
    auto it = this->domainSockets_.find(sockName);
    if (it == this->domainSockets_.end()) {
      LOG_warn << "Cannot close socket. It doesn't exist. SockName: " << sockName << endl; 
      return;
    } 
    Socket* socket = it->second.first;
    socket->Close();
    delete socket;
    this->domainSockets_.erase(it);
    LOG_info << "AsyncSockets is now stopped and socket is closed." << endl; 

  } else {
    LOG_warn << "Closing Socket that is not open. status: "
             << (int)this->status_ << endl;
    return;
  }

  if (this->networkSockets_.size() + this->domainSockets_.size() == 0) {
    LOG_info << "All Sockets are closed" << endl; 
    this->status_ = Status::CLOSED;
  } 
}


Socket* AsyncSockets::AddSocket(const std::string& sockName,
    const SocketFamily sockFamily,
    const SocketType sockType) {
  DEBUG_FUNC_START;
  auto sockmapitr = this->domainSockets_.find(sockName);
  if (sockmapitr != this->domainSockets_.end()) {
    LOG_err << "Socket is already added. SocketName: " << sockName << endl; 
    return nullptr;
  } 

  Socket* socket = new Socket(sockFamily, sockType);
  SocketMode mode = SocketMode::UNDEF;

  std::pair<Socket*, SocketMode> newSocket(socket, mode);

  this->domainSockets_[sockName] = std::move(newSocket);

  return socket;
}

Socket* AsyncSockets::AddSocket(const uint16_t portNumber,
    const SocketFamily sockFamily,
    const SocketType sockType) {

  DEBUG_FUNC_START;
  DEBUG_args << "  PortNumber: " << portNumber << endl; 
  auto sockmapitr = this->networkSockets_.find(portNumber);
  if (sockmapitr != this->networkSockets_.end()) {
    LOG_err << "Socket is already added. PortNumber: " << portNumber << endl; 
    return nullptr;
  } 

  Socket* socket = new Socket(sockFamily, sockType);
  SocketMode mode = SocketMode::UNDEF;

  std::pair<Socket*, SocketMode> newSocket(socket, mode);

  this->networkSockets_[portNumber] = std::move(newSocket);

  return socket;
}

int AsyncSockets::GetEpollFd() const {
  return this->epollFd_;
}

void AsyncSockets::SetNumMaxEvent(const int maxEvent) {
  if (maxEvent <= 0) {
    LOG_err << "MaxEvent cannot be equal to or less than 0." << endl; 
    return;
  } 
  this->numMaxEvent_ = maxEvent;
}

int AsyncSockets::GetSocketFd(const uint16_t portNumber) const {
  DEBUG_FUNC_START;
  auto sockmapitr = this->networkSockets_.find(portNumber);
  if (sockmapitr == this->networkSockets_.end()) {
    LOG_err << "Socket Not Found. PortNumber: " << portNumber << endl; 
    return -1;
  } 

  Socket* socket = sockmapitr->second.first;

  return socket->GetSocketFd();
}

int AsyncSockets::GetSocketFd(const std::string& socketName) const {
  DEBUG_FUNC_START;
  auto sockmapitr = this->domainSockets_.find(socketName);
  if (sockmapitr == this->domainSockets_.end()) {
    LOG_err << "Socket Not Found. SocketName: " << socketName << endl; 
    return -1;
  } 

  Socket* socket = sockmapitr->second.first;
  return socket->GetSocketFd();
}

void AsyncSockets::Listen(const std::string& socketName) {
  DEBUG_FUNC_START;
  auto sockmapitr = this->domainSockets_.find(socketName);
  if (sockmapitr == this->domainSockets_.end()) {
    LOG_err << "Socket Not Found. SocketName: " << socketName << endl; 
    return;
  } 

  Socket* socket = sockmapitr->second.first;
  assert (socket->GetSocketFamily() == SocketFamily::LOCAL &&
          "SocketFamily must be LOCAL");
  
  SocketMode& sockMode = sockmapitr->second.second;
  sockMode = SocketMode::LISTEN;

  bool result = socket->Listen(socketName);
  if (result == false) {
    LOG_err << "Could not Listen on Socket! socketName: " << socketName << endl;
    return;
  }
  
  const int socketFd = socket->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::OPEN_READY;
}

void AsyncSockets::Listen(const uint16_t portNumber) {
  DEBUG_FUNC_START;
  auto sockmapitr = this->networkSockets_.find(portNumber);
  if (sockmapitr == this->networkSockets_.end()) {
    LOG_err << "Socket Not Found. PortNumber: " << portNumber << endl; 
    return;
  } 

  Socket* socket = sockmapitr->second.first;
  assert (socket->GetSocketFamily() == SocketFamily::IP &&
          "SocketFamily must be IP");

  SocketMode& sockMode = sockmapitr->second.second;
  sockMode = SocketMode::LISTEN;

  bool result = socket->Listen(portNumber);
  if (result == false) {
    LOG_err << "Could not Listen on Socket! portNumber: " << portNumber << endl;
    return;
  }
  
  const int socketFd = socket->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::OPEN_READY;
}

void AsyncSockets::ListenAll() {
  DEBUG_cout << "EXPERIMENTAL FUNC ListenAll." << endl;

  for (auto& sock : this->domainSockets_) {
    this->Listen(sock.first);
  }

  for (auto& sock : this->networkSockets_) {
    this->Listen(sock.first);
  }
  
}


void AsyncSockets::Wait() {
  if (this->status_ == Status::OPEN_READY) {
    this->status_ = Status::OPEN;
    this->waitForEvent();
  } else {
    LOG_alert << "Status is not OPEN_READY. Cannot start WAITING for an event." << endl; 
  }
}


void AsyncSockets::waitForEvent(ssize_t epollTimeout) {
  // epollTimeout is in milliseconds. -1 means no timeout, 0 means return immediately.
  // #TEST: instead of class member, using local var.
  
  //int socketFd = this->GetSocketFd();
  
  while (true) {
    if (this->status_ != Status::OPEN) {
      LOG_log << "Status is no longer open. Halting event loop!" << endl;
      break;
    } 
    int numEvents = 0;
    numEvents = epoll_wait (this->epollFd_, this->events_, this->numMaxEvent_, epollTimeout);
    LOG_info << "  Event Triggered! numEvents: " << numEvents << endl;
    if (numEvents == -1) {
      LOG_warn << "epoll_wait returned -1. errno: " << errno
               << " errmsg: " << strerror(errno) << endl;
      continue;
    }
    for (int i = 0; numEvents > i; ++i) {
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
        LOG_err << "EPOLLERR & EPOLLHUP Error!!" << endl;
        close (this->events_[i].data.fd);
        continue;

      } else {
        LOG_err << "EPOLL Error! Else...?" << endl;
        close (this->events_[i].data.fd);
        continue;
      }
    }
  }
}

int AsyncSockets::createEpoll() {
  int epollFd = epoll_create1(0);
  if (epollFd == consts::ERROR) {
    LOG_fatal << "Failed to create epollfd." << endl; 
    throw Exception (ExceptionType::EPOLL_ERROR);
  }
  LOG_info << "Epoll has been created. EpollFd: " << epollFd << endl; 
  return epollFd;
}    


bool AsyncSockets::addFdToEpoll (const int epollFd, const int targetFd, const uint32_t events) {
  if (epollFd < 0) {
    LOG_err << "Epoll is not initialized." << endl; 
    throw Exception(ExceptionType::EPOLL_ERROR);
  } 
  if (targetFd < 0) {
    LOG_err << "Adding invalid fd to epoll. Ignored." << endl; 
    return false;
  } 
  
  const int ERROR = -1;
  
  struct epoll_event event;
  
  event.data.fd = targetFd;
  event.events = EPOLLIN | events | EPOLLET | EPOLLRDHUP; // Don't forge EPOLLRDHUP!
  
  int result = ERROR;
  result = epoll_ctl (epollFd, EPOLL_CTL_ADD, targetFd, &event);
  if (result == ERROR) {
    LOG_err << "controlEpoll has failed. errno: " << errno
              << " epollFd: " << epollFd << " targetFd: " << targetFd << endl;
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
    LOG_err << "Descriptor cannot be less than 0." << endl; 
    throw Exception(ExceptionType::SOCKET_ERROR); 
  }

  int flags = fcntl (fd, F_GETFL, 0);
  if (flags == consts::ERROR) {
    LOG_err << "Failed to make it nonblock fd. Invalid fd." << endl; 
    throw Exception(ExceptionType::NON_BLOCK_ERROR); 
  }

  flags |= O_NONBLOCK;
  
  int result = fcntl (fd, F_SETFL, flags);
  if (result == consts::ERROR) {
    LOG_err << "Failed to make it nonblock fd. Set flag failed." << endl; 
    throw Exception(ExceptionType::NON_BLOCK_ERROR); 
  }
}


}

#if _UNIT_TEST


#include <iostream>
#include <unistd.h>

using namespace lio;


int main() {
  const std::string sockName = "./.testSock";

  pid_t id = fork();
  if (id == 0) {
    // child (client)
    sleep(1);
    DEBUG_cout << "Child Start!" << endl; 
    AsyncSockets* asock = new AsyncSockets(Socket::SocketFamily::LOCAL);
    asock->Connect(sockName);

    /*
    std::string userInput;
    while (true) {
      std::getline(std::cin, userInput);

      if (userInput == "q") {
        break;
      }
      
      asock->Write(userInput);
    }
    */
    

    std::string data = "12345678901234567890123456789012345678901234567890";

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


