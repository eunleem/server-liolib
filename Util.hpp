#ifndef _UTIL_HPP_
#define _UTIL_HPP_

/*
  Name
    Util

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 17, 2015
  
  History
    September 23, 2013
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/


#include <algorithm> 
#include <cctype>
#include <chrono>
#include <exception>
#include <functional> 
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <sstream> // stringstream
#include <vector> // vector<>()
#include <utility>

#include <cstdio>
#include <ctime> // time_t time() gmtime() strftime() struct tm
#include <cstring> // memset()

#include <dirent.h> // opendir()
#include <sys/ipc.h> // ftok()
#include <sys/types.h> // ftok() key_t
#include <sys/stat.h> // stat()
#include <unistd.h> // fcntl(), getpagesize()
#include <fcntl.h> // fcntl()

#include <errno.h> //

#include "liolib/DataBlock.hpp" // DataBlock<>()
#include "liolib/Consts.hpp"
#include "liolib/CustomExceptions.hpp" // InvalidArgumentException


#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false
#include "liolib/Debug.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;


typedef std::chrono::system_clock::time_point datetime;
typedef std::chrono::steady_clock::time_point steadytime;

namespace Util
{
  bool        IsDirectoryExistent ( const string& );
  bool        IsDirectoryExisting ( const char* );
  bool        IsDirectoryExisting ( const string& );

  bool        IsFileExistent ( const string& filePath, bool ifNotCreate = false );
  bool        IsFileExisting ( const char* filePath, bool ifNotCreate = false );
  bool        IsFileExisting ( const string& filePath, bool ifNotCreate = false );

  key_t       GenerateUniqueKey (string& keyFilePath, int projId = 1 );

  std::string Timestamp();
  std::string TimeToString(std::chrono::system_clock::time_point tp);

namespace File {
  ssize_t GetSize(std::fstream& file);

  bool IsDirectoryExistent ( const string& );
  bool IsDirectoryExisting ( const char* );
  bool IsDirectoryExisting ( const string& );

  bool IsFileExistent ( const string& filePath, bool ifNotCreate = false );
  bool IsFileExisting ( const char* filePath, bool ifNotCreate = false );
  bool IsFileExisting ( const string& filePath, bool ifNotCreate = false );

  bool _isFileExisting ( const char* filePath, bool ifNotCreate = false );
  bool _isDirectoryExisting ( const char* );

  template<typename LEN_T>
  ssize_t WriteString(std::ostream& os, const std::string& str) {
    LEN_T strlen = static_cast<LEN_T>(str.length());
    os.write((char*)&strlen, sizeof(strlen));
    os.write(str.c_str(), strlen);

    return strlen;
  }

  template<typename LEN_T>
  ssize_t ReadString(std::istream& is, std::string& str) {
    LEN_T strlen = 0;
    is.read((char*)&strlen, sizeof(strlen));
    str.resize(strlen);
    is.read((char*)str.c_str(), strlen);

    return str.length();
  }

  ssize_t WriteString(std::ostream& os, const std::string& str);
  ssize_t ReadString(std::istream& is, std::string& str);
}
  
namespace Time {
  datetime GetNow();
  uint64_t GetMillisecondsSinceEpoch(datetime tp = GetNow());

  datetime GetTimepoint(
      uint16_t year = 0,
      uint8_t month = 0,
      uint8_t day = 0,
      uint8_t hour = 0,
      uint8_t minute = 0,
      uint8_t second = 0,
      bool getUtc = false);

  string Timestamp(const string& format = "%F %T %Z");
  string TimeToString(datetime tp,
      const string& format = "%F %T %Z");
  string TimeToString(steadytime tp,
      const string& format = "%F %T %Z");

  string TimeToString(time_t rawTime,
      const string& format = "%F %T %Z");

}


namespace String {
  
  // To<T> only support, double, float, ulong, uint, short, ushort. NO int8_t or uint8_tc
  template<typename TYPE>
  TYPE To(const string& value) {
    std::stringstream ss;
    ss << value;
    TYPE convertedValue;
    ss >> convertedValue;
    return convertedValue;
  }

  template<typename TYPE>
  TYPE To(const char* value) {
    std::stringstream ss;
    ss << value;
    TYPE convertedValue;
    ss >> convertedValue;
    return convertedValue;
  }

  // trim from start
  static inline string& TrimBeg(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  static inline string& TrimEnd(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  // trim from both ends
  static inline string& Trim(std::string &s) {
    return TrimBeg(TrimEnd(s));
  }

  ssize_t Find(const string& toFind, char* location, size_t length);

  ssize_t Find(const string& toFind, lio::DataBlock<char*>& block);

  bool Compare(char* location, size_t length, const string& str);

  std::string Retrieve(char* location, size_t length);
  
  // 0 = UNLIMITED
  std::string RetrieveBetween(char* startLocation,
                         const string& endStr,
                         size_t lengthMax = 1024 * 8); // 8KB Length Limit
  


  unsigned int    ToUInt (const std::string &value);
  unsigned int    ToUInt (const char* value);
  
  bool            ToUpperFly(std::string& value);
  bool            ToUpperFly(std::string&& value);
  bool            ToUpperFly(char* ptr, size_t lenth);
  
  bool            ToLowerFly(std::string& value);
  bool            ToLowerFly(char* ptr, size_t lenth);

  std::string     ToUpper(const std::string& value);
  std::string     ToLower(const std::string& value);

  std::vector<std::string>
                  Tokenize(const std::string& source,
                           const std::string& delimiter);

  //string          Replace(const string& value);
  //string          Trim(const string& value);

  bool            CaseInsensitiveCompare(const std::string& rvalue, const std::string& lvalue);

  std::string     RandomString(const size_t length, const unsigned int complexity = 2);
  void            RandomString(char* dest, const size_t length, const unsigned int complexity = 2);

  string*         UriDecode(const void* address, const size_t length); 
  string          UriDecode(const string& uri); 
  bool            UriDecode(const void* address, const size_t length, string* decoded); 
  ssize_t         UriDecodeFly(char* ptr, size_t length);  // Returns finished length;

  size_t          Append(char* ptr, const string& str, size_t pos);


}

}


#endif /* _UTIL_HPP_ */
