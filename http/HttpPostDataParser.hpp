#ifndef _HTTPPOSTDATAPARSER_HPP_
#define _HTTPPOSTDATAPARSER_HPP_
/*
  Name
    HttpPostDataParser

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Aug 26, 2014
  
  History
    April 24, 2014
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <string>
#include <map>
#include <vector>

#include "liolib/http/Http.hpp"
#include "liolib/DataBlock.hpp"
#include "liolib/Util.hpp"

namespace lio {

using std::string;
using std::map;
using std::vector;

class HttpPostDataParser {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPPOSTDATAPARSER_EXCEPTION_MESSAGES \
  "HttpPostDataParser Exception has been thrown."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********


  HttpPostDataParser();
  ~HttpPostDataParser();

  bool SetData(const string& fieldValue);
  bool SetContent(DataBlock<> content);

  bool ParsePostData();

  map<string, string>& GetPostData();
protected:
  
private:
  DataBlock<> content;
  http::ContentType contentType;
  string boundary; // To be used for MULTIPART data.
  map<string, string> postData;

  void parse();

  inline
  string getBoundary(const string& fieldValue);

  
};

}

#endif

