#ifndef _HTTPREQUESTPARSER_HPP_
#define _HTTPREQUESTPARSER_HPP_
/*
  Name
    HttpRequestParser

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    May 12, 2014
  
  History

  ToDos
    Handle multiple requests in one Request string.


  Milestones
    1.0

  Learning Resources
    http://
  
  Copyright (c) All Rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false
#include "liolib/Debug.hpp"

#include <string>
#include <list>

#include "liolib/StringMap.hpp"
#include "liolib/DataBlock.hpp" // DataBlock
#include "liolib/CustomExceptions.hpp" // NotYetImplemented
#include "liolib/Consts.hpp" // STRING_NOT_FOUND

#include "liolib/Util.hpp" //Util::String::ToUpper

#include "liolib/http/Http.hpp" // Http RequestMethods



namespace lio {

using std::string;
using std::cout;
using std::endl;

using std::list;


class HttpRequestParser {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  BAD_REQUEST,
  METHOD_NOT_ALLOWED,
  CONTENT_LENGTH_REQUIRED,
  BAD_FIELD,
  OVERSIZE
};
#define HTTPREQUESTPARSER_EXCEPTION_MESSAGES \
  "HttpRequestParser Exception has been thrown.", \
  "Bad Request from Client. 400", \
  "Method Not Allowed. 405", \
  "Content-Length field is required and is empty. Length Required 411", \
  "One of the fields in the header is invalid.", \
  "Field is too long."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const
  char*           what() const noexcept;
  virtual const
  ExceptionType   type() const noexcept;
  
private:
  ExceptionType     exceptionType_;
  static const 
  char* const     exceptionMessages_[];
};

// ******** Exception Declaration END *********


  static const size_t URI_LENGTH_MAX;
  static const size_t FIELD_LENGTH_MAX;

  HttpRequestParser(const string* requestRawStr);
  virtual
  ~HttpRequestParser();


  http::RequestMethod GetRequestMethod() const;
  string GetUri() const;

  // SetAcceptedContentType
  // SetMaxLength

  string GetHost() const;
  string GetUserAgent() const;

  DataBlock<string*> GetBodyContent() const;
  string GetContentType() const;
  string GetContentLength() const;

  bool IsGzipSupported();
  bool IsKeepAliveSupported();

  //void        Parse();


protected:
  
private:
  //list<string*> strBlocks_;
  StringMap<string> requestStrMap_;

  http::RequestMethod requestMethod_;

  // Location for Memory Address and Pointer. Position for numeric value of a relative position.
  size_t contentBodyStartPosition_ = string::npos;


  bool parseEssentialFields(const string* requestStr);

  inline
  size_t findRequestMethod(const string* requestStr);

  inline
  size_t findUri(const string* requestStr, size_t findStartPosition);

  inline
  size_t findAcceptEncoding(const string* requestStr);


  inline
  size_t findConnection(const string* requestStr);

  /* Name
   *  findHeaderField
   *
   * Description
   *  Finds Header field from the End of Header.
   *  That way, it doesn't go through the body which might contain the same string as header field.
   *
   * Output
   *  Returns
   *    size_t - Position where field is found
   *    string::npos - when not found.
   *  Internally
   *    Calls requestMap_.AddKeyValue(headerField, DataBlock(position));
   */
  size_t findHeaderField(const string* requestStr, string headerField, const size_t headerEndPosition);


  bool findContentBody(const string* requestStr, const size_t headerEndPosition);


  DataBlock<string*> getFieldValue(const string& fieldName) const;

  size_t bodyLength_;

};

}

#endif

