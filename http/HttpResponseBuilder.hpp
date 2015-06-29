#ifndef _HTTPRESPONSEBUILDER_HPP_
#define _HTTPRESPONSEBUILDER_HPP_
/*
  Name
    HttpResponseBuilder

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    Jul 28, 2014
  
  History
    October 16, 2013
      Created

  ToDos
    1. AddHeaderField(HeaderField);
      Create HeaderFieldObject


  Milestones
    1.0
      

  Aliases Used
    ContentType = MIME Type
   
  Learning Resources
    HTTP Protocol
      Http Request/Response Structure
        http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
 
    MIME Type
      #REF: http://en.wikipedia.org/wiki/MIME_type

    Http Response Status Code
      #REF: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false
#include "liolib/Debug.hpp"

#include <string>

#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"


#include "liolib/http/Http.hpp"
#include "liolib/DataBlock.hpp"


namespace lio {


using std::string;
using lio::http::ContentType;
using lio::http::ResponseCode;


class HttpResponseBuilder {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define HTTPRESPONSEBUILDER_EXCEPTION_MESSAGES \
  "HttpResponseBuilder Exception has been thrown."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType
    = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********


  HttpResponseBuilder(const ResponseCode responseCode = ResponseCode::OK);
  ~HttpResponseBuilder();

  DataBlock<string*>  GetHeader();
  DataBlock<>         GetBody() const;


  // #DEPRECATED
  bool          AddHeaderField (const string& field, const string& fieldValue);

  // Set will automatically REPLACE if the field already exists.
  bool          SetHeaderField (const string& field, const string& fieldValue);
  bool          AddToHeaderField (const string& field, const string& additionalValue);

  

  // Set Body will Always REPLACE existing body.
  bool          SetBody (const DataBlock<>& bodyDataBlock, bool isGzipped = false);
  bool          SetBody (const string& text, bool isGzipped = false);
  bool          SetBody (string* text, bool isGzipped = false);
  //bool          SetBody (string&& text, bool isGzipped = false);
  bool          SetBody (const rapidjson::Document& jsondoc);

protected:
  
private:
  string        responseHeader_;
  DataBlock<>   responseContent_;

  string* tempTextBody;

  bool          isGzipped_;

  bool          setResponseCode (const ResponseCode responseCode);

};

}

#endif

