#ifndef _HTTP_HPP_
#define _HTTP_HPP_

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false

#include "liolib/Debug.hpp"

#include <string> // string

namespace lio {
namespace http {

using std::string;

enum class HttpVersion : uint8_t {
  V1_0,
  V1_1,
  V2_0
};

enum class RequestMethod : uint8_t {
  UNDEF = 0,
  GET,
  POST,
  HEAD,
  PUT,
  DELETE,
  TRACE,
  CONNECT
};

const string RequestMethodString[] = {
  "Undefined",
  "GET",
  "POST",
  "HEAD",
  "PUT",
  "DELETE",
  "TRACE",
  "CONNECT"
};
    
enum class RequestField : uint8_t {
  ACCEPT = 0,
  ACCEPT_CHARSET,
  ACCEPT_ENCODING,
  ACCEPT_LANGUAGE,
  CACHE_CONTROL,
  CONNECTION,
  COOKIE,
  CONTENT_LENGTH,
  CONTENT_TYPE,
  EXPECT,
  HOST,
  REFERER,
  USER_AGENT
};

const string RequestFieldString[] = {
  "Accept",
  "Accept-Charset",
  "Accept-Encoding",
  "Accept-Language",
  "Cache-Control",
  "Connection",
  "Cookie",
  "Content-Length",
  "Content-Type",
  "Expect"
  "Host",
  "Referer",
  "User-Agent"
};

enum class ResponseField : uint8_t {
  AGE = 0,
  ALLOW,
  CACHE_CONTROL,
  CONNECTION,
  CONTENT_DISPOSITION,
  CONTENT_ENCODING,
  CONTENT_LANGUAGE,
  CONTENT_LENGTH,
  CONTENT_MD5,
  DATE,
  EXPIRES,
  LAST_MODIFIED,
  LOCATION,
  PRAGMA,
  RETRY_AFTER,
  SET_COOKIE
};


const string ResponseFieldString[] = {
  "Age",
  "Allow",
  "Cache-Control",
  "Connection",
  "Content-Disposition",
  "Content-Encoding",
  "Content-Language",
  "Content-Length",
  "Content-MD5",
  "Date",
  "Expires",
  "Last-Modified",
  "Location",
  "Pragma",
  "Retry-After",
  "Set-Cookie"
};


/**
 * ContentType = Internet Media Type = MIME Type
 *  #REF: http://en.wikipedia.org/wiki/Internet_media_type 
 *  #REF: http://en.wikipedia.org/wiki/MIME_type
 *
 * Only contains Types that are supported.
 *
 */
enum class ContentType : uint8_t {
  UNDEF = 0,
  HTML,
  CSS,
  PLAINTEXT,
  XML,
  JAVASCRIPT,
  JSON,
  PDF,
  IMG_ICON,
  IMG_JPG,
  IMG_GIF,
  IMG_PNG,
  AUDIO_MP3,
  AUDIO_OGG,
  AUDIO_FLAC,
  ZIP,
  FORMDATA,
  FORMDATA_MULTIPART
};

const string ContentTypeString[] = {
  "Undefined",
  "text/html",
  "text/css",
  "text/plain",
  "text/xml",
  "application/javascript",
  "application/json",
  "application/pdf",
  "image/x-icon",
  "image/jpeg",
  "image/gif",
  "image/png",
  "audio/mpeg",
  "audio/ogg",
  "audio/ogg",
  "application/zip",
  "application/x-www-form-urlencoded",
  "multipart/formdata"
};

enum class ContentCharset : uint8_t {
  UNDEF = 0,
  UTF8
};

const string ContentCharsetString[] = {
  "Undefined",
  "charset=utf-8"
};

enum class ResponseCode : uint8_t {
  UNDEF = 0,
  CONTINUE, // 100 continue
  OK, // 200 OK
  BAD_REQUEST, // 400 Bad Request
  NOT_FOUND, // 404 Not Found
  METHOD_NOT_ALLOWED, // 405 Method Not Allowed
  LENGTH_REQUIRED, // 411 Length Required
  REQUEST_ENTITY_TOO_LARGE, // The request is larger than the server is willing or able to process.
  REQUEST_URI_TOO_LONG,
  SERVER_ERROR, // 500 Internal Server Error
  SERVICE_UNAVAILABLE // 503 Service Unavailable
};

const string ResponseCodeString[] = {
  "Undefined",
  "100 Continue",
  "200 OK",
  "400 Bad Request",
  "404 Not Found",
  "405 Method Not Allowed",
  "411 Length Required",
  "413 Request Entity Too Large",
  "414 Request-URI Too Long",
  "500 Internal Server Error",
  "503 Service Unavailable"
};





}
}

#endif
