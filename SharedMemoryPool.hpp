#ifndef _SHAREDMEMORYPOOL_HPP_
#define _SHAREDMEMORYPOOL_HPP_

/*
  Name
    SharedMemoryPool

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description


  Last Modified Date
    Mar 2, 2013

  Learning Resources
    Semaphores Tutorial
      http://beej.us/guide/bgipc/output/html/multipage/semaphores.html
  
  Copyright (c) All Rights reserved to LIFEINO.
*/

// CONCURRENCY PROBLEM
// GOTTA USE SEMAPHORES

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "Debug.hpp" // DEBUG_cout

#include <string>

#include <sys/ipc.h> // ftok()
#include <sys/sem.h> // semget() semctl() semop()


#include "Semaphore.hpp"
#include "MemoryPool.hpp"
#include "SharedMemory.hpp"


using std::string;


class SharedMemoryPool : protected MemoryPool {
public:
  struct Config {
    
  };
  SharedMemoryPool(const key_t &uniqueKey, size_t poolSize, size_t blockSize = 1024,
                   shm_mode_t shmMode = shm_mode_t::CREATE);

  ~SharedMemoryPool();
  
  void*         Smpalloc(size_t allocSize);
  bool          Smpfree(void* freePtr);
  
  // TEST ONLY
  void          t_PrintMap();
protected:

  SharedMemoryPool(const key_t &uniqueKey, size_t shmSize);
  virtual
  bool          initMemoryPool();
private:
  key_t         m_uniqueKey;
  
  SharedMemory* m_shm;
  Semaphore*    m_sem;
  
  
};

#endif
