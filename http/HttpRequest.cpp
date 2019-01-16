#include "HttpRequest.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpRequest::Exception::exceptionMessages_[] = {
  HTTPREQUEST_EXCEPTION_MESSAGES
};
#undef HTTPREQUEST_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpRequest::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpRequest::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpRequest::ExceptionType
HttpRequest::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


HttpRequest::HttpRequest() :
  buffer(),
  headerSize(0),
  method(http::RequestMethod::UNDEF),
  uri(),
  contentLength(0),
  contentType(http::ContentType::UNDEF),
  language(Language::ENGLISH),
  isKeepAliveSupported(false),
  isGzipSupported(false),
  content(),
  postDataParser(nullptr)
{
  DEBUG_FUNC_START; // Prints out function name in yellow

}

HttpRequest::HttpRequest(DataBlock<char*> buffer) :
  buffer(buffer),
  headerSize(0),
  method(http::RequestMethod::UNDEF),
  uri(),
  contentLength(0),
  contentType(http::ContentType::UNDEF),
  isKeepAliveSupported(false),
  isGzipSupported(false),
  content(),
  postDataParser(nullptr)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
}

HttpRequest::~HttpRequest() {
  DEBUG_FUNC_START;

  if (this->postDataParser != nullptr) {
    DEBUG_cout << "PostDataParser will be deleted!" << endl; 
    delete this->postDataParser;
    DEBUG_cout << "PostDataParser is now deleted!" << endl; 
  } 

}


size_t HttpRequest::GetHeaderSize() const {
  if (headerSize == 0) {
    DEBUG_cerr << "ERROR. Header Size is 0." << endl; 
  } 
  return this->headerSize;
}

http::RequestMethod HttpRequest::GetRequestMethod() const {
  if (this->method == http::RequestMethod::UNDEF) {
    DEBUG_cerr << "RequestMethod empty." << endl; 
  } 
  return this->method;
}

const string& HttpRequest::GetWholeUri() const {
  return this->uri;
}

string HttpRequest::GetUri() const {
  size_t endPos = this->uri.find("?", 0);

  return this->uri.substr(0, endPos);
}

string HttpRequest::GetQueryString(const string& fieldName) const {

  size_t qmPos = this->uri.find("?", 0);
  if (fieldName.empty() || fieldName == "?") {
    // Get All Value after ? (Question mark) in URl.
    if (qmPos != string::npos) {
      return this->uri.substr(qmPos + strlen("?"));
    } 
  } 

  size_t pos = this->uri.find(fieldName + "=", qmPos);
  if (pos == string::npos) {
    // not found
    DEBUG_cout << "FieldName: " << fieldName << " is not found in URI QueryString." << endl; 
    return "";
  } 

  size_t endPos = this->uri.find("&", pos + fieldName.length());
  if (endPos == string::npos) {
    // last field in query string.
    DEBUG_cout << "END REACHED" << endl; 
    endPos = this->uri.length();
  } 

  size_t subStartPos = pos + fieldName.length() + strlen("=");
  size_t fieldValueLength = endPos - subStartPos;
  DEBUG_cout << "FieldValueLength: " << fieldValueLength << endl; 

  string queryStringValue = this->uri.substr(subStartPos, fieldValueLength);
  DEBUG_cout << "queryStringValue: " << queryStringValue << endl; 
  return queryStringValue;
}

const string& HttpRequest::GetHost() const {
  return this->host;
}

const string& HttpRequest::GetUserAgent() const {
  return this->userAgent;
}


size_t HttpRequest::GetContentLength() const {
  return this->contentLength;
}

DataBlock<> HttpRequest::GetContent() const {
  return this->content;
}

http::ContentType HttpRequest::GetContentType() const {
  if (this->contentType == http::ContentType::UNDEF) {
    DEBUG_cerr << "ContentType is undefined." << endl; 
  } 
  return this->contentType;
}

HttpRequest::Language HttpRequest::GetAcceptLanguage() const {
  return this->language;
}

const string& HttpRequest::GetReferer() const {
  return this->referer;
}

