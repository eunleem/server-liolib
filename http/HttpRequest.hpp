#ifndef _HTTPREQUEST_HPP_
#define _HTTPREQUEST_HPP_
/*
  Name
    HttpRequest

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Aug 26, 2014
  
  History
    April 01, 2014
      Created

  ToDos
    

  Milestones
    1.0
      

  Alias
    CRLF = \r\n
 
    RequestString = Header + CRLFCRLF [ + Content ] // Inside [ ] are optional
    Header = FirstLine + Field + Field + ... CRLFCRLF // Field is "Header Field"
      Field ex: Accept, Host, User-Agent
    FirstLine = RequestType + URI + HTTP_VERSION CRLF
    Field = field + ': ' + fieldContent + CRLF
 
    Used names from
      http://en.wikipedia.org/wiki/List_of_HTTP_header_fields
 
    GET, POST, ... are called Request Method
    Accept, host, User-Agent, ... are called Header Field or just field.
 
  Learning Resources
    HTTP Protocol
      Http Request/Response Structure
        http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html

  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <string>
#include <map>
#include <list>
#include <cstdlib>

#include "liolib/Consts.hpp"
#include "liolib/http/Http.hpp"
#include "liolib/http/HttpPostDataParser.hpp"
#include "liolib/DataBlock.hpp"

#include "liolib/Util.hpp"

namespace lio {

using std::string;
using std::list;
using std::map;

using namespace Util;

class HttpRequest {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPREQUEST_EXCEPTION_MESSAGES \
  "HttpRequest Exception has been thrown."

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

enum class Language {
  ENGLISH,
  KOREAN
};

  HttpRequest();
  HttpRequest(DataBlock<char*> buffer);
  virtual
  ~HttpRequest();

  //#TODO: Add ParseRawHeader() to automatically get all data.

  size_t                GetHeaderSize() const;
  http::RequestMethod   GetRequestMethod() const;

  const string&         GetWholeUri() const;
  string                GetUri() const;

  string                GetQueryString(const string& fieldName) const;

  const string&         GetHost() const;
  const string&         GetUserAgent() const;
  size_t                GetContentLength() const;
  DataBlock<void*>      GetContent() const;
  http::ContentType     GetContentType() const;
  Language              GetAcceptLanguage() const;
  const string&         GetReferer() const;

  map<string, string>&  GetPostData();
  map<string, string>&  GetCookies();

  bool IsKeepAliveSupported() const;
  bool IsGzipSupported() const;

  bool SetBuffer(DataBlock<char*>& buffer);

  bool SetHeaderSize(const size_t headerSize);

  bool SetField(const string& fieldName, const string& fieldValue);
  bool SetField(string&& fieldName, string&& fieldValue);

  bool SetRequestMethod(http::RequestMethod method);
  bool SetRequestMethod(const string& string);
  bool SetUri(const string& uri);
  bool SetHost(const string& host);
  bool SetUserAgent(const string& userAgent);
  bool SetReferer(const string& referer);
  bool SetCookies(const string& fieldValue);
  bool SetContentLength(const size_t contentLength);
  bool SetAcceptLanguae(const Language acceptLanguage);
  bool SetIsKeepAliveSupported(const bool isKeepAliveSupported);
  bool SetIsGzipSupported(const bool isGzipSupported);
  bool SetContent(const DataBlock<void*>& content);
  bool SetContentType(const http::ContentType contentType);

protected:
  
private:

  DataBlock<char*> buffer;

  size_t headerSize;

  http::RequestMethod method;
  string uri;
  string host;
  string userAgent;
  string referer;
  map<string, string> cookies;
  size_t contentLength;
  http::ContentType contentType;
  Language language;
  
  bool isKeepAliveSupported;
  bool isGzipSupported;


  DataBlock<void*> content;
  HttpPostDataParser* postDataParser;


  bool parseRequestMethod();
  bool parseUri();

  int  parsePostData(const string& fieldValue);



  
};

}

#endif
