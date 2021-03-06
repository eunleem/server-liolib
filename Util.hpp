#ifndef _UTIL_HPP_
#define _UTIL_HPP_

/*
  Name
    Util

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Sep 28, 2015
  
  History
    September 23, 2013
      Created

  ToDos
    CONVERT TEST TO GTEST and ADD MORE TESTS
    


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
#include <iomanip> // put_time
#include <locale>
#include <string>
#include <sstream> // std::stringstream
#include <vector> // vector<>()
#include <utility>

#include <cstdio>
#include <cstdlib> // srand rand
#include <ctime> // time_t time() gmtime() strftime() struct tm
#include <cstring> // memset()
#include <cctype> // toupper(), tolower()

#include <dirent.h> // opendir()
#include <sys/ipc.h> // ftok()
#include <sys/types.h> // ftok() key_t
#include <sys/stat.h> // stat() mkdir()
#include <unistd.h> // fcntl(), getpagesize()
#include <fcntl.h> // fcntl()

#include <sys/socket.h> // getaddrinfo(), gai_strerror()
#include <netdb.h> // getaddrinfo(), gai_strerror()
#include <sys/un.h> // sockaddr_un, UNIX_PATH_MAX // I copied UNIX_PATH_MAX from linux/un.h to sys/un.h

#include <errno.h> //

#include "liolib/DataBlock.hpp" // DataBlock<>()
#include "liolib/Consts.hpp"
#include "liolib/CustomExceptions.hpp" // InvalidArgumentException


#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false

#include "liolib/Debug.hpp"
#include "liolib/Test.hpp"

using std::endl;


typedef std::chrono::system_clock::time_point datetime;
typedef std::chrono::steady_clock::time_point steadytime;

enum class Result : int {
  SUCCESSFUL = 0,
  GOOD = 0,
  ERROR = -1,
  FAIL = -1,
  STOP = -100,
  INTERRUPT = -100,
  RETRY = -200
};

namespace Util
{
bool IsDirectoryExistent(const std::string&);
  bool IsDirectoryExisting(const char*);
  bool IsDirectoryExisting(const std::string&);

  bool IsFileExistent(const std::string& filePath, bool ifNotCreate = false);
  bool IsFileExisting(const char* filePath, bool ifNotCreate = false);
  bool IsFileExisting(const std::string& filePath, bool ifNotCreate = false);

  key_t GenerateUniqueKey(std::string& keyFilePath, int projId = 1);

  bool Retry(std::function<Result()> tryWhat, unsigned int numRetry = 3,
             unsigned int intervalSeconds = 10);

namespace Convert {
  std::string ToString(const struct sockaddr& in_addr);
  uint32_t ToUInt32(const struct sockaddr& in_addr);
}

namespace Test {
  size_t RandomNumber(size_t max, size_t min = 0);
}

namespace File {
  ssize_t GetSize(std::fstream& file);

  bool IsDirectoryExistent(const std::string& path);
  bool IsDirectoryExisting(const char* path, bool ifNotCreate = false);
  bool IsDirectoryExisting(const std::string& path, bool ifNotCreate = false);

  bool IsFileExistent(const std::string& filePath, bool ifNotCreate = false);
  bool IsFileExisting(const char* filePath, bool ifNotCreate = false);
  bool IsFileExisting(const std::string& filePath, bool ifNotCreate = false);

  inline bool _isFileExisting(const char* filePath, bool ifNotCreate = false);
  inline bool _isDirectoryExisting(const char*);

  bool CreateDirectory(const std::string& path);

  bool Rename(std::string oldName, std::string newName);
  bool Remove(std::string filePath);

  template <typename LEN_T = uint32_t>
  ssize_t WriteString(std::ostream& os, const std::string& str,
                      unsigned fixedMaxLength = 0) {
    static_assert(std::is_same<LEN_T, uint64_t>::value ||
                      std::is_same<LEN_T, uint32_t>::value ||
                      std::is_same<LEN_T, uint16_t>::value ||
                      std::is_same<LEN_T, uint8_t>::value,
                  "WriteString<T>: T must be uintXY_t");

    LEN_T strlen = static_cast<LEN_T>(str.length());
    os.write((char*)&strlen, sizeof(strlen));
    os.write(str.c_str(), strlen);

    if (fixedMaxLength > 0) {
      if (fixedMaxLength < str.length()) {
        LOG_alert << "String length exceeds the fixedMaxLength. "
                     "strlen: " << str.length() << endl;
      }
      const unsigned skip = fixedMaxLength - str.length();
      for (unsigned i = 0; i < skip; ++i) {
        os.write("\0", 1);
      }
    }

    return strlen;
  }

  template <typename LEN_T = uint32_t>
  ssize_t ReadString(std::istream& is, std::string& str,
                     unsigned fixedMaxLength = 0) {
    static_assert(std::is_same<LEN_T, uint64_t>::value ||
                      std::is_same<LEN_T, uint32_t>::value ||
                      std::is_same<LEN_T, uint16_t>::value ||
                      std::is_same<LEN_T, uint8_t>::value,
                  "ReadString<T>: T must be uintXY_t");

    LEN_T storedStrLength = 0;
    is.read((char*)&strlen, sizeof(storedStrLength));
    str.resize(storedStrLength);
    is.read(const_cast<char*>(str.c_str()), storedStrLength);

    if (fixedMaxLength > 0) {
      if (fixedMaxLength < str.length()) {
        LOG_alert << "Stored length exceeds Max Length or Fixed length. "
                     "storedLength: " << str.length() << endl;
      }
      const unsigned skip = fixedMaxLength - str.length();
      char a;
      for (unsigned i = 0; i < skip; ++i) {
        is.read(&a, 1);
      }
    }

    return str.length();
  }

  ssize_t WriteString(std::ostream& os, const std::string& str,
                      unsigned fixedLength = 0);
  ssize_t ReadString(std::istream& is, std::string& str,
                     unsigned fixedLength = 0);
}
  
namespace Time {
  datetime GetNow();
  uint64_t GetMillisecondsSinceEpoch(datetime tp = GetNow());
  uint64_t GetMillisecondsSinceEpoch(steadytime tp);

  datetime GetTimepoint(
      uint16_t year = 0,
      uint8_t month = 0,
      uint8_t day = 0,
      uint8_t hour = 0,
      uint8_t minute = 0,
      uint8_t second = 0,
      bool getUtc = false);


  std::string Timestamp(const std::string& format = "%F %T %Z", bool in_localtime = true);
  std::string TimestampNum();
  std::string TimeToString(const datetime tp, const std::string& format = "%F %T %Z");
  std::string TimeToString(const steadytime tp, const std::string& format = "%F %T %Z");

  std::string ToString(const steadytime tp, const std::string& format = "%F %T %Z", bool in_localtime = true);
  std::string ToString(const datetime tp, const std::string& format = "%F %T %Z", bool in_localtime = true);

  std::string ToString(const time_t rawTime, const std::string& format = "%F %T %Z", bool in_localtime = true);
  

  time_t steady_clock_to_time_t(const std::chrono::steady_clock::time_point tp);
}


namespace String {
  
  // To<T> only support, double, float, ulong, uint, short, ushort. NO int8_t or uint8_tc
  template<typename TYPE>
  TYPE To(const std::string& value) {
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
  static inline std::string& TrimBeg(std::string &s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  static inline std::string& TrimEnd(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(
                                                   std::isspace))).base(),
            s.end());
    return s;
  }

  // trim from both ends
  static inline std::string& Trim(std::string &s) {
    return TrimBeg(TrimEnd(s));
  }

  ssize_t Find(const std::string& toFind, char* location, size_t length);

  ssize_t Find(const std::string& toFind, lio::DataBlock<char*>& block);

  bool Compare(char* location, size_t length, const std::string& str);

  std::string Retrieve(char* location, size_t length);

  std::string CaptureBetween(const std::string& src, unsigned startOffset,
                             const std::string& endStr);
  // 0 = UNLIMITED
  std::string RetrieveBetween(char* startLocation,
                         const std::string& endStr,
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

  std::vector<std::string>&   Split(const std::string &s, char delim, std::vector<std::string>& elems);
  std::vector<std::string>    Split(const std::string &s, char delim);


  std::vector<std::string>
                  Tokenize(const std::string& source,
                           const std::string& delimiter);

  //string          Replace(const std::string& value);
  //string          Trim(const std::string& value);

  bool            CaseInsensitiveCompare(const std::string& rvalue, const std::string& lvalue);

  std::string     RandomString(const size_t length, const unsigned int complexity = 2);
  void            RandomString(char* dest, const size_t length, const unsigned int complexity = 2);

  const char*     _getRandomStringPool(const unsigned complexity);

  std::string*    UriDecode(const void* address, const size_t length); 
  std::string     UriDecode(const std::string& uri); 
  bool            UriDecode(const void* address, const size_t length, std::string* decoded); 
  ssize_t         UriDecodeFly(char* ptr, size_t length);  // Returns finished length;


  ssize_t         SubstrUtf8(std::string& str, size_t maxBytes);

  size_t          TrimIncompleteUTF8(std::string& str);

  std::string     JsonEncode(const std::string& str);
  bool            IsSafeForJson(const std::string& str);

  bool            IsUserInputSafe(const std::string& str);

  size_t          Append(char* ptr, const std::string& str, size_t pos);



}

}


#endif /* _UTIL_HPP_ */
