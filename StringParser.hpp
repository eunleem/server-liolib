#ifndef _STRINGPARSER_HPP_
#define _STRINGPARSER_HPP_
/*
  Name
    StringParser

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    October 30, 2013
  
  History
    October 30, 2013
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifndef _DEBUG
  #undef _DEBUG
  #define _DEBUG true
#endif

#include "Debug.hpp"
//#include "liolib/Debug.hpp"

#include <iostream>

#include <vector>

#include "StringMap.hpp"

namespace lio {

// ******** Exception Declaration *********
enum class StringParserExceptionType : std::uint8_t {
  GENERAL,
  CONFIG_ERROR
};
#define STRINGPARSER_EXCEPTION_MESSAGES \
  "StringParser Exception has been thrown.", \
  "Configuration Error."

class StringParserException : public std::exception {
public:
  StringParserException (StringParserExceptionType exceptionType = StringParserExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               StringParserExceptionType type() const noexcept;
  
private:
  const StringParserExceptionType       exceptionType_;
  static const char* const    exceptionMessages_[];
};

// ******** Exception Declaration END*********

class StringParser {
public:
  struct Config {
    string delimiter;
    string headerValueSeparator;
  };

  StringParser(const string* content, Config& config);
  ~StringParser();

  int ParseFields ();
protected:
  
private:
  Config config_;

  StringMap<string>* content_;

  /* Name
   *  parseFields()
   * Description
   *  parses all the fields in this->content_ by separator.
   * Output
   *  Returns
   *    int: number of fields parsed.
   */
  int parseFields();

  int tokenize(const string& text, const string delimiter);
  
};

}

#endif

