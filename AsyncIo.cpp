#include "AsyncIo.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"

namespace lio {


// ===== Exception Implementation =====
const char* const
AsyncIo::Exception::exceptionMessages_[] = {
  ASYNCIO_EXCEPTION_MESSAGES
};
#undef ASYNCIO_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


AsyncIo::Exception::Exception(ExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
AsyncIo::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const AsyncIo::ExceptionType
AsyncIo::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


// SERVER MODE
AsyncIo::AsyncIo(const int fd) :
  numMaxEvent_(50),
  epollFd_(0),
  fd_(fd),
  status_(Status::INIT),
  isSetToStop_(false),
  events_(nullptr)
{
  DEBUG_FUNC_START;

  this->epollFd_ = this->createEpoll();

  this->events_ = (epoll_event *) calloc (this->numMaxEvent_, sizeof (epoll_event));
}

AsyncIo::~AsyncIo() {
  DEBUG_FUNC_START;
  /*
  if (this->events != NULL) {
    delete this->events;
  }*/

  delete events_;
}

void AsyncIo::StartWaiting() {
  this->setNonBlocking(this->fd_);
  this->addFdToEpoll(this->epollFd_, this->fd_);

  this->status_ = Status::WAITING;
  this->waitForEvent();
}

void AsyncIo::Stop() {
  DEBUG_FUNC_START;
  // CLOSE SOCKET
  if (this->status_ == Status::WAITING ||
      this->status_ == Status::STOPPING)
  {
    DEBUG_cout << "AsyncIo is now stopped and fd is closed." << endl; 
    close(this->fd_);
  } 

  this->status_ = Status::STOPPED;
}

void AsyncIo::StopGracefully() {
  DEBUG_FUNC_START;
  if (this->status_ == Status::WAITING) {
    DEBUG_cout << "AsyncIo is Set to Gracefully stop." << endl; 
    this->isSetToStop_ = true;
    this->status_ = Status::STOPPING;
  } 
}

void AsyncIo::waitForEvent() {
  // #REF:
  //  http://man7.org/linux/man-pages/man2/epoll_wait.2.html
  while (true) {
    ssize_t numEvents = 0;
    DEBUG_cout << "Waiting..." << endl;
    numEvents = epoll_wait (this->epollFd_, this->events_, this->numMaxEvent_, -1);
    if (numEvents == -1) {
      if (numEvents == EINTR) {
        DEBUG_cout << "epoll_wait was interrupted by Signal." << endl; 
      } else {
        DEBUG_cerr << "epoll_wait error. errno: " << errno << endl; 
      }
    } 
    DEBUG_cout << "  Event Triggered! " << "numEvents: " << numEvents << endl;
    FdEventArgs::EventType eventType;
    for (ssize_t i = 0; numEvents > i; ++i) {

      if (this->events_[i].events & EPOLLIN) {
        eventType = FdEventArgs::EventType::EPOLLIN;
        this->OnFdEvent(FdEventArgs(this->events_[i].data.fd, eventType));
      }

      if (this->events_[i].events & EPOLLOUT) {
        eventType = FdEventArgs::EventType::EPOLLOUT;
        this->OnFdEvent(FdEventArgs(this->events_[i].data.fd, eventType));
      }

      if ( (this->events_[i].events & EPOLLERR) ||
           (this->events_[i].events & EPOLLHUP) )
      {
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
      DEBUG_cout << "AsyncIo is Gracefully Stopping." << endl; 
      this->Stop();
      return;
    }
  }
}

void AsyncIo::OnFdEvent(const FdEventArgs& event) {
  assert(!"OnFdEvent must be overriden with custom implementation.");
}



int AsyncIo::createEpoll() {
  int epollFd = epoll_create1(0);
  if (epollFd == consts::ERROR) {
    DEBUG_cerr << "Failed to create epollfd." << endl; 
    throw Exception (ExceptionType::EPOLL_ERROR);
  }
  return epollFd;
}    


bool AsyncIo::addFdToEpoll (const int epollFd, const int targetFd, const uint32_t events) {
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
    DEBUG_cerr << "controlEpoll has failed." << endl; 
    throw Exception(ExceptionType::EPOLL_ERROR);
    return false;
  }

  return true;
}


void AsyncIo::removeFdFromEpoll (const int fd) {
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
void AsyncIo::setNonBlocking (const int fd) {
  if ( fd < 0 ) {
    DEBUG_cerr << "Descriptor cannot be less than 0." << endl; 
    throw Exception(ExceptionType::NON_BLOCK_ERROR); 
    return;
  }

  int flags = fcntl (fd, F_GETFL, 0);
  if (flags == consts::ERROR) {
    DEBUG_cerr << "Invalid fd." << endl; 
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
    AsyncIo* asock = new AsyncIo(Socket::SocketFamily::LOCAL);
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
    AsyncIo* asock = new AsyncIo(Socket::SocketFamily::LOCAL);
    asock->Listen(sockName);
  }



  return 0;
}

#endif
#undef _UNIT_TEST


