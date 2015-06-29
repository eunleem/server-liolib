#include "HttpResponseBuilder.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
HttpResponseBuilder::Exception::exceptionMessages_[] = {
  HTTPRESPONSEBUILDER_EXCEPTION_MESSAGES
};
#undef HTTPRESPONSEBUILDER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.

HttpResponseBuilder::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpResponseBuilder::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpResponseBuilder::ExceptionType
HttpResponseBuilder::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====

HttpResponseBuilder::HttpResponseBuilder (const ResponseCode responseCode) :
  tempTextBody(nullptr)
{
  DEBUG_FUNC_START;

  //this->responseHeader_ = new string("HTTP/1.1 ");

  this->setResponseCode(responseCode);
}

HttpResponseBuilder::~HttpResponseBuilder() {
  DEBUG_FUNC_START;
  if (this->tempTextBody != nullptr) {
    delete tempTextBody;
  } 
}


DataBlock<string*> HttpResponseBuilder::GetHeader() {
  size_t headerEndPosition = this->responseHeader_.length();
  if (headerEndPosition > 5) {
    size_t pos = this->responseHeader_.find("\r\n\r\n", headerEndPosition - 5);
    if (pos == string::npos) {
      // If header is ending with invalid sequence,
      this->responseHeader_.append("\r\n");
    }
  } else {
    DEBUG_cerr << "Reponse header not yet ready." << endl; 
    return DataBlock<string*>();
  }
  
  DataBlock<string*> result(&this->responseHeader_, 0, this->responseHeader_.length());
  return result;
}

DataBlock<> HttpResponseBuilder::GetBody() const {
  return this->responseContent_;
}

bool HttpResponseBuilder::AddHeaderField(const string& field, const string& fieldValue) {
  DEBUG_cerr << "DEPRECATED FUNCTION. Use SetHeaderField instead." << endl; 
  return this->SetHeaderField(field, fieldValue);
}

bool HttpResponseBuilder::SetHeaderField (const string& field, const string& fieldValue) {

  size_t startPos = this->responseHeader_.find(field + ": ");
  if (startPos != string::npos) {
    // Already Existing.
    DEBUG_cout << "HeaderField already exists. It will be replaced." << endl; 
    size_t endPos = this->responseHeader_.find_first_of("\r\n", startPos, 1024);
    if (endPos != string::npos) {
      this->responseHeader_.erase(startPos, endPos);
    } 
  } 

  this->responseHeader_.append(field + ": " + fieldValue + "\r\n");
  return true;
}

bool HttpResponseBuilder::AddToHeaderField(const string& field, const string& additionalContent) {
  size_t startPos = this->responseHeader_.find(field + ": ");
  string existingValue;
  if (startPos != string::npos) {
    DEBUG_cout << "HeaderField already exists. It will be replaced." << endl; 
    size_t endPos = this->responseHeader_.find_first_of("\r\n", startPos, 1024);
    if (endPos != string::npos) {
      existingValue = this->responseHeader_.substr(startPos + field.length() + 2, endPos);
      this->responseHeader_.erase(startPos, endPos);
    } 
  } else {
    // Header field not found
    DEBUG_cerr << "AddToHeaderField is called but header does not exists. Doing nothing. Did you mean calling SetToHeaderField?" << endl; 
    return false;
  }

  return this->SetHeaderField (field, existingValue + "," + additionalContent);
}

bool HttpResponseBuilder::SetBody(const DataBlock<>& bodyDataBlock, bool isGzipped) {
  this->SetHeaderField("Content-Length", std::to_string(bodyDataBlock.length));
  this->isGzipped_ = isGzipped;
  if (isGzipped) {
    this->SetHeaderField("Content-Encoding", "gzip");
  }

  this->responseContent_ = bodyDataBlock;

  return true;
}

bool HttpResponseBuilder::SetBody(const string& text, bool isGzipped) {
  this->SetHeaderField("Content-Length", std::to_string(text.length()));
  this->isGzipped_ = isGzipped;
  if (isGzipped) {
    this->SetHeaderField("Content-Encoding", "gzip");
  }

  DataBlock<> bodyDataBlock((void*)text.c_str(), 0, text.length());
  this->responseContent_ = bodyDataBlock;

  return true;
}

