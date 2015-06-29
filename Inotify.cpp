#include "Inotify.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
Inotify::Exception::exceptionMessages_[] = {
  INOTIFY_EXCEPTION_MESSAGES
};
#undef INOTIFY_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Inotify::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Inotify::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Inotify::ExceptionType
Inotify::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


Inotify::Inotify() :
  numMaxEvent_(50),
  inotiFd_(inotify_init()),
  epollFd_(0),
  isEpollInternal_(true)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
  DEBUG_cout << "Inotifd: " << this->inotiFd_ << endl; 
  if (this->inotiFd_ < 0) {
    DEBUG_cerr << "Failed to initialize" << endl; 
    throw Exception(ExceptionType::INIT_FAIL);
    return;
  } 

  this->events_ = (epoll_event *) calloc (this->numMaxEvent_, sizeof (epoll_event));
  AsyncIo::setNonBlocking(this->inotiFd_);

  this->epollFd_ = AsyncIo::createEpoll();
  AsyncIo::addFdToEpoll(this->epollFd_, this->inotiFd_);
}

Inotify::Inotify(int epollFd) :
  numMaxEvent_(50),
  inotiFd_(inotify_init()),
  epollFd_(epollFd),
  isEpollInternal_(false)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
  if (this->inotiFd_ < 0) {
    DEBUG_cerr << "Failed to initialize" << endl; 
    throw Exception(ExceptionType::INIT_FAIL);
    return;
  } 

  AsyncIo::setNonBlocking(this->inotiFd_);
  AsyncIo::addFdToEpoll(this->epollFd_, this->inotiFd_);
}

Inotify::~Inotify() {
  DEBUG_FUNC_START;

  for (auto& path : this->watchingDirPaths_) {
    this->RemoveFromWatch(path.first);
  } 

  if (this->events_ != nullptr) {
    delete this->events_;
  } 
  if (this->isEpollInternal_) {
    // Epoll is created by this module. So close it.
    close(this->epollFd_);
  } else {
    // Epoll is not created by this module. So do not close it here.
  }
  close(this->inotiFd_);

}

int Inotify::GetInotifyFd() const {
  return this->inotiFd_;
}

const map<int, string>& Inotify::GetWatchingDirs() const {
  return this->watchingDirPaths_;
}
void Inotify::AddToWatch(const string& targetPath, uint32_t watchEvent) {
  int wd = inotify_add_watch(this->inotiFd_, targetPath.c_str(), watchEvent);

  if (wd < 0) {
    DEBUG_cerr << "Failed to add to watch. path: " << targetPath << endl; 
    return;
  } 
  
  this->watchingDirPaths_[wd] = targetPath;

  return;
}

void Inotify::RemoveFromWatch(int wd) {
  auto it = this->watchingDirPaths_.find(wd);
  if (it != this->watchingDirPaths_.end()) {
    inotify_rm_watch(this->inotiFd_, (*it).first);
    this->watchingDirPaths_.erase(it);
    DEBUG_cout << "Removed inotify watch for " << this->watchingDirPaths_[wd] << endl; 
  } else {
    DEBUG_cerr << "Tried to remove inotify watch but it did not exist." << endl; 
  }

}

void Inotify::SetFileCreateHandler(std::function<void(const string&)> handler) {
  if (handler != nullptr) {
    this->fileCreateHandler_ = handler;
  } 
}
void Inotify::SetFileModifyHandler(std::function<void(const string&)> handler) {
  if (handler != nullptr) {
    this->fileModifyHandler_ = handler;
  } 
}
void Inotify::SetFileDeleteHandler(std::function<void(const string&)> handler) {
  if (handler != nullptr) {
    this->fileDeleteHandler_ = handler;
  } 
}

void Inotify::StartWatching() {
  if (this->watchingDirPaths_.empty()) {
    DEBUG_cerr << "Nothing is added to watching list. Doing nothing." << endl; 
    return;
  } 
  DEBUG {
    DEBUG_cout << "Now Watching directories." << endl; 
    for (auto& dir : this->watchingDirPaths_) {
      DEBUG_cout << dir.second << endl; 
    } 
  }
  this->waitForEvent();
}

ssize_t Inotify::HandleInotifyEvent(int fd) {
  return this->processEvent(fd);
}

