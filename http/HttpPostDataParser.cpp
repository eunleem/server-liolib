#include "HttpPostDataParser.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpPostDataParser::Exception::exceptionMessages_[] = {
  HTTPPOSTDATAPARSER_EXCEPTION_MESSAGES
};
#undef HTTPPOSTDATAPARSER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpPostDataParser::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpPostDataParser::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpPostDataParser::ExceptionType
HttpPostDataParser::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


HttpPostDataParser::HttpPostDataParser() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpPostDataParser::~HttpPostDataParser() {
  DEBUG_FUNC_START;

}

bool HttpPostDataParser::SetContent(DataBlock<> content) {
  if (content.IsNull()) {
    DEBUG_cerr << "Content is NULL!" << endl; 
    return false;
  } 
  this->content = content;
  return true;
}

bool HttpPostDataParser::SetData(const string& fieldValue) {
  http::ContentType type = http::ContentType::UNDEF;

  size_t pos = fieldValue.find(http::ContentTypeString[(int) http::ContentType::FORMDATA]);
  if (pos != string::npos) {
    type = http::ContentType::FORMDATA;
  } else {
    pos = fieldValue.find(http::ContentTypeString[(int) http::ContentType::FORMDATA_MULTIPART]);
    if (pos != string::npos) {
      type = http::ContentType::FORMDATA_MULTIPART;
    } 
  }

  if (type == http::ContentType::UNDEF) {
    DEBUG_cerr << "Content-Type is undefined. fieldValue" << fieldValue << endl; 
    return false;
  }

  this->contentType = type;

  if (this->contentType == http::ContentType::FORMDATA_MULTIPART) {
    DEBUG_cerr << "Form multipart is not supported yet." << endl; 
    return false;

    pos = fieldValue.find("boundary=");
    if (pos != string::npos) {

    } else {
      DEBUG_cerr << "WARN: Boundary value might be required. but it's not provided." << endl; 
    }

  } else if (this->contentType == http::ContentType::FORMDATA) {

  }
  DEBUG_cout << "SetData returned true." << endl; 

  return true;
}

bool HttpPostDataParser::ParsePostData() {
  if (this->content.IsNull()) {
    DEBUG_cerr << "Content is Null." << endl; 
    return false;
  } 

  if (this->contentType == http::ContentType::UNDEF) {
    DEBUG_cerr << "ContentType is Undefined." << endl; 
    return false;
  } 

  if (this->contentType == http::ContentType::FORMDATA) {
    // FORMDATA 
    this->parse();
    return true;
    
  } else if (this->contentType == http::ContentType::FORMDATA_MULTIPART) {
    // FORMDATA MULTIPART
    static_assert(true, "Not yet implemented.");
    return false;
  } 
  
  return true;
}

string HttpPostDataParser::getBoundary(const string& fieldValue) {
  static_assert(true, "Not yet implemented.");
  return "";

}

map<string, string>& HttpPostDataParser::GetPostData() {
  return this->postData;
}

void HttpPostDataParser::parse() {
  char* ptr = (char*) this->content.GetObject();
  size_t length = this->content.GetLength();

  if (length > 1024 * 256) {
    DEBUG_cerr << "Posted form data is way... too big. length: " << length << endl; 
    return;
  } else if (length < 2) {
    DEBUG_cerr << "Posted form data is way too small. length: " << length << endl;
    return;
  } 

  string key;
  string value;
  string temp;

  size_t i = 0;
  while (length > i) {

    if ((*ptr) == '=') {
      if (temp.length() <= 0) {
        DEBUG_cerr << "Invalid posted form data format." << endl; 
        return;
      } 
      key = temp;
      temp.clear();

    } else if ((*ptr) == '&' ||
               (*ptr) == '\n' ||
               (*ptr) == '\r')
    {
      if (temp.length() <= 0) {
        DEBUG_cerr << "Invalid posted form data format." << endl; 
        return;
      } 
      value = temp;
      string decodedValue = Util::String::UriDecode(value);
      this->postData[key] = decodedValue;
      temp.clear();

    } else {
      temp += (*ptr);

    }

    i += 1;
    ptr += 1;
  } 
  if (temp.length() <= 0) {
    DEBUG_cerr << "Invalid posted form data format." << endl; 
    return;
  } 
  value = temp;
  string decodedValue = Util::String::UriDecode(value);
  this->postData[key] = decodedValue;

  size_t count = this->postData.size();

  DEBUG_cout << count << " fields have been parsed!" << endl; 
}


//HttpPostDataParser::




}


#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpPostDataParser : public HttpPostDataParser {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpPostDataParser, TESTNAME) {
  MockHttpPostDataParser mockHttpPostDataParser;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

