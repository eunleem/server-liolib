#include "SharedMemoryPool.hpp"

#define ONSHAREDMEMORY true

SharedMemoryPool::SharedMemoryPool(const key_t &uniqueKey, size_t poolSize, size_t blockSize, shm_mode_t shmMode)
: MemoryPool (poolSize, blockSize, ONSHAREDMEMORY) { // True: On Shared Memory Mode Enabled.
   
  m_uniqueKey = uniqueKey;

  m_sem = new Semaphore(m_uniqueKey);
  
  // This function allocates shared memory
  this->initMemoryPool(); 
}

SharedMemoryPool::~SharedMemoryPool() {
  m_shm->SetToDestroyShmOnDelete();
  m_sem->SetToDestroySemOnDelete();
  
  delete m_shm;
  delete m_sem;
}

bool SharedMemoryPool::initMemoryPool() {
  m_shm = new SharedMemory(m_uniqueKey, this->GetPoolSize(), shm_mode_t::CREATE);
  this->setPoolHeadPtr(m_shm->GetShmAddr()); // Set poolHeadPtr to 
  
  return true;
}

void* SharedMemoryPool::Smpalloc(size_t allocSize) {
  m_sem->Lock(0);
  void* allocatedMemory = Mpalloc(allocSize);
  m_sem->Release(0);
  return allocatedMemory;
}

bool SharedMemoryPool::Smpfree(void* freePtr) {
  m_sem->Lock(0);
  Mpfree(freePtr);
  m_sem->Release(0);
  return true;
}

void SharedMemoryPool::_PrintMap() {
  m_sem->Lock(0);
  _PrintBlockMap();
  m_sem->Release(0);
}




#undef ONSHAREDMEMORY 

//TEST

#define _UNIT_TEST 0
#if _UNIT_TEST

#include <iostream>
#include <unistd.h>

int main() {
  pid_t forkpid;
  forkpid = fork();
  try{
    if (forkpid > 0) {
      // Parent
      SharedMemoryPool* shmp = new SharedMemoryPool("AioNetwork.cpp", 2048*1024);
      //shmp->_printPoolInfo();
       
      uintptr_t smpA = shmp->Smpalloc(1024);
      std::cout << smpA << std::endl;
      uintptr_t smpB = shmp->Smpalloc(191100);
      std::cout << smpB << std::endl;
      uintptr_t smpC = shmp->Smpalloc(291100);
      std::cout << smpC << std::endl;
      uintptr_t smpD = shmp->Smpalloc(391100);
      std::cout << smpD << std::endl;

      shmp->Smpfree(smpB);

      shmp->Smpfree(smpC);

      uintptr_t smpE = shmp->Smpalloc(22024);
      std::cout << smpE << std::endl;

      delete shmp;
    } else if (forkpid == 0) {
      SharedMemoryPool* shmp = new SharedMemoryPool("AioNetwork.cpp", 2048*1024);
      //shmp->_printPoolInfo();

       
      uintptr_t smpA = shmp->Smpalloc(1024);
      std::cout << smpA << std::endl;
      uintptr_t smpB = shmp->Smpalloc(191100);
      std::cout << smpB << std::endl;
      uintptr_t smpC = shmp->Smpalloc(291100);
      std::cout << smpC << std::endl;
      uintptr_t smpD = shmp->Smpalloc(391100);
      std::cout << smpD << std::endl;

      shmp->Smpfree(smpB);

      shmp->Smpfree(smpC);

      uintptr_t smpE = shmp->Smpalloc(22024);
      std::cout << smpE << std::endl;

      delete shmp;
    }
  } catch (int e) {
    std::cout << "e: " << e << std::endl;
  }
  return 0;
}
#endif
#undef _UNIT_TEST
