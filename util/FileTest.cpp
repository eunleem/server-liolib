#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "File.hpp"

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
