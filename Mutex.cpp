#include "Mutex.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
Mutex::Exception::exceptionMessages_[] = {
  MUTEX_EXCEPTION_MESSAGES
};
#undef MUTEX_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Mutex::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Mutex::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Mutex::ExceptionType
Mutex::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====

ddf


Mutex::Mutex() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

Mutex::~Mutex() {
  DEBUG_FUNC_START;

}

void Mutex::sampleFunc() {
  DEBUG_POINT; // prints out LINE NUMBER and a messaage "POINT REACHED" in GREEN.
}

//Mutex::

}

#define _UNIT_TEST false
#if _UNIT_TEST

#include "Test.hpp"
//#include "liolib/Test.hpp"

#include <iostream>

using namespace lio;

int main() {
  return 0;
}
#endif
#undef _UNIT_TEST

