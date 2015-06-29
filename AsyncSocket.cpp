#include "AsyncSocket.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
AsyncSocket::Exception::exceptionMessages_[] = {
  ASYNCSOCKET_EXCEPTION_MESSAGES
};
#undef ASYNCSOCKET_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


AsyncSocket::Exception::Exception(ExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
AsyncSocket::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const AsyncSocket::ExceptionType
AsyncSocket::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====

const int AsyncSocket::defaultNumEventMax_ = 100;

// SERVER MODE

AsyncSocket::AsyncSocket(const Socket::SocketFamily sockFamily,
                         const Socket::SocketType sockType) :
  socket_(nullptr),
  numMaxEvent_(this->defaultNumEventMax_),
  epollFd_(0),
  isSetToStop_(false),
  events_(nullptr),
  status_(Status::INIT)
{
  DEBUG_FUNC_START;
  DEBUG_cout << "  SocketFamily: " << (sockFamily == Socket::SocketFamily::LOCAL?"LOCAL" : "IP") << endl;
  DEBUG_cout << "  SocketType: " << (sockType == Socket::SocketType::TCP?"TCP" : "UDP") << endl;

  this->socket_ = new Socket(sockFamily, sockType);
  this->epollFd_ = this->createEpoll();

  this->events_ = (epoll_event *) calloc (this->numMaxEvent_, sizeof (epoll_event));
}

AsyncSocket::AsyncSocket(const int existingFd,
                         Socket::SocketMode mode,
                         const Socket::SocketFamily sockFamily,
                         const Socket::SocketType sockType) :
  socket_(nullptr),
  mode_(mode),
  numMaxEvent_(this->defaultNumEventMax_),
  epollFd_(0),
  isSetToStop_(false),
  events_(nullptr),
  status_(Status::INIT)
{
  DEBUG_FUNC_START;
  DEBUG_cout << "  SocketFamily: " << (sockFamily == Socket::SocketFamily::LOCAL?"LOCAL" : "IP") << endl;
  DEBUG_cout << "  SocketType: " << (sockType == Socket::SocketType::TCP?"TCP" : "UDP") << endl;

  this->socket_ = new Socket(existingFd, mode, sockFamily, sockType);
  this->epollFd_ = this->createEpoll();

  this->events_ = (epoll_event *) calloc (this->numMaxEvent_, sizeof (epoll_event));
}


AsyncSocket::~AsyncSocket() {
  DEBUG_FUNC_START;
  /*
  if (this->events != NULL) {
    delete this->events;
  }*/

  if (this->status_ != Status::STOPPED) {
    this->Stop();
  } 

  delete events_;
  delete this->socket_;
}

void AsyncSocket::StopGracefully() {
  DEBUG_FUNC_START;
  if (this->status_ == Status::LISTENING ||
      this->status_ == Status::CONNECTED) {
    DEBUG_cout << "AsyncSocket is Set to Gracefully stop." << endl; 
    this->isSetToStop_ = true;
    this->status_ = Status::STOPPING;
  } 

}

void AsyncSocket::Stop() {
  DEBUG_FUNC_START;
  // CLOSE SOCKET
  if (this->status_ == Status::LISTENING ||
      this->status_ == Status::CONNECTED ||
      this->status_ == Status::STOPPING) {
    DEBUG_cout << "AsyncSocket is now stopped and socket is closed." << endl; 
    this->socket_->Close();
  } 

  this->status_ = Status::STOPPED;
}

void AsyncSocket::SetNumMaxEvent(const int maxEvent) {
  if (maxEvent <= 0) {
    DEBUG_cerr << "MaxEvent cannot be equal to or less than 0." << endl; 
    return;
  } 
  this->numMaxEvent_ = maxEvent;
}

int AsyncSocket::GetSocketFd() const {
  return this->socket_->GetSocketFd();
}

int AsyncSocket::GetEpollFd() const {
  return this->epollFd_;
}

void AsyncSocket::Listen() {
  DEBUG_FUNC_START;
  int socketFd = this->socket_->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::LISTENING;
  this->waitForEvent();
}

void AsyncSocket::Listen(const string& socketName) {
  DEBUG_FUNC_START;
  assert (this->socket_->GetSocketFamily() == Socket::SocketFamily::LOCAL &&
          "SocketFamily must be LOCAL");

  this->mode_ = Socket::SocketMode::LISTEN;

  bool result = this->socket_->Listen(socketName);
  if (result == false) {
    std::cerr << "Could not initialize Socket!" << endl;
    return;
  }
  
  int socketFd = this->socket_->GetSocketFd();
  this->setNonBlocking (socketFd);
  
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::LISTENING;
  this->waitForEvent();
}

void AsyncSocket::Connect (const string& sockName) {
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

void AsyncSocket::Listen(const uint16_t portNumber) {
  DEBUG_FUNC_START;
  assert (this->socket_->GetSocketFamily() != Socket::SocketFamily::LOCAL &&
          "SocketFamily must be NON-LOCAL");
  
  this->mode_ = Socket::SocketMode::LISTEN;

  bool result = this->socket_->Listen(portNumber);
  if (result == false) {
    std::cerr << "Could not initialize Socket!" << endl;
    return;
  }
  
  const int socketFd = this->socket_->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::LISTENING;
  this->waitForEvent();
}

void AsyncSocket::ListenFirst(const uint16_t portNumber) {
  DEBUG_FUNC_START;
  assert (this->socket_->GetSocketFamily() != Socket::SocketFamily::LOCAL &&
          "SocketFamily must be NON-LOCAL");
  
  this->mode_ = Socket::SocketMode::LISTEN;

  bool result = this->socket_->Listen(portNumber);
  if (result == false) {
    std::cerr << "Could not initialize Socket!" << endl;
    return;
  }

  
  const int socketFd = this->socket_->GetSocketFd();
  this->setNonBlocking (socketFd);
  this->addFdToEpoll(this->epollFd_, socketFd);

  this->status_ = Status::LISTENING;
  //this->waitForEvent();
}


void AsyncSocket::Wait() {
  if (this->status_ == Status::LISTENING) {
    this->waitForEvent();
  }
}

void AsyncSocket::Connect (const uint16_t portNumber, const string& destIpAddr) {
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

int AsyncSocket::Write(const void* dataLocation, size_t length) {
  if (this->status_ == Status::LISTENING ||
      this->status_ == Status::CONNECTED) {
    return this->socket_->Write(dataLocation, length);
  } else {
    DEBUG_cerr << "Tried to write to socket that is not listening nor connected" << endl; 
  }
  return -1;
}

int AsyncSocket::Write(const string& content) {
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

ssize_t AsyncSocket::Read(string* content) {
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




void AsyncSocket::waitForEvent(ssize_t epollTimeout) {
  // epollTimeout is in milliseconds. -1 means no timeout, 0 means return immediately.
  // #TEST: instead of class member, using local var.
  
  //int socketFd = this->GetSocketFd();
  
  while (true) {
    if (this->status_ != Status::LISTENING) {
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

    if (this->isSetToStop_ == true) {
      DEBUG_cout << "AsyncSocket is Gracefully Stopping." << endl; 
      this->Stop();
      return;
    } 
  }
}

void AsyncSocket::OnFdEvent(const FdEventArgs& event) {
  TEST {
    if (event.fd == this->socket_->GetSocketFd()) {
      while (true) {
        DEBUG_cout << "  SocketEvent\n";

        struct sockaddr in_addr;
        socklen_t in_len = sizeof (in_addr);

        int acceptFd = accept (event.fd, &in_addr, &in_len);
        if (acceptFd == consts::ERROR) {
          if ((errno == EAGAIN) ||
              (errno == EWOULDBLOCK)) {
            // All requests have been processed.
            DEBUG_cout << "   SocketEvent: All Request have been processed." << endl; 
            return;
          } else {
            DEBUG_cerr << "   AsyncSocket Accept Error" << endl;
            throw Exception (ExceptionType::ACCEPT_ERROR);
            return;
          }
        }
        DEBUG_cout << "  AcceptEvent: " << acceptFd << endl;
        this->setNonBlocking (acceptFd);
        this->addFdToEpoll (this->epollFd_, acceptFd);
      }
      DEBUG_cerr << "   Server Event Unknown. Ignored." << endl;
      return;

    } else {
      DEBUG_cout << "     AcceptEvent\n";
      string readStr = "";
      while (true) {
        char buf[2048];
        memset(buf, 0, sizeof(buf));
        ssize_t readCount = read(event.fd, buf, sizeof(buf));
        if (readCount == -1) {
          if (errno == EAGAIN) {
            DEBUG_cout << "Socket is no longer readable." << endl; 
            break;

          } 
        } else if (readCount > 0) {
          readStr.append(buf);
        } else if (readCount == 0) {
          DEBUG_cout << "Clinent has closed connection." << endl; 
          close(event.fd);
        }
      }
      sleep(3);
      DEBUG_cout << "String Read. ReadCount: " << readStr.length() << endl; 
      //DEBUG_cout << readStr << endl;
      readStr.clear();
    }

  } else {
    assert(!"OnAcceptEvent must be overriden with custom implementation.");
    return;
  }
  
}



int AsyncSocket::createEpoll() {
  int epollFd = epoll_create1(0);
  if (epollFd == consts::ERROR) {
    DEBUG_cerr << "Failed to create epollfd." << endl; 
    throw Exception (ExceptionType::EPOLL_ERROR);
  }
  DEBUG_cout << "Epoll has been created. EpollFd: " << epollFd << endl; 
  return epollFd;
}    


bool AsyncSocket::addFdToEpoll (const int epollFd, const int targetFd, const uint32_t events) {
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


void AsyncSocket::removeFdFromEpoll (const int fd) {
  assert(!"NOT YET IMPLEMENTED");
}

//  Name
//    Make Fd Non Blocking
//  Parameter
//    int fd
//  Return
//    True on Success
//    False on Fail
//
void AsyncSocket::setNonBlocking (const int fd) {
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
    AsyncSocket* asock = new AsyncSocket(Socket::SocketFamily::LOCAL);
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
    AsyncSocket* asock = new AsyncSocket(Socket::SocketFamily::LOCAL);
    asock->Listen(sockName);
  }



  return 0;
}

#endif
#undef _UNIT_TEST


