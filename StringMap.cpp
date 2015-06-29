#include "StringMap.hpp"

#define _UNIT_TEST false



namespace lio {
// ===== Exception Implementation =====
const char* const
StringMapException::exceptionMessages_[] = {
  STRINGMAP_EXCEPTION_MESSAGES
};
#undef STRINGMAP_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


StringMapException::StringMapException(StringMapExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
StringMapException::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const StringMapExceptionType
StringMapException::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====

}

#if _UNIT_TEST

#include <iostream>
#include <string>

#include <map>


#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::pair;

string testContent = "Hello this is the test content and this is such an awesome test!";

TEST(StringMapTest, InitTest) {
  StringMap<string> strMap (&testContent);
  const string* content = strMap.GetContent();

  EXPECT_EQ(content == &testContent, true);
}

TEST(StringMapTest, AddKeyValue) {
  unsigned int location = static_cast<unsigned int> (content->find("Hello"));
  unsigned int length = static_cast<unsigned int>(sizeof("Hello") - 1);

  DataBlock<string*> keyData (location, length);
  strMap.AddKeyValue("A", keyData);
  strMap.AddKeyValue();

  EXPECT_EQ(content == &testContent, true);
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}


/*
using namespace lio;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::pair;

int main() {
  string testContent = "Hello this is the test content and this is such an awesome test!";

  StringMap<string> strMap (&testContent);

  const string* content = strMap.GetContent();
  DEBUG_cout << "orgContentLength: " << content->length() << endl; 

  unsigned int location = static_cast<unsigned int> (content->find("Hello"));
  unsigned int length = static_cast<unsigned int>(sizeof("Hello") - 1);

  //string keyA = "A";
  DataBlock keyData (location, length);
  strMap.AddKeyValue("A", keyData);
  

  location = static_cast<unsigned int> (content->find("content"));
  length = static_cast<unsigned int>(sizeof("content") - 1);

  //string keyB = "B";
  DataBlock keyDataB (location, length);
  strMap.AddKeyValue("B", keyDataB);

  strMap.AddKeyValue("C", DataBlock(6, 0));

  
  DataBlock dba (2,6);
  DataBlock dbb (2,5);

  const bool MEANT_TO_BE_FAILED = true;
  UnitTest::Test<DataBlock>(dba, dbb, "DBAB MUST FAIL", MEANT_TO_BE_FAILED);

  string value = strMap.GetValueByKey("A");

  UnitTest::Test<string>(value, "Hello", "GetValueByKeyA");

  value = strMap.GetValueByKey("B");

  DataBlock returned = strMap.GetDataBlockByKey("C");

  UnitTest::Test<string>(value, "content", "GetValueByKeyB");
  
  UnitTest::Test<DataBlock>(returned, DataBlock(6, 0), "GetDataBlockByKeyC");
  DEBUG_cout << "orgContentLength: " << content->length() << endl; 

  int tokenizedCount = strMap.Tokenize(" ");
  DEBUG_cout << "Token: " << tokenizedCount << endl; 
  UnitTest::Test<int>(tokenizedCount, 13, "Tokenize");

  DataBlock section1 = strMap.GetSectionByIndex(0);
  DEBUG_cout << "SectionStr:" << content->substr(section1.relativePosition, section1.length) << "END" << endl;

  UnitTest::Test<DataBlock>(section1, DataBlock((unsigned int) 0, 5), "GetSectionByIndex1");


  string appendingStr = "dddddddd";
  strMap.AppendContent(&appendingStr);
  UnitTest::Test<char>(*content->rbegin(), 'd', "Append String Test");



  UnitTest::ReportTestResult();


  return 0;
}
*/
#endif
#undef _UNIT_TEST