map<string, string>& HttpRequest::GetPostData() {
  if (this->postDataParser == nullptr) {
    DEBUG_cerr << "PostData is not available." << endl; 
    throw Exception();
  } 

  this->postDataParser->SetContent(this->content);
  this->postDataParser->ParsePostData();

  return this->postDataParser->GetPostData();
}

map<string, string>& HttpRequest::GetCookies() {
  if (this->cookies.size() <= 0) {
    DEBUG_cout << "No Cookies found." << endl; 
  } 

  return this->cookies;
}

bool HttpRequest::IsKeepAliveSupported() const {
  return this->isKeepAliveSupported;
}

bool HttpRequest::IsGzipSupported() const {
  return this->isGzipSupported;
}


bool HttpRequest::SetBuffer (DataBlock<char*>& buffer) {
  if (buffer.IsNull()) {
    DEBUG_cerr << "Buffer is Null." << endl; 
    return false;
  } 
  this->buffer = buffer;
  return true;
}

bool HttpRequest::SetHeaderSize(const size_t headerSize) {
  if (headerSize < 10) {
    DEBUG_cerr << "HeaderSize is too small." << endl; 
    return false;
  } 
  this->headerSize = headerSize;
  return true;
}

bool HttpRequest::SetRequestMethod(http::RequestMethod method) {
  this->method = method;
  return true;
}

bool HttpRequest::SetRequestMethod(const string& requestMethod) {
  if (requestMethod.length() > 7) {
    DEBUG_cerr << "RequestMethod is too long.." << endl; 
    return false;
  } 

  string method = Util::String::ToUpper(requestMethod);
  if (method == "GET") {
    this->method = http::RequestMethod::GET;
  } else if (method == "POST") {
    this->method = http::RequestMethod::POST;
  } else if (method == "HEAD") {
    this->method = http::RequestMethod::HEAD;
  } else if (method == "PUT") {
    this->method = http::RequestMethod::PUT;
  } else if (method == "DELETE") {
    this->method = http::RequestMethod::DELETE;
  } else if (method == "TRACE") {
    this->method = http::RequestMethod::TRACE;
  } else if (method == "CONNECT") {
    this->method = http::RequestMethod::CONNECT;
  } else {
    DEBUG_cerr << "Unknown request type." << endl; 
    return false;
  }
  return true;
}

bool HttpRequest::SetUri(const string& uri) {
  if (uri.length() > 1024) {
    DEBUG_cerr << "uri length is too long. length: " << uri.length() << endl; 
    return false;
  } 

  if (uri.find("..") != string::npos ||
      uri.find("\\") != string::npos ||
      uri.find("<") != string::npos ||
      uri.find(">") != string::npos ||
      uri.find("#") != string::npos ||
      uri.find("|") != string::npos ||
      uri.find("^") != string::npos ||
      uri.find("~") != string::npos ||
      uri.find("`") != string::npos ||
      uri.find("[") != string::npos ||
      uri.find("]") != string::npos ||
      uri.find("\"") != string::npos) {
    DEBUG_cerr << "Contains dangerous chars." << endl; 
    return false;
  } 

  this->uri = Util::String::UriDecode(uri);
  return true;
}


bool HttpRequest::SetField (const string& fieldName, const string& fieldValue) {
  bool result = false;

  if (fieldName == "Host") {
    result = this->SetHost(fieldValue);
    
  } else if (fieldName == "User-Agent") {
    result = this->SetUserAgent(fieldValue);

  } else if (fieldName == "Content-Length") {
    result = this->SetContentLength(Util::String::To<size_t>(fieldValue));

  } else if (fieldName == "Connection") {
    if (fieldValue.find("Keep-Alive") != string::npos ||
        fieldValue.find("keep-alive") != string::npos) {
      // Found
      this->SetIsKeepAliveSupported(true);
    } 
    result = true;

  } else if (fieldName == "Accept-Encoding") {
    if (fieldValue.find("gzip") != string::npos) {
      this->SetIsGzipSupported(true);
    }
    result = true;

  } else if (fieldName == "Accept-Language") {
    string fieldValueLower = Util::String::ToLower(fieldValue);
    if (fieldValueLower.find("ko-kr") != string::npos) {
      this->SetAcceptLanguae(Language::KOREAN);
    }
    result = true;

  } else if (fieldName == "Referer") {
    result = this->SetReferer(fieldValue);

  } else if (fieldName == "Content-Type") {
    if (this->postDataParser == nullptr) {
      this->postDataParser = new HttpPostDataParser();
    } 
    result = this->postDataParser->SetData(fieldValue);

  } else if (fieldName == "Cookie") {
    result = this->SetCookies(fieldValue);

    
  } 

  return result;
}

