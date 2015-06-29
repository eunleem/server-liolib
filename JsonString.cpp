#include "JsonString.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

JsonString::JsonString(size_t reserveSize) {
  DEBUG_FUNC_START; // Prints out function name in yellow
}

JsonString::~JsonString() {
  DEBUG_FUNC_START;

}


//JsonString::
//JsonString::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockJsonString : public JsonString {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(JsonString, TESTNAME) {
  MockJsonString mockJsonString;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

