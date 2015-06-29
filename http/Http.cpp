#include "Http.hpp"

namespace lio {
namespace http {
/*
const string
ContentType::contentTypes_[] = {
  CONTENT_TYPE_NAMES
};
#undef CONTENT_TYPE_NAMES


const string ContentType::GetContentTypeStr(const Type type){
  return contentTypes_[static_cast<unsigned int>(type)];
}
*/

}
}


#define _UNIT_TEST false
#if _UNIT_TEST
#include "liolib/Test.hpp"

#include <iostream>
#include <string>

using namespace lio;
using std::string;
using std::cout;
using std::endl;
using std::cerr;

int main() {
  string type = http::ContentTypes[(int)http::ContentType::HTML];

  UnitTest::Test<string>(type, "text/html", "HTML");
  UnitTest::ReportTestResult();
  
  /*UnitTest::Test(dba, dbb, "DBAB");

  string value = strMap.GetValueByKey("A");

  UnitTest::Test<string>(value, "Hello", "GetValueByKeyA");

  value = strMap.GetValueByKey("B");

  DataBlock returned = strMap.GetDataBlockByKey("C");

  UnitTest::Test<string>(value, "content", "GetValueByKeyB");
  
  UnitTest::Test<DataBlock>(returned, DataBlock(6, 0), "GetDataBlockByKeyC");

  UnitTest::ReportTestResult();
*/

  return 0;
}
#endif
#undef _UNIT_TEST



