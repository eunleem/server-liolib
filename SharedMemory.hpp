#ifndef _SHAREDMEMORY_HPP_
#define _SHAREDMEMORY_HPP_
/*
  Name
    SharedMemory

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description
    Shared Memory

  Last Modified Date
    Mar 2, 2013

  Learning Resources
    Tutorial
      http://beej.us/guide/bgipc/output/html/multipage/shm.html
    Definition
      http://linux.die.net/man/2/shmget
      http://linux.die.net/man/2/shmat
      http://linux.die.net/man/3/ftok
    Epoll Implementation Example
      #TODO: Add Examples
  
  Copyright (c) All Rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "Debug.hpp"

#include <string>
#include <exception>

#include <cstdint> // intptr_t

#include <sys/ipc.h> // shmget()
#include <sys/shm.h> // shmget() shmat()
#include <unistd.h> // getpagesize()
#include <sys/types.h> // key_t ftok() shmat()
#include <errno.h> // errno

#include "Util.hpp" // GenerateUniqueKey()




using std::string;



namespace lio {

enum class SharedMemoryExceptionType : std::uint16_t {
  GENERAL,
  NOT_INIT,
  INIT_FAILED,
  SHM_EXISTS,
  SHM_ACCESS_DENIED,
  SHM_ATTACH_FAILED
};

#define SHARED_MEMORY_EXCEPTION_MESSAGES \
  "SharedMemory Exception has been thrown.", \
  "SharedMemory is not Initialized Yet.", \
  "Inialization of Shared Memory has failed.", \
  "SharedMemory already exists.", \
  "SharedMemory already exists. And Access is denied.", \
  "Failed to attach to SharedMemory using ShmId."

class SharedMemoryException : public std::exception {
public:
  SharedMemoryException (SharedMemoryExceptionType exceptionType = SharedMemoryExceptionType::GENERAL);

  virtual const char* what() const throw();
  virtual const SharedMemoryExceptionType type() const throw();
private:
  SharedMemoryExceptionType     exceptionType_;
  static const char* const    exceptionMessages_[];
};



class SharedMemory {
public:
  enum class Mode {
    AUTO,
    CREATE,
    LOAD
  };

  struct Config {
    Config() 
      : permission(0640) {}
    key_t key;
    size_t size;
    Mode mode;
    int permission;
  };

  SharedMemory(const Config& config);
  virtual
  ~SharedMemory();

  static
  key_t GenerateKey(const string& keyFilePath, int8_t projId = 1);


  const size_t      GetShmSize() const;
  const void*       GetShmAddress() const ;

  void              SetToRemoveOnDelete();
  
  
  // Test Function
  int         t_GetShmId();

  
protected:
  //int         initShm(bool isToCreate);


private:
  Config      config_;

  int         shmId_;
  void*       shmAddress_;

  // if true, Destructor will get rid of SHM
  bool        isSetToDestroyShm_;
  
  void        createShm();
  void        loadShm();

  void        removeShm();
};

}

#endif

