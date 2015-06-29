#include "MemoryPoolManager.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
MemoryPoolManager::Exception::exceptionMessages_[] = {
  MEMORYPOOLMANAGER_EXCEPTION_MESSAGES
};
#undef MEMORYPOOLMANAGER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


MemoryPoolManager::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
MemoryPoolManager::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const MemoryPoolManager::ExceptionType
MemoryPoolManager::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


MemoryPoolManager::MemoryPoolManager(Config& config) 
  : config_(config)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

  this->poolList_.reserve(4);

  
  this->addPool(config.defaultMpConfig);


}

MemoryPoolManager::~MemoryPoolManager() {
  DEBUG_FUNC_START;

  // Delete 
  for (auto& mp : this->poolList_) {
    delete mp;
  } 
}

void* const MemoryPoolManager::Mpalloc(size_t allocSize) {

  for (auto& mp : this->poolList_) {
    size_t freeSize = mp->GetFreeSize();
    if (freeSize > allocSize) {
      
    }
  } 

  return nullptr;
}

size_t MemoryPoolManager::Mpfree (const void* freePtr) {

  return 0;
}

//MemoryPoolManager::
//MemoryPoolManager::
//MemoryPoolManager::
void MemoryPoolManager::addPool(MemoryPool::Config& poolConfig) {
  MemoryPool* newPool = new MemoryPool(poolConfig);
  this->poolList_.push_back(newPool);


}

}

#define _UNIT_TEST false
#if _UNIT_TEST

//#include "Test.hpp"
#include "liolib/Test.hpp"

#include <iostream>

using namespace lio;

int main() {
  return 0;
}
#endif
#undef _UNIT_TEST

