#include "SharedMemory.hpp"


namespace lio {

// ===== Exception Implementation =====
const char* const SharedMemoryException::exceptionMessages_[] = {
  SHARED_MEMORY_EXCEPTION_MESSAGES
};
#undef SHARED_MEMORY_EXCEPTION_MESSAGES // no longer used. For better compiling performance, undefine no longer used #define is recommended.

SharedMemoryException::SharedMemoryException(SharedMemoryExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char* SharedMemoryException::what() const throw() {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const SharedMemoryExceptionType SharedMemoryException::type() const throw() {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====




SharedMemory::SharedMemory(const Config& config)
  : config_(config),
    shmId_(-1),
    shmAddress_(nullptr),
    isSetToDestroyShm_(false) {

   DEBUG_FUNC_START;

  size_t pageSize = getpagesize();
  this->config_.size = ((config.size / pageSize) + 1) * pageSize;
  DEBUG_cout << "Actual SharedMemory Size is multiples of PageSize. ShmSize created: " << this->config_.size << endl;
  this->isSetToDestroyShm_ = false;
  // 3. shmget to get shmId with IPC_CREAT or IPC_EXCL flag
  
  if (this->config_.mode == Mode::AUTO) {
    try {
      this->createShm();
      DEBUG_cout << "SHM Mode is Auto. Created Shm." << endl;
    } catch (SharedMemoryException& ex) {
      if (ex.type() == SharedMemoryExceptionType::SHM_EXISTS) {
        this->loadShm();
        DEBUG_cout << "SHM Mode is Auto. Loaded Shm." << endl;
      } else {
        DEBUG_cerr << "SHM Auto mode failed. " << ex.what() << endl; 
        throw SharedMemoryException(ex.type());
      }
    }
  } else if (this->config_.mode == Mode::CREATE) {
    this->createShm();
  } else if (this->config_.mode == Mode::LOAD) {
    this->loadShm();
  }
}

SharedMemory::~SharedMemory() {
  DEBUG_cout << "~SharedMemory() is called.\n";
  // Detach from Shm
  shmdt((void *)this->shmAddress_);
  // If it's set to "DESTROY" the Shm, Do so.
  if (this->isSetToDestroyShm_) {
    DEBUG_cout << "SHM is removed." << endl; 
    this->removeShm();
  }
}
void SharedMemory::removeShm() {
  DEBUG_cout << "Shm is set to destroy. Destroying Shm. ShmId : " << this->shmId_ << endl;
  shmctl(this->shmId_, IPC_RMID, NULL);
}

key_t SharedMemory::GenerateKey (const string& keyFilePath, int8_t projId) {
  // #TODO: Check whether the file exists or not.
  if (projId <= 0) {
    // Minor Error. Show Warning or leave a warning Log.
    // Doesn't have to stop the program tho.
  }
  return ftok (keyFilePath.c_str(), static_cast<int>( projId));
}

void SharedMemory::SetToRemoveOnDelete() {
  DEBUG_FUNC_START;
  this->isSetToDestroyShm_ = true;
}

const void* SharedMemory::GetShmAddress() const  {
  if ( this->shmAddress_ == nullptr) {
    DEBUG_cerr << "Tried to get Shm Address before creating SHM" << endl;
    throw SharedMemoryException(SharedMemoryExceptionType::NOT_INIT);
  }
  return this->shmAddress_;
}

const size_t SharedMemory::GetShmSize() const {
  return this->config_.size;
}




// ***************** Private Functions *********************

void SharedMemory::createShm() {
  key_t key = this->config_.key;
  size_t size = this->config_.size;
  int permission = this->config_.permission;

  this->shmId_ = shmget(key, size, permission | IPC_CREAT | IPC_EXCL);
  
  if (this->shmId_ >= 0) {
    DEBUG_cout << "New Shared Memory has been created.\n";
  } else if (errno == EEXIST) {
    throw SharedMemoryException(SharedMemoryExceptionType::SHM_EXISTS);
  } else if (errno == EACCES) {
    throw SharedMemoryException(SharedMemoryExceptionType::SHM_ACCESS_DENIED);
  } else {
    throw SharedMemoryException(SharedMemoryExceptionType::INIT_FAILED);
  }
  //std::cout << "shmId: " << this->shmId << std::endl;
  void* result = shmat(this->shmId_, (void *)0, 0);
  if ((long) result <= -1) {
    DEBUG_cerr << "Creating Shm. Shmat failed." << endl; 
    throw SharedMemoryException(SharedMemoryExceptionType::SHM_ATTACH_FAILED);
  }
  this->shmAddress_ = result;
  DEBUG_cout << "Successfully attached to the newly created shared memory.\n";
}

void SharedMemory::loadShm() {
  this->shmId_ = shmget(this->config_.key, 0, IPC_EXCL);
  if (this->shmId_ >= 0) {
    this->shmAddress_ = shmat(this->shmId_, (void *)0, 0);
  } else {
    DEBUG_cerr << "Loading Shm. Shmat failed." << endl; 
    throw SharedMemoryException(SharedMemoryExceptionType::SHM_ATTACH_FAILED);
  }
}


}

#define _UNIT_TEST false
#if _UNIT_TEST
// TEST
#include <iostream>

using namespace std;
using namespace lio;
int main () {
  pid_t forkpid;
  cout << "pageSize: " << getpagesize() << endl;
  forkpid = fork();

  try {
    if (forkpid > 0) {
      // Parent
      SharedMemory::Config config;
      config.key = SharedMemory::GenerateKey("SharedMemoryKey");
      config.size = 1024 * 1024 * 2; // 2MB
      config.mode = SharedMemory::Mode::CREATE;

      SharedMemory* shm = new SharedMemory(config);
      std::cout << "Parent Starting..." << std::endl;    
      std::cout << std::hex << "shmAddr: " << shm->GetShmAddress() << std::endl;
      std::cout << std::dec << "shmSize: " << shm->GetShmSize() << std::endl;
      
      char* temp;

      temp = (char *) shm->GetShmAddress();
      for(int j = 0; j < 10; j++) {
        (*temp) = 'C';
        temp++;
        //cout << std::dec<<j << endl;
      }
      (*temp) = '\0';

      shm->SetToDestroyShmOnDelete();
      sleep(4);
      std::cout << "Parent Exiting..." << std::endl;
      delete shm;
    } else if (forkpid == 0) {
      // Child
      sleep(2);
      std::cout << "Child Starting..." << std::endl;    
      SharedMemory::Config config;
      config.key = SharedMemory::GenerateKey("SharedMemoryKey");
      config.size = 1024 * 1024 * 2; // 2MB
      config.mode = SharedMemory::Mode::LOAD;

      SharedMemory* shmC = new SharedMemory(config);
      std::cout << std::hex << shmC->GetShmAddress() << std::endl;
      char* tempa;
      tempa = (char *) shmC->GetShmAddress();
      cout << tempa << endl;
      cout  << "END" << endl;
      //std::cout << "data from shm:" << tempa;
      sleep(1);
      std::cout << "Child Exiting..." << std::endl;
      delete shmC;
    }
  } catch (int e) {
    std::cout << "e: " << e << std::endl;
  }

  
  return 0;
}
#endif
#undef _UNIT_TEST
