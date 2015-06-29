#ifndef _INOTIFY_HPP_
#define _INOTIFY_HPP_
/*
  Name
    Inotify
      Detects file changes in a directory.

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 11, 2015
  
  History
    Mar 25, 2014
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <string> // string
#include <list> // list
#include <map>
#include <functional> // function

#include <sys/types.h>
#include <sys/inotify.h> // inotify_init()

#include "liolib/AsyncIo.hpp" // AsyncIo
#include "liolib/Util.hpp" // IsDirectoryExisting()

//#inclue "liolib/DataBlock.hpp"

namespace lio {

using std::string;

class Inotify {
public:
// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  INIT_FAIL
};
#define INOTIFY_EXCEPTION_MESSAGES \
  "Inotify Exception has been thrown.", \
  "Could not init inotify."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********

  enum class Event : uint8_t {
    DIR_CREATE = 1,
    DIR_MODIFY = 2,
    DIR_DELETE = 4,
    FILE_CREATE = 8,
    FILE_MODIFY = 16,
    FILE_DELETE = 32
  };

  Inotify();
  Inotify(int epollFd);
  virtual
  ~Inotify();

  int GetInotifyFd() const;
  const std::map<int, string>& GetWatchingDirs() const;

  void AddToWatch(const string& dirPath,
      uint32_t watchEvent = IN_MOVE | IN_MODIFY | IN_CREATE | IN_DELETE);

  void RemoveFromWatch(int wd);

  virtual
  void SetFileCreateHandler(std::function<void(const string&)>);
  virtual
  void SetFileModifyHandler(std::function<void(const string&)>);
  virtual
  void SetFileDeleteHandler(std::function<void(const string&)>);

  ssize_t HandleInotifyEvent(int fd);

  void StartWatching();

  
  
protected:
  
private:

  void waitForEvent(ssize_t epollTimeout = -1);

  ssize_t processEvent(int fd);

  const
  uint16_t      numMaxEvent_;
  struct epoll_event* events_;

  int inotiFd_;
  int epollFd_;

  bool isEpollInternal_;

  std::map<int, string> watchingDirPaths_;

  std::function<void(const string&)> fileCreateHandler_;
  std::function<void(const string&)> fileDeleteHandler_;
  std::function<void(const string&)> fileModifyHandler_;

  std::function<void(const string&)> dirCreateHandler_;
  std::function<void(const string&)> dirDeleteHandler_;
  std::function<void(const string&)> dirModifyHandler_;
  
};

}

#endif

