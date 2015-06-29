#include "Semaphore.hpp"

namespace lio {


using Util::GenerateUniqueKey;

// ===== Exception Implementation =====
const char* const Semaphore::Exception::exceptionMessages_[] = {
  SEMAPHORE_EXCEPTION_MESSAGES
};
#undef SEMAPHORE_EXCEPTION_MESSAGES // no longer used #define is better to be undef.

Semaphore::Exception::Exception(Semaphore::ExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char* Semaphore::Exception::what() const throw() {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Semaphore::ExceptionType Semaphore::Exception::type() const throw() {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====



Semaphore::Semaphore(Config& config)
  : config_(config) {

  DEBUG_FUNC_START;

  // #TODO: Check max number of semaphores.
  if (this->config_.semCount < SEMMSL) {
    DEBUG_cerr << "semCount exceeds limit." << endl; 
    throw Exception(ExceptionType::SEM_INIT_FAILED);
  }

  this->isSetToDestroySem_ = false;
  
  string keyPath = this->config_.semKeyDir + this->config_.semKeyName;

  this->semKey_ = GenerateUniqueKey(keyPath);
  
  switch (this->config_.semMode) {
    case Mode::AUTO:
      try {
        this->createSem();
      } catch(Exception& ex) {
        if (ex.type() == ExceptionType::SEM_EXISTS) {
          this->loadSem();
        } else {
          throw Exception(ex.type());
        }
      }
      break;
    case Mode::CREATE:
      this->createSem();
      break;
    case Mode::LOAD:
      this->loadSem();
      break;
    default:
      DEBUG_cerr << "Invalid Semaphore Mode." << endl; 
      throw Exception(ExceptionType::SEM_INIT_FAILED);
      break;
  }
}


Semaphore::~Semaphore() {
  DEBUG_FUNC_START;
  //#TODO: Gotta release all the Sem?? Not sure. Nothing will be done yet.
  if (this->isSetToDestroySem_) {
    DEBUG_cout << "Semaphore is set to be destroyed!\n";
    union semun arg;
    /* remove it: */
    if (semctl(this->semId_, 0, IPC_RMID, arg) == -1) {
      // ERROR
      // #TODO: check if this exception must be handled.
      throw Semaphore::Exception(Semaphore::ExceptionType::SEM_UNSET_FAILED);
    }
  }
}

bool Semaphore::Lock(int semNum) {

  const int ERROR = -1;

  if (semNum >= this->config_.semCount) {
    //ERROR
    DEBUG_cerr << "Lock: Sem Number out of range." << endl; 
    throw Exception(ExceptionType::GENERAL);
  }
  struct sembuf sb;
  
  sb.sem_num = semNum;
  sb.sem_op = -1; // LOCK
  sb.sem_flg = SEM_UNDO;
  
  int semOpResult = 0;
  semOpResult = semop (this->semId_, &sb, 1);
  if (semOpResult == ERROR) { // Error!
    DEBUG_cerr << "semop has failed. Errno: " << errno << endl; 
    return false;
  }
  
  return true;
}

bool Semaphore::Release(int semNum) {

  const int ERROR = -1;

  if (semNum >= this->config_.semCount) {
    //ERROR
    DEBUG_cerr << "Release: Sem Number out of range." << endl; 
    throw Exception(ExceptionType::GENERAL);
  }
  struct sembuf sb;
  
  sb.sem_num = semNum;
  sb.sem_op = 1; // RELEASE
  sb.sem_flg = SEM_UNDO;
  
  int semOpResult = 0;
  semOpResult = semop (this->semId_, &sb, 1);
  if (semOpResult == ERROR) { // Error!
    DEBUG_cerr << "semop has failed. Errno: " << errno << endl; 
    return false;
  }
  
  return true;  
  
}

void Semaphore::SetToDestroySemOnDelete() {
  isSetToDestroySem_ = true;
}

// #DEPRECATED
int Semaphore::DestroySem() {
  isSetToDestroySem_ = true;
  return 0;
}


void Semaphore::createSem() {
  struct sembuf sb;

  // #TODO:Check if all the arguments are not empty.
  
  semId_ = semget(this->semKey_,
                  this->config_.semCount,
                  this->config_.semPermission | IPC_CREAT | IPC_EXCL);
  
  if (this->semId_ >= 0) { // Succesfully Created Semaphore
    sb.sem_op = 1;
    sb.sem_flg = 0;
    for (sb.sem_num = 0; sb.sem_num < this->config_.semCount; sb.sem_num++) {
      int semOpResult = 0;
      semOpResult = semop (this->semId_, &sb, 1);
      if (semOpResult == -1) { // ERROR with SemOp
        semctl(this->semId_, 0, IPC_RMID);
        DEBUG_cerr << "semop has failed." << endl;
        throw Semaphore::Exception(Semaphore::ExceptionType::SEM_INIT_FAILED);
      }
    }
  } else if (errno == EEXIST) { // Semaphore already exists
    DEBUG_cerr << "Semaphore: SEM already Exists." << endl; 
    throw Semaphore::Exception(Semaphore::ExceptionType::SEM_EXISTS);
  } else {
    DEBUG_cerr << "Semaphore: SEM Initialization failed." << endl; 
    throw Semaphore::Exception(Semaphore::ExceptionType::SEM_INIT_FAILED);
  }
}

void Semaphore::loadSem() {
  //struct sembuf sb;

  // #TODO:Check if all the arguments are not empty.
  
  semId_ = semget(this->semKey_, this->config_.semCount, 0);
  
  if (this->semId_ >= 0) { // Succesfully Loaded Existing Semaphore
    return;
  } else {
    DEBUG_cerr << "Failed to load Semaphore." << endl; 
    throw Semaphore::Exception(Semaphore::ExceptionType::SEM_INIT_FAILED);
  }
}


}

#define _UNIT_TEST true
#if _UNIT_TEST

// TEST
#include <iostream>
#include <unistd.h>

using std::cout;
using std::endl;

using lio::Semaphore;
void Semaphore::_PrintKey() {
  cout << "SemKey: " << semKey_ << endl;
  
}

void Semaphore::_PrintSemId() {
  cout << "SemId: " << semId_ << endl;
}

int main () {
  Semaphore::Config config;

  Semaphore* sem = new Semaphore(config);
  
  sem->_PrintKey();
  sem->_PrintSemId();
  cout << "LOCKING" << endl;
  sem->Lock(0);
  cout << "LOCKED" << endl;
  cout << getpid() << endl;
  sleep(2);
  
  sem->Release(0);
  cout << "RELEASED" << endl;
  
  delete sem;
}
#endif

#undef _UNIT_TEST