void Inotify::waitForEvent(ssize_t epollTimeout) {
  while (true) {
    ssize_t numEvents = 0;
    DEBUG_cout << "Waiting..." << endl;
    numEvents = epoll_wait (this->epollFd_, this->events_, this->numMaxEvent_, epollTimeout);
    DEBUG_cout << "  Event Triggered! " << "numEvents: " << numEvents << endl;
    for (ssize_t i = 0; numEvents > i; ++i) {
      if (this->events_[i].events & EPOLLIN) {
        this->processEvent(this->events_[i].data.fd);

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

ssize_t Inotify::processEvent(int fd) {
  DEBUG_cout << "EventFd: " << fd << endl; 
  char buffer[1024 * (sizeof(struct inotify_event) + 16) ];
  ssize_t readCount = 0;
  while (true) {
    readCount = read(fd, buffer, sizeof(buffer));
    if (readCount < 0) {
      if (errno == EAGAIN) {
        DEBUG_cout << "Read all." << endl; 
      } else {
        DEBUG_cerr << "read Error. errno: " << errno << endl; 
      }
      break;
    } 
    DEBUG_cout << "ReadCount: " << readCount << endl; 

    ssize_t index = 0;
    while (index < readCount) {
      struct inotify_event *event = (struct inotify_event *) &buffer[index];
      if (event->len) {
        DEBUG_cout << "Name: " << event->name << endl; 

        const std::map<int, string>& dirs = this->GetWatchingDirs();

        auto it = dirs.find(event->wd);
        if (it == dirs.end()) {
          DEBUG_cerr << "NOTI ERROR. Continuing." << endl; 
          goto NOTISKIP;
        } 

        const string& dirName = (*it).second;
        DEBUG_cout << "dirName: " << dirName << endl; 
        string filePath = dirName;
        if (filePath.back() != '/') {
          filePath.push_back('/');
        } 
        filePath.append(event->name);

        if (event->mask & IN_CREATE) {
          if (event->mask & IN_ISDIR) {
            // Directory Created.
            DEBUG_cout << "Direcotry Created." << endl; 
          } else {
            DEBUG_cout << "File Created" << endl; 
            if (this->fileCreateHandler_) {
              this->fileCreateHandler_(filePath);
            } else {
              DEBUG_cout << "No handler has been set " << endl; 
            }
          }
        } else if (event->mask & IN_DELETE) {
          if (event->mask & IN_ISDIR) {
            // Directory Deleted.
            DEBUG_cout << "Directory Deleted." << endl; 
          } else {
            DEBUG_cout << "File Deleted" << endl; 
            if (this->fileDeleteHandler_) {
              this->fileDeleteHandler_(filePath);
            } else {
              DEBUG_cout << "No handler has been set " << endl; 
            }
          }
        } else if (event->mask & IN_MODIFY) {
          if (event->mask & IN_ISDIR) {
            // Directory Modified.
            DEBUG_cout << "Directory modified" << endl; 

          } else {
            DEBUG_cout << "File modified." << endl; 
            if (this->fileModifyHandler_) {
              this->fileModifyHandler_(filePath);
            } else {
              DEBUG_cout << "No handler has been set " << endl; 
            }
          }
        } else if (event->mask & IN_MOVED_FROM) {
          if (event->mask & IN_ISDIR) {
            DEBUG_cout << "Directory Renamed from." << endl; 
          } else {
            DEBUG_cout << "File Renamed from." << endl; 
          }
        } else if (event->mask & IN_MOVED_TO) {
          if (event->mask & IN_ISDIR) {
            DEBUG_cout << "Directory Renamed to." << endl; 
          } else {
            DEBUG_cout << "File Renamed to." << endl; 
          }
        } 
      } 
NOTISKIP:
      index += sizeof(struct inotify_event);
    }
  } 

  return readCount;
}

//Inotify::
//Inotify::

}

#if _UNIT_TEST
/*
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockInotify : public Inotify {
public:
  MOCK_METHOD2(AddToWatch, void(const string&, uint32_t));
  MOCK_METHOD0(StartToWatch, void());

};

using ::testing::AtLeast;

TEST(Inotify, TESTNAME) {
  MockInotify mockInotify;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
*/

using namespace lio;

void filecreate(const string& name) {
  DEBUG_cout << "Handler Called! name: " << name << endl; 
}
int main () {
  DEBUG_cout << "Inotify Test has started." << endl; 
  Inotify* inoti = new Inotify();
  inoti->AddToWatch("./util/");
  //inoti->SetFileCreateHandler(filecreate);
  inoti->SetFileCreateHandler([](const string& name) {
      DEBUG_cout << "Lambda! name: " << name << endl; 
      return;
  });
  inoti->StartWatching();
  delete inoti;

}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