bool HttpResponseBuilder::SetBody(string* text, bool isGzipped) {
  this->SetHeaderField("Content-Length", std::to_string(text->length()));
  this->isGzipped_ = isGzipped;
  if (isGzipped) {
    this->SetHeaderField("Content-Encoding", "gzip");
  }

  if (this->tempTextBody != nullptr) {
    DEBUG_cout << "TempTextBody already set. Will be deleted and set with the new value." << endl; 
    delete this->tempTextBody;
  } 
  this->tempTextBody = text;

  DataBlock<> bodyDataBlock((void*)text->c_str(), 0, text->length());
  this->responseContent_ = bodyDataBlock;

  return true;
}

bool HttpResponseBuilder::SetBody(const rapidjson::Document& jsondoc) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  jsondoc.Accept(writer);

  if (this->tempTextBody != nullptr) {
    DEBUG_cout << "TempTextBody already set. Will be deleted and set with the new value." << endl; 
    delete this->tempTextBody;
  } 

  this->tempTextBody = new string(buffer.GetString());

  this->SetHeaderField("Content-Length", std::to_string(tempTextBody->length()));

  DataBlock<> bodyDataBlock((void*)tempTextBody->c_str(), 0, tempTextBody->length());
  this->responseContent_ = bodyDataBlock;

  return true;
}

#if 0
bool HttpResponseBuilder::SetBody(string&& text, bool isGzipped) {
  this->AddHeaderField("Content-Length", std::to_string(text.length()));
  this->isGzipped_ = isGzipped;
  if (isGzipped) {
    this->AddHeaderField("Content-Encoding", "gzip");
  }

  DataBlock<> bodyDataBlock((void*)text.c_str(), 0, text.length());
  this->responseContent_ = bodyDataBlock;

  return true;
}
#endif

bool HttpResponseBuilder::setResponseCode(const ResponseCode responseCode) {
  DEBUG_FUNC_START;
  if (this->responseHeader_.empty() == false) {
    // if not empty, then WARN
    DEBUG_clog << "WARNING: ResponseHeader is not empty. It will be overwritten." << endl;
  }

  this->responseHeader_ = "HTTP/1.1 ";
  this->responseHeader_.append(http::ResponseCodeString[static_cast<int>(responseCode)]);
  this->responseHeader_.append("\r\n");

  return true;
}

}

#if _UNIT_TEST


#include <iostream>
#include <functional>

using namespace lio;

int main() {
  HttpResponseBuilder* response =
    new HttpResponseBuilder(HttpResponseBuilder::ResponseCode::OK);

  string field = "Host";
  string fieldValue = "test.com";
  response->SetHeaderField(field, fieldValue);

  DEBUG_cout << "A1" << endl;

  string body = "<html><body>CONTENT</body></html>";
  DEBUG_cout << "body Address: " << std::hex << &body << endl;

  response->SetBody(DataBlock(static_cast<void*>(&body), 0, body.length()));
  DEBUG_cout << "A2" << endl;

  DataBlock headerBlock = response->GetHeader();
  DataBlock bodyBlock = response->GetBody();
  DEBUG bodyBlock._PrintInfo();

  string* header = static_cast<string*>(headerBlock.address);
  string* bodyA = static_cast<string*>(bodyBlock.address);
  DEBUG_cout << "body Address: " << std::hex << bodyBlock.address << endl;

  const bool MUST_FAIL = true;
  UnitTest::Test<string>((*header),"HTTP/1.1 200 OK\r\nHost: test.com\r\n\r\n","Header", MUST_FAIL);
  DEBUG_cout << "A3" << endl;
  UnitTest::Test<string>((*header),"HTTP/1.1 200 OK\r\nHost: test.com\r\nContent-Length: 33\r\n\r\n","Header w body");
  DEBUG_cout << "A4" << endl;
  UnitTest::Test<string>((*bodyA),body,"Body");
  DEBUG_cout << "A5" << endl;
  UnitTest::ReportTestResult();
  return 0;
}
#endif
#undef _UNIT_TEST