bool HttpRequest::SetHost(const string& value) {
  if (value.empty() == true) {
    DEBUG_cerr << "Host is empty." << endl; 
    return false;
  } 
  this->host = value;
  return true;
}

bool HttpRequest::SetUserAgent(const string& value) {
  if (value.empty() == true) {
    DEBUG_cerr << "UserAgent is empty." << endl; 
    return false;
  } 
  this->userAgent = value;
  return true;
}

bool HttpRequest::SetReferer(const string& value) {
  if (value.empty() == true) {
    DEBUG_cerr << "UserAgent is empty." << endl; 
    return false;
  } 
  this->referer = value;
  return true;
}

bool HttpRequest::SetCookies(const string& fieldValue) {
  size_t length = fieldValue.length();

  if (length > 1024 * 64) {
    DEBUG_cerr << "Cookie data is way... too big. length: " << length << endl; 
    return false;

  } else if (length < 3) {
    DEBUG_cerr << "Cookie data is way too small. length: " << length << endl;
    return false;

  } 

  string key;
  string value;
  string temp;

  int count = 0;
  size_t i = 0;
  while (length > i) {

    if (fieldValue[i] == '=') {
      if (temp.length() <= 0) {
        DEBUG_cerr << "Invalid cookie data format." << endl; 
        return false;
      } 
      if (temp[0] == ' ') {
        key = temp.substr(1);
      } else {
        key = temp;
      }
      temp.clear();

    } else if (fieldValue[i] == ';' ||
               fieldValue[i] == '\n' ||
               fieldValue[i] == '\r')
    {
      if (temp.length() <= 0) {
        DEBUG_cerr << "Invalid cookie data format." << endl; 
        return false;
      } 
      value = temp;
      string decodedValue = Util::String::UriDecode(value);
      this->cookies[key] = decodedValue;
      count += 1;
      temp.clear();

    } else {
      temp += fieldValue[i];

    }

    i += 1;
  } 

  if (temp.length() <= 0) {
    DEBUG_cerr << "Invalid cookie data format." << endl; 
    return false;
  } 

  value = std::move(temp);
  string decodedValue = Util::String::UriDecode(value);
  this->cookies[key] = decodedValue;
  count += 1;

  DEBUG_cout << count << " cookies have been parsed!" << endl; 
  return true;
}

bool HttpRequest::SetContentLength(const size_t contentLength) {
  this->contentLength = contentLength;
  return true;
}

bool HttpRequest::SetAcceptLanguae(const Language lang) {
  this->language = lang;
  return true;
}

bool HttpRequest::SetIsKeepAliveSupported(const bool isSupported) {
  this->isKeepAliveSupported = isSupported;
  return true;
}

bool HttpRequest::SetIsGzipSupported(const bool isSupported) {
  this->isGzipSupported = isSupported;
  return true;
}

bool HttpRequest::SetContent(const DataBlock<void*>& content) {
  // What kind of check should be done here??
  this->content = content;
  return true;
}

bool HttpRequest::SetContentType(const http::ContentType contentType) {
  this->contentType = contentType;
  return true;
}

// ==============================

bool HttpRequest::parseRequestMethod() {
  static_assert(true, "I'm not sure to implement this or not yet. Don't use it yet.");

  if (this->buffer.IsNull()) {
    DEBUG_cerr << "Cannot parse Null Buffer." << endl; 
    return false;
  } 

  ssize_t pos = String::Find(" /", (char*) this->buffer.GetObject(), 10);
  if (pos == consts::NOT_FOUND) {
  } 
  
  return true;
}

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpRequest : public HttpRequest {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpRequest, TESTNAME) {
  MockHttpRequest mockHttpRequest;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

