#include "File.hpp"


#define _UNIT_TEST false
#if _UNIT_TEST
  #include "liolib/TestNew.hpp"
#endif


namespace lio {

// ===== Exception Implementation =====
const char* const
File::Exception::exceptionMessages_[] = {
  FILE_EXCEPTION_MESSAGES
};
#undef FILE_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


File::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
File::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const File::ExceptionType
File::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


File::File() {
  DEBUG_FUNC_START; // Prints out function name in yellow



}

File::~File() {
  DEBUG_FUNC_START;

}

void File::sampleFunc() {
  DEBUG_POINT; // prints out LINE NUMBER and a messaage "POINT REACHED" in GREEN.
}

void File::Test() {
  DEBUG_cout << "WHAT THE" << endl; 
  return;
}

bool File::RetBool(bool val) {
  return val;
}

//File::
//File::
//File::
//File::
}

#if _UNIT_TEST


#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockFile : public File {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(SampleTest, WhatTheHellIsIt) {
  MockFile mfile;

  EXPECT_CALL(mfile, Test())
    .Times(AtLeast(1));
  mfile.Test();
  EXPECT_EQ(true, mfile.RetBool(false));
  //EXPECT_TRUE(mfile.RetBool(false));
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else



#endif

#undef _UNIT_TEST

