#include "StringParser.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
StringParserException::exceptionMessages_[] = {
  STRINGPARSER_EXCEPTION_MESSAGES
};
#undef STRINGPARSER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


StringParserException::StringParserException(StringParserExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
StringParserException::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const StringParserExceptionType
StringParserException::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


StringParser::StringParser(const string* content, Config& config)
  : content_(nullptr) {
  DEBUG_cout0 << "StringParser(string*, Config&) has been called." << endl;
  
  if (content->empty()) {
    DEBUG_cerr << "content is empty." << endl;
    throw StringParserException(StringParserExceptionType::CONFIG_ERROR);
  }

  if (config.delimiter.empty()) {
    throw StringParserException(StringParserExceptionType::CONFIG_ERROR);
  }

  this->config_ = config;


  this->content_ = new StringMap<string>(content);
}

StringParser::~StringParser() {
  DEBUG_cout0 << "~StringParser() has been called." << endl;
  if (this->content_ != nullptr) {
    delete content_;
  }
}

int StringParser::ParseFields() {
  const string* content = this->content_->GetContent();

}

int StringParser::parseFields() {
  
  return 0;
}

}

#define _UNIT_TEST false
#if _UNIT_TEST

#include "Test.hpp"
//#include "liolib/Test.hpp"

#include <iostream>
#include <functional>

using namespace lio;

int main() {
  return 0;
}
#endif
#undef _UNIT_TEST























































