#include "HttpRequestParser.hpp"

#define _UNIT_TEST false

#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation =====
const char* const
HttpRequestParser::Exception::exceptionMessages_[] = {
  HTTPREQUESTPARSER_EXCEPTION_MESSAGES
};
#undef HTTPREQUESTPARSER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpRequestParser::Exception::Exception(HttpRequestParser::ExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
HttpRequestParser::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpRequestParser::ExceptionType
HttpRequestParser::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


const size_t HttpRequestParser::URI_LENGTH_MAX = 128;
const size_t HttpRequestParser::FIELD_LENGTH_MAX = 1024;

HttpRequestParser::HttpRequestParser(const string* requestRawStr) :
  requestStrMap_(const_cast<string*>(requestRawStr)),
  bodyLength_(0)
{
  DEBUG_FUNC_START;

  // #TODO: It does too much work in constructor. Change Design.
  this->parseEssentialFields(requestRawStr);
}

HttpRequestParser::~HttpRequestParser() {
  DEBUG_FUNC_START;
}

bool HttpRequestParser::parseEssentialFields(const string* requestStr) {
  DEBUG_FUNC_START;

  //DEBUG_cout << "STR: " << (*requestStr) << endl;
  size_t currentPosition = 0;

  currentPosition = this->findRequestMethod(requestStr);
  currentPosition = this->findUri(requestStr, currentPosition);
  
  // GET BODY CONTENT
  // Get only when the request type is POST
  if (this->requestMethod_ == http::RequestMethod::POST) {

    size_t headerEndPosition = requestStr->find("\r\n\r\n", currentPosition);
    if (headerEndPosition == consts::STRING_NOT_FOUND) {
      // Invalid Format
      DEBUG_cerr << "Cannot find the end of the header. " << endl;
      throw Exception(ExceptionType::BAD_REQUEST);
    } else {
      this->contentBodyStartPosition_ = headerEndPosition + sizeof("\r\n\r\n") - 1;
    }

    size_t contentLengthFieldPosition = this->findHeaderField(requestStr,
                                                              "Content-Length",
                                                              headerEndPosition);
    if (contentLengthFieldPosition == consts::STRING_NOT_FOUND) {
      DEBUG_cerr << "Cannot find the end of Content-Length field. " << endl;
      throw Exception(ExceptionType::CONTENT_LENGTH_REQUIRED); 
    }
    

    size_t contentTypeFieldPosition = this->findHeaderField(requestStr,
                                                            "Content-Type",
                                                            headerEndPosition);
    if (contentTypeFieldPosition == consts::STRING_NOT_FOUND) {
      // #TODO: Set Content-Type to Text?
      DEBUG_cerr << "Content Type is required for post request." << endl;
      throw Exception(ExceptionType::BAD_REQUEST);
    }
    
    this->findContentBody (requestStr, headerEndPosition);
  }

  return true;
}


size_t HttpRequestParser::findRequestMethod (const string* requestStr) {
  const int MATCH = 0;
  //const int leadingCharToSkipMax = 5;
  size_t currentPosition = 0;

  while (requestStr->at(currentPosition) == ' ' ||
         requestStr->at(currentPosition) == '\n' ||
         requestStr->at(currentPosition) == '\r')
  {
    currentPosition += 1;
  } 

  // GET REQUEST METHOD
  string method = Util::String::ToUpper(requestStr->substr(currentPosition, 7));

  if (method.compare(0, 3, "GET") == MATCH) {
    this->requestMethod_ = http::RequestMethod::GET;
    currentPosition += sizeof("GET") - 1;
  } else if (method.compare(0, 4, "POST") == MATCH) {
    this->requestMethod_ = http::RequestMethod::POST;
    currentPosition += sizeof("POST") - 1;
  } else if (method.compare(0, 4, "HEAD") == MATCH) {
    this->requestMethod_ = http::RequestMethod::HEAD;
    currentPosition += sizeof("HEAD") - 1;
  } else if (method.compare(0, 3, "PUT") == MATCH) {
    this->requestMethod_ = http::RequestMethod::PUT;
    currentPosition += sizeof("PUT") - 1;
  } else if (method.compare(0, 6, "DELETE") == MATCH) {
    this->requestMethod_ = http::RequestMethod::DELETE;
    currentPosition += sizeof("DELETE") - 1;
  } else if (method.compare(0, 5, "TRACE") == MATCH) {
    this->requestMethod_ = http::RequestMethod::TRACE;
    currentPosition += sizeof("TRACE") - 1;
  } else if (method.compare(0, 7, "CONNECT") == MATCH) {
    this->requestMethod_ = http::RequestMethod::CONNECT;
    currentPosition += sizeof("CONNECT") - 1;
  } else {
    // ERROR
    DEBUG_cerr << "Could not get Request method" << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }

  //DEBUG_cout << "RequestMethod: " << (int) this->requestMethod_ << endl;

  return currentPosition;
}

