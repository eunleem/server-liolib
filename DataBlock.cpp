#include "DataBlock.hpp"

#define _UNIT_TEST false


#if _UNIT_TEST


#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;


TEST(DataBlockTest, InitTest) {
  try {
    DataBlock<int> a;
    ASSERT_THAT(false, true);
  } catch (std::exception& ex) {
  }
  try {
    // Should this be allowed??
    DataBlock<intptr_t> a;
    ASSERT_THAT(false, true);
  } catch (std::exception& ex) {
  }

  try {
    DataBlock<void*> dv;
    DataBlock<string*> ds;
    DataBlock<char*> dc;

  } catch (std::exception& ex){
    ASSERT_THAT(false,true);
  }
}
TEST(DataBlockTest, EqEqCompareTest) {
  DataBlock<> a, b;
  b.index = 10;

  EXPECT_EQ(a == b, false);
}

TEST(DataBlockTest, GetStringValue) {
  DataBlock<string*> a;
  string fullString("This is String that is full!");
  size_t index = 3;
  size_t length = 3;

  a.SetObject(&fullString);
  a.SetIndex(index);
  a.SetLength(length);

  EXPECT_EQ(a.GetValue() == fullString.substr(index, length), true);
}


TEST(virtualTest, VirtualTest) {
  string t = "Hello";
  DataBlock<string*> s (&t,1,1);
  s._PrintInfo();

}

TEST(CharPtrCompare, Fail) {
  char c[20] = "HelloWorld";
  DataBlock<char*> dc(c, 2, 3);
  DataBlock<char*> dca(c, 3, 3);
  EXPECT_EQ(dc == dca, false);
  dca.SetIndex(2);
  dca.SetLength(3);
  EXPECT_EQ(dc == dca, true);

}
TEST(StringPtrCompare, Fail) {
  string s("HelloWorld");
  DataBlock<string*> dc(&s, 2, 3);
  DataBlock<string*> dca(&s, 3, 3);
  EXPECT_EQ(dc == dca, false);
  dca.SetIndex(2);
  dca.SetLength(3);
  EXPECT_EQ(dc == dca, true);

}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

/*
#include <string>

using namespace lio;

using std::string;

int main() {
  DEBUG_cout << "DataBlock UnitTest" << endl; 

  DataBlock<> defDb;
  DataBlock<> cmpDb;
  defDb.index = 10;
  defDb._PrintInfo();

  UnitTest::Test<DataBlock<>>(defDb, cmpDb, "Compare DataBlock", UnitTest::MEANT_TO_BE_FAILED);
  cmpDb.index = 10;
  UnitTest::Test<DataBlock<>>(defDb, cmpDb, "Compare DataBlock");

  DEBUG_cout << "String* DataBlock" << endl; 

  string fullString = "This is Test string. Hello world!";

  DataBlock<string*> partialString;

  partialString.object = &fullString;
  partialString.index = 3;
  partialString.length = 5;
  string retPartString = partialString.GetValue();
  DEBUG_cout << "partString: " << retPartString << endl; 

  UnitTest::Test<string>(retPartString, "s is ", "Compare Partial String");



  UnitTest::ReportTestResult();
  return 0;
}
*/
#endif

#undef _UNIT_TEST