size_t HttpRequestParser::findUri (const string* requestStr, size_t currentPosition) {
  // GET URI
  size_t uriStartPosition = requestStr->find(" /", currentPosition) + 1;
  if (uriStartPosition == consts::STRING_NOT_FOUND ||
      uriStartPosition > 20) { // if uriStartPosition is larger than n, something is likely to be wrong. So STOP IT!
    
    //DEBUG_cerr << "D" << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }
  currentPosition = uriStartPosition;

  size_t uriEndPosition = requestStr->find(" HTTP/1", currentPosition);
  if (uriEndPosition == consts::STRING_NOT_FOUND) {
    DEBUG_cerr << "Could not find end of URI." << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }
  currentPosition = uriEndPosition;

  size_t uriLength = uriEndPosition - uriStartPosition;
  if (uriLength > URI_LENGTH_MAX) {
    DEBUG_cerr << "Uri is too long." << endl;
    throw Exception(ExceptionType::OVERSIZE);
  }

  string uri = requestStr->substr(uriStartPosition, uriLength);
  if (uri.find("..") != consts::STRING_NOT_FOUND ||
      uri.find("\\") != consts::STRING_NOT_FOUND) {
    // URI should not contain .. char to prevent it accessing unintended files.
    DEBUG_cerr << "Uri contains dangerous chars." << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }

  this->requestStrMap_.AddKeyValue("Uri", uriStartPosition, uriLength);

  DEBUG_cout << "URI: " << requestStr->substr(uriStartPosition, uriLength) << endl;

  return currentPosition;
}

size_t HttpRequestParser::findAcceptEncoding(const string* requestStr) {
  string fieldToFind = "Accept-Encoding: ";
  size_t fieldPosition = requestStr->rfind(fieldToFind, this->contentBodyStartPosition_ - 4);
  if (fieldPosition == consts::STRING_NOT_FOUND) {
    return consts::STRING_NOT_FOUND;
  }

  fieldPosition += fieldToFind.length() - 1;
  size_t fieldEndPosition = requestStr->find("\r\n", fieldPosition);
  if (fieldEndPosition == consts::STRING_NOT_FOUND) {
    DEBUG_cerr << "Could not find the end of header field." << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }

  size_t fieldLength = fieldEndPosition - fieldPosition;
  if (fieldLength > this->FIELD_LENGTH_MAX) {
    DEBUG_cerr << "Header field value is too long." << endl;
    throw Exception(ExceptionType::OVERSIZE);
  }
  //DEBUG_cout << "findHeaderField: headerField: " << headerField << endl;
  //DEBUG_cout << "findHeaderField: headerFieldLength: " << fieldLength << endl;
  this->requestStrMap_.AddKeyValue("Accept-Encoding", fieldPosition, fieldLength);
  
  return fieldPosition;
}

size_t HttpRequestParser::findConnection(const string* requestStr) {
  string fieldToFind = "Connection: ";
  size_t fieldPosition = requestStr->rfind(fieldToFind, this->contentBodyStartPosition_ - 4);
  if (fieldPosition == consts::STRING_NOT_FOUND) {
    return consts::STRING_NOT_FOUND;
  }

  fieldPosition += fieldToFind.length() - 1;
  size_t fieldEndPosition = requestStr->find("\r\n", fieldPosition);
  if (fieldEndPosition == consts::STRING_NOT_FOUND) {
    DEBUG_cerr << "Could not find the end of header field." << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }

  size_t fieldLength = fieldEndPosition - fieldPosition;
  if (fieldLength > this->FIELD_LENGTH_MAX) {
    DEBUG_cerr << "Header field value is too long." << endl;
    throw Exception(ExceptionType::OVERSIZE);
  }
  //DEBUG_cout << "findHeaderField: headerField: " << headerField << endl;
  //DEBUG_cout << "findHeaderField: headerFieldLength: " << fieldLength << endl;
  this->requestStrMap_.AddKeyValue("Connection", fieldPosition, fieldLength);
  
  return fieldPosition;
}


size_t HttpRequestParser::findHeaderField(const string* requestStr, string headerField, const size_t headerEndPosition) {
    
  string fieldToFind = headerField;
  if (fieldToFind.find(": ") == consts::STRING_NOT_FOUND) {
    fieldToFind += ": ";
  } else {
    headerField = headerField.substr(headerField.length() - 3);
  }

  size_t fieldPosition = requestStr->rfind(fieldToFind, headerEndPosition);
  if (fieldPosition == consts::STRING_NOT_FOUND) {
    // return Wrong Request
    //throw Exception(ExceptionType::CONTENT_LENGTH_REQUIRED);
    return consts::STRING_NOT_FOUND;
  }

  fieldPosition += fieldToFind.length() - 1;
  size_t fieldEndPosition = requestStr->find("\r\n", fieldPosition);
  if (fieldEndPosition == consts::STRING_NOT_FOUND) {
    DEBUG_cerr << "Could not find the end of header field." << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }

  size_t fieldLength = fieldEndPosition - fieldPosition;
  if (fieldLength > this->FIELD_LENGTH_MAX) {
    DEBUG_cerr << "Header field value is too long." << endl;
    throw Exception(ExceptionType::OVERSIZE);
  }
  //DEBUG_cout << "findHeaderField: headerField: " << headerField << endl;
  //DEBUG_cout << "findHeaderField: headerFieldLength: " << fieldLength << endl;
  this->requestStrMap_.AddKeyValue(headerField, fieldPosition, fieldLength);
  
  return fieldPosition;
}

bool HttpRequestParser::findContentBody (const string* requestStr, const size_t headerEndPosition) {
  size_t bodyStartPosition = headerEndPosition + sizeof("\r\n\r\n") - 1;
  string bodyLengthStr = this->requestStrMap_.GetValueByKey("Content-Length");
  if (bodyLengthStr == "") {
    DEBUG_cerr << "Content-Length is empty." << endl;
    throw Exception(ExceptionType::BAD_REQUEST);
  }

  DEBUG_cout << "bodyLengthStr: " << bodyLengthStr << endl;

  size_t bodyLength = 0;
  bodyLength = std::stoul(bodyLengthStr);
  
  DEBUG_cout << "bodyLength: " << bodyLength << endl;

  this->requestStrMap_.AddKeyValue("ContentBody",
                                   DataBlock<string*>(requestStr,
                                                      bodyStartPosition, bodyLength));
  this->bodyLength_ = bodyLength;

  return true;
}


//***********************************************************

http::RequestMethod HttpRequestParser::GetRequestMethod() const {
  return this->requestMethod_;
}

string HttpRequestParser::GetUri() const {
  try {
    return this->requestStrMap_.GetValueByKey("Uri");
  } catch (StringMapException& ex) {
    switch(ex.type()) {
     case StringMapExceptionType::NO_VALUE_FOUND:
      return "";
      break;
     default:
      break;
    }
  }
  return "";
}

string HttpRequestParser::GetHost() const {
  try {
    return this->requestStrMap_.GetValueByKey("Host");
  } catch (StringMapException& ex) {
    switch(ex.type()) {
     case StringMapExceptionType::NO_VALUE_FOUND:
      {
        const string* requestStr = this->requestStrMap_.GetContent();

        DataBlock<string*> hostValueRange = this->getFieldValue("Host");
        if (hostValueRange.IsNull() == true) {
          return "";
        }

        return requestStr->substr(hostValueRange.index, hostValueRange.length);
      }
      break;
     default:
      break;
    }
  }


  return "";
}

string HttpRequestParser::GetUserAgent() const {
  const string* requestStr = this->requestStrMap_.GetContent();

  DataBlock<string*> userAgentValueRange = this->getFieldValue("User-Agent");

  return requestStr->substr(userAgentValueRange.index, userAgentValueRange.length);
}

DataBlock<string*> HttpRequestParser::GetBodyContent() const {
  try {
    return this->requestStrMap_.GetDataBlockByKey("ContentBody");
  } catch (StringMapException& ex) {
    switch(ex.type()) {
     case StringMapExceptionType::NO_VALUE_FOUND:
      return DataBlock<string*>();
      break;
     default:
      break;
    }
  }
  return DataBlock<string*>();
}

string HttpRequestParser::GetContentType() const {
  try {
    return this->requestStrMap_.GetValueByKey("Content-Type");
  } catch (StringMapException& e) {
    switch(e.type()) {
     case StringMapExceptionType::NO_VALUE_FOUND:
      return "";
      break;
     default:
      break;
    }
  }
  return "";
}


// #TODO: REFACTOR! I don't like this ..
bool HttpRequestParser::IsGzipSupported() {
  try {
    string encodings = this->requestStrMap_.GetValueByKey("Accept-Encoding");
    Util::String::ToUpperFly(encodings);
    size_t gzipPosition = encodings.find("GZIP");
    if (gzipPosition == consts::STRING_NOT_FOUND) {
      // not found
      return false;
    } else {
      // Found! good!
      return true;
    }
  } catch (StringMapException& e) {
    switch(e.type()) {
     case StringMapExceptionType::NO_VALUE_FOUND:
      {
        const string* requestStr = this->requestStrMap_.GetContent();
        size_t acceptEncodingPosition = this->findAcceptEncoding(requestStr);
        if (acceptEncodingPosition == consts::STRING_NOT_FOUND) {
          return false;
        } else {
          string encodings = this->requestStrMap_.GetValueByKey("Accept-Encoding");
          Util::String::ToUpperFly(encodings);
          size_t gzipPosition = encodings.find("GZIP");
          if (gzipPosition == consts::STRING_NOT_FOUND) {
            // not found
            return false;
          } else {
            // Found! good!
            return true;
          }
        }
      }
      return false;
      break;
     default:
      DEBUG_cerr << e.what() << endl;
      break;
    }
  }
  return false;
  
}

bool HttpRequestParser::IsKeepAliveSupported() {
  try {
    string encodings = this->requestStrMap_.GetValueByKey("Connection");
    Util::String::ToUpperFly(encodings);
    size_t keepAlivePosition = encodings.find("KEEP-ALIVE");
    if (keepAlivePosition == consts::STRING_NOT_FOUND) {
      // not found
      return false;
    } else {
      // Found! good!
      return true;
    }
  } catch (StringMapException& e) {
    switch(e.type()) {
     case StringMapExceptionType::NO_VALUE_FOUND:
      {
        const string* requestStr = this->requestStrMap_.GetContent();
        size_t acceptEncodingPosition = this->findConnection(requestStr);
        if (acceptEncodingPosition == consts::STRING_NOT_FOUND) {
          return false;
        } else {
          string encodings = this->requestStrMap_.GetValueByKey("Connection");
          Util::String::ToUpperFly(encodings);
          size_t gzipPosition = encodings.find("KEEP-ALIVE");
          if (gzipPosition == consts::STRING_NOT_FOUND) {
            // not found
            return false;
          } else {
            // Found! good!
            return true;
          }
        }
      }
      return false;
      break;
     default:
      DEBUG_cerr << e.what() << endl;
      break;
    }
  }
  return false;
  
}

// *********************************

DataBlock<string*> HttpRequestParser::getFieldValue(const string& fieldName) const {
  const string* requestStr = this->requestStrMap_.GetContent();

  size_t headerEndPosition;

  if (this->contentBodyStartPosition_ == consts::STRING_NOT_FOUND) {
    headerEndPosition = requestStr->find("\r\n\r\n");
    if (headerEndPosition == consts::STRING_NOT_FOUND) {
      DEBUG_cerr << "getFieldValue failed to find the end of the header." << endl;
      throw Exception(ExceptionType::BAD_REQUEST);
    }
  } else {
    headerEndPosition =  this->contentBodyStartPosition_;
  }
  

  string strToFind = fieldName + ": ";

  size_t fieldLocation = requestStr->rfind(strToFind, headerEndPosition);
  if (fieldLocation == consts::STRING_NOT_FOUND) {
    // NOTHIGN FOUND.
    return DataBlock<string*>();
  }

  size_t fieldValueLocation = fieldLocation + strToFind.length();
  size_t fieldValueLength = requestStr->find("\r\n", fieldValueLocation) - fieldValueLocation;
  if (fieldValueLength == consts::STRING_NOT_FOUND) {
    DEBUG_cerr << "getFieldValue failed to find the end of the field value." << endl;
    throw Exception(ExceptionType::BAD_FIELD);
  }

  if (fieldValueLength > this->FIELD_LENGTH_MAX) {
    DEBUG_cerr << "getFieldValue field length too long:." << endl;
    throw Exception(ExceptionType::BAD_FIELD);
  }

  return DataBlock<string*>(requestStr, fieldValueLocation, fieldValueLength);
}


}

#if _UNIT_TEST

#include <string>

using namespace lio;

int main() {
  string testReqStr = "GET /index.html HTTP/1.1\r\nHost: www.lifeino.com\r\nConnection: close\r\nUser-Agent: Firefox 24 Linux(x86) Blah!\r\nOtherField: Hello\r\n\r\n";


  UnitTest::PerformanceTestStart("All Tests");
  HttpRequestParser reqParser (&testReqStr);


  string uri = reqParser.GetUri();
  DEBUG_cout << "Uri: " << uri << endl; 
  UnitTest::Test<string>(uri, "/index.html", "URI TEST");

  http::RequestMethod method = reqParser.GetRequestMethod();
  UnitTest::Test<http::RequestMethod>(method, http::RequestMethod::GET , "METHOD TEST");

  string host = reqParser.GetHost();
  UnitTest::Test<string>(host, "www.lifeino.com", "HOST TEST");

  string userAgent = reqParser.GetUserAgent();
  UnitTest::Test<string>(userAgent, "Firefox 24 Linux(x86) Blah!", "UserAgent Test");
  UnitTest::PerformanceTestEnd("All Tests");

  UnitTest::ReportTestResult();


  return 0;
}
#endif
#undef _UNIT_TEST


