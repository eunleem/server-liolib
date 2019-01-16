#include "Util.hpp"

#define _UNIT_TEST false


namespace Util {

bool IsDirectoryExistent( const std::string& dirPath ) {
  DEPRECATED_FUNC("Util::File::IsDirectoryExisting");
  return File::_isDirectoryExisting( dirPath.c_str() );
}

bool IsDirectoryExisting( const char* dirPath ) {
  DEPRECATED_FUNC("Util::File::IsDirectoryExisting");
  return File::_isDirectoryExisting( dirPath );
}

bool IsDirectoryExisting ( const std::string& dirPath ) {
  DEPRECATED_FUNC("Util::File::IsDirectoryExisting");
  return File::_isDirectoryExisting( dirPath.c_str() );
}

bool IsFileExistent (const std::string& filePath, bool ifNotCreate) {
  DEPRECATED_FUNC("Util::File::IsFileExisting");
  return File::_isFileExisting(filePath.c_str(), ifNotCreate);
}

bool IsFileExisting ( const char* filePath, bool ifNotCreate ) {
  DEPRECATED_FUNC("Util::File::IsFileExisting");
  return File::_isFileExisting(filePath, ifNotCreate);
}

bool IsFileExisting ( const std::string& filePath, bool ifNotCreate ) {
  DEPRECATED_FUNC("Util::File::IsFileExisting");
  return File::_isFileExisting(filePath.c_str(), ifNotCreate);
}

/*
  http://linux.die.net/man/3/ftok
  2nd parameter of ftok() accepts INT type but only uses least significant 8 bits (1 byte).
*/
key_t GenerateUniqueKey ( std::string& keyFilePath, int projId ) {
  // #TODO: Check whether the file exists or not.
  if ( projId > 128 || projId <= 0) {
    // Minor Error. Show Warning or leave a warning Log.
    // Doesn't have to stop the program tho.
    LOG_err << "ProjId must be less than 128 and greater than 0." << endl; 
    throw;
  }
  return ftok ( keyFilePath.c_str(), projId );
}

std::string Timestamp() {
  DEPRECATED_FUNC("Util::Time::Timestamp()");
  time_t rawTime;
  struct tm* gmtTime;
  char buffer[30];

  memset(buffer, 0, sizeof(buffer));
  
  time(&rawTime);
  gmtTime = gmtime(&rawTime);

  // Time to Formatted String
  //   http://www.cplusplus.com/reference/ctime/strftime/   
  //   %F = yyyy-MM-dd
  //   %T = HH:mm:ss
  //   %Z = GMT or PST (Time Zone)
  strftime (buffer, 30, "%F %T %Z", gmtTime);
 
  return std::string(buffer);
}

std::string TimeToString(std::chrono::system_clock::time_point tp) {
  DEPRECATED_FUNC("Util::Time::ToString()");
  return Util::Time::TimeToString(tp);
}

bool Retry(std::function<Result()> tryWhat, unsigned int numRetry,
           unsigned int intervalSeconds) {

  unsigned int retryCount = 0;
  while (numRetry > retryCount) {
    Result result = tryWhat();
    if (result == Result::GOOD) {
      return true;
    }
    if (result == Result::INTERRUPT) {
      LOG_info << "Stop retrying and return right away." << endl;
      return false;
    }
    LOG_info << "Trying again after " << intervalSeconds << " seconds... "
             << "(" << ++retryCount << "/" << numRetry << ")" << endl;
    sleep(intervalSeconds);
  }
  return false;
}

namespace Convert {

  std::string ToString(const struct sockaddr& in_addr) {
    const socklen_t in_len = sizeof(in_addr);
    char hostBuf[NI_MAXHOST], portBuf[NI_MAXSERV];
    int result = -1;
    result = getnameinfo (&(in_addr), in_len,
                          hostBuf, sizeof(hostBuf),
                          portBuf, sizeof(portBuf),
                          NI_NUMERICHOST | NI_NUMERICSERV);
    if (result == (int)Result::ERROR) {
      LOG_warn << "connection info error... errno: " << errno << endl;
      return "";
    }

    const std::string ipaddrStr =  std::string(hostBuf) + ":" + std::string(portBuf);
    LOG_info << "Converted Ip Address to String. " << ipaddrStr << endl;
    return ipaddrStr;
  }

  uint32_t ToUInt32(const struct sockaddr& in_addr) {
    struct sockaddr_in* addr_in = (struct sockaddr_in*) &in_addr;
    DEBUG_temp << "addr_in->sin_addr.s_addr:" << addr_in->sin_addr.s_addr << endl;
    uint32_t result = static_cast<uint32_t>(addr_in->sin_addr.s_addr);
    DEBUG_temp << "uint32_t:" << result << endl;
    return result;
  }
}

namespace Test {
  size_t RandomNumber(size_t max, size_t min) {
    return rand() % max + min;
  }
}

namespace File {
  ssize_t GetSize(std::fstream& file) {
    if (file.is_open() == false) {
      LOG_err << "Coult not open file to Get Size" << endl; 
      return -1;
    } 

    auto orgPos = file.tellg();
    file.seekg(0, std::ios::end);
    auto pos = file.tellg();
    file.seekg(orgPos, std::ios::beg);
    return static_cast<ssize_t>(pos);
  }

  bool IsDirectoryExisting ( const std::string& dirPath, bool ifNotCreate ) {
    bool isExisting = File::_isDirectoryExisting( dirPath.c_str() );
    if (isExisting == false) {
      if (ifNotCreate == true) {
        isExisting = File::CreateDirectory(dirPath);
      }
    }
    return isExisting;
  }

  bool IsFileExisting ( const std::string& filePath, bool ifNotCreate ) {
    return File::_isFileExisting(filePath.c_str(), ifNotCreate);
  }

  bool _isDirectoryExisting( const char* dirPath ) {
    if ( dirPath == NULL) {
      throw InvalidArgumentException();
    }

    DIR *dir;
    dir = opendir (dirPath);

    if (dir != NULL)
    {
      (void) closedir (dir);
      return true;
    }

    return false;
  }

  bool _isFileExisting ( const char* filePath, bool ifNotCreate ) {
    bool isExistent = (access(filePath, R_OK|W_OK) != -1);
    if (isExistent == false) {
      if (ifNotCreate == true) {
        std::fstream file(filePath, std::ios::out);
        if (file.is_open() == true) {
          file.close();
          LOG_info << "File Created. path: " << filePath << endl; 
          return true;
        } else {
          LOG_err << "Could not create file. filePath: " << filePath << endl; 
          return false;
        }
      } 
    } 

    return isExistent;
  }

  bool CreateDirectory(const std::string& path) {
    int result = mkdir(path.c_str(), S_IRWXU | S_IRWXG);
    if (result == -1) {
      DEBUG_warn << "Failed to create directory. errno: " << errno << " path: " << path << endl;
      return false;
    } 

    return true;
  }

  bool Rename(std::string oldName, std::string newName) {
    bool isExisting = File::_isFileExisting(oldName.c_str());
    if (isExisting == false) {
      LOG_warn << "File does not exist. Rename Failed." << endl; 
      return false;
    } 

    int result = std::rename(oldName.c_str(), newName.c_str());
    if (result != 0) {
      LOG_warn << "Rename Failed. errno: " << errno << endl; 
      return false;
    } 

    return true;
  }

  bool Remove(std::string filePath) {
    bool isExisting = File::_isFileExisting(filePath.c_str());
    if (isExisting == false) {
      LOG_warn << "File does not exist. Remove Failed." << endl; 
      return false;
    } 

    int result = std::remove(filePath.c_str());
    if (result != 0) {
      LOG_err << "Remove failed. errno: " << errno << endl; 
      return false;
    } 

    return true;
  }

  ssize_t WriteString(std::ostream& os, const std::string& str, unsigned fixedMaxLength) {
    DEPRECATED_FUNC("WriteString<>");
    uint32_t strlen = str.length();
    os.write((char*)&strlen, sizeof(strlen));
    os.write(str.c_str(), strlen);

    if (fixedMaxLength > 0) {
      if (fixedMaxLength < str.length()) {
        LOG_alert << "String length exceeds the fixedMaxLength. "
                     "strlen: " << (int)str.length() << endl;
      }
      const unsigned skip = fixedMaxLength - str.length();
      for (unsigned i = 0; i < skip; ++i) {
        os.write("\0", 1);
      }
    }

    return strlen;
  }

  ssize_t ReadString(std::istream& is, std::string& str, unsigned fixedMaxLength) {
    DEPRECATED_FUNC("ReadString<>");
    uint32_t strlen = 0;
    is.read((char*)&strlen, sizeof(strlen));
    str.resize(strlen);
    is.read((char*)str.c_str(), strlen);

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
}

namespace Time {
  datetime GetNow() {
    return std::chrono::system_clock::now();
  }

  uint64_t GetMillisecondsSinceEpoch(datetime tp) {
    uint64_t milliseconds_since_epoch = 
      std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();

    return milliseconds_since_epoch;
  }

  uint64_t GetMillisecondsSinceEpoch(steadytime tp) {
    return static_cast<uint64_t>(steady_clock_to_time_t(tp)) * 1000;
  }

  // Without any argument, it will return Today's date with 00:00:00 time.
  datetime GetTimepoint(
      uint16_t year,
      uint8_t month,
      uint8_t day,
      uint8_t hour,
      uint8_t minute,
      uint8_t second,
      bool getUtc)
  {
    time_t rawTime;
    struct tm* timeinfo;

    // When called without any parameters, it returns current time.
    time(&rawTime);
    if (getUtc == true) {
      timeinfo = gmtime(&rawTime);
    } else {
      timeinfo = localtime(&rawTime);
    }

    // REF: http://www.cplusplus.com/reference/ctime/tm/
    if (year != 0) {
      timeinfo->tm_year = year - 1900; // years since 1900
    } 
    if (month != 0) {
      timeinfo->tm_mon = month - 1; // 0 is January, 11 is December
    } 
    if (day != 0) {
      timeinfo->tm_mday = day;
    } 
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = minute;
    timeinfo->tm_sec = second;
    timeinfo->tm_isdst = -1; // -1 allows mktime() to configure it automatically

    rawTime = mktime(timeinfo);
    if (rawTime == -1) {
      LOG_err << "Could not convert to datetime." << endl; 
      throw std::exception();
    } 

    return std::chrono::system_clock::from_time_t(rawTime);
  }

  std::string Timestamp(const std::string& format, bool in_localtime) {
    return ToString(std::chrono::system_clock::now(), format, in_localtime);
  }

  std::string TimestampNum() {
    return ToString(std::chrono::system_clock::now(), "%Y%m%d%H%M%S");
  }

  std::string TimeToString(const datetime tp, const std::string& format) {
    DEPRECATED_FUNC("ToString()");
    std::time_t rawTime = std::chrono::system_clock::to_time_t(tp);
    return ToString(rawTime, format);
  }

  std::string TimeToString(const steadytime tp, const std::string& format) {
    DEPRECATED_FUNC("ToString()");
    std::time_t rawTime = Time::steady_clock_to_time_t(tp);
    return ToString(rawTime, format);
  }

  std::string ToString(const steadytime tp, const std::string& format,
                       bool in_localtime) {
    std::time_t rawTime = Time::steady_clock_to_time_t(tp);
    return ToString(rawTime, format, in_localtime);
  }

  std::string ToString(const datetime tp, const std::string& format,
                       bool in_localtime) {
    std::time_t rawTime = std::chrono::system_clock::to_time_t(tp);
    return ToString(rawTime, format, in_localtime);
  }

  std::string ToString(const time_t rawTime, const std::string& format,
                       bool in_localtime) {
    struct tm* time;
    std::string buffer(32, '\0');
    if (in_localtime == true) {
      time = std::localtime(&rawTime);
    } else {
      time = std::gmtime(&rawTime);
    }

    if (Util::String::ToLower(format) == "http") {
      const_cast<std::string&>(format) = "%a, %d %b %Y %H:%M:%S %Z";
    }

    // Time to Formatted String
    //   http://www.cplusplus.com/reference/ctime/strftime/   
    //   %F = yyyy-MM-dd
    //   %T = HH:mm:ss
    //   %Z = GMT or PST (Time Zone)
    strftime((char*)buffer.c_str(), 32, format.c_str(), time);

    std::size_t len = std::strlen(buffer.c_str());
    buffer.resize(len);
    return buffer;
    //std::stringstream ss;
    //ss << std::put_time(std::localtime(&rawTime), format.c_str());
    //return ss.str();
  }

  time_t steady_clock_to_time_t(const std::chrono::steady_clock::time_point t) {
      return std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now() +
          (t - std::chrono::steady_clock::now()));
  }
}

namespace String {


  unsigned int ToUInt (const std::string &value) {
    return To<unsigned int>(value);
  }

  unsigned int ToUInt (const char* value) {
    return To<unsigned int>(value);
  }

  ssize_t Find (const std::string& toFind, lio::DataBlock<char*>& block) {
    // #TODO: NOT thoroughly tested yet.
    const size_t index = block.GetIndex();
    const size_t length = block.GetLength();
    char* obj = (char*) block.GetObject();

    obj += index;

    size_t toFindIndex = 0;
    for (size_t i = 0; length > i; ++i) {
      if ((*obj) == toFind[toFindIndex]) {
        toFindIndex += 1;
        if (toFindIndex == toFind.length()) {
          return index + i - (toFind.length() - 1);
        } 
      } else if ((*obj) == '\0') {
        // End of String reached and Not Found.
        return -1;
      } else {
        toFindIndex = 0;
      }
      obj += 1;
    } 
    return -1;
  }

  ssize_t Find (const std::string& toFind, char* location, size_t length) {
    // #TODO: NOT thoroughly tested yet.

    size_t toFindIndex = 0;
    for (size_t i = 0; length > i; ++i) {
      if ((*location) == toFind[toFindIndex]) {
        toFindIndex += 1;
        if (toFindIndex == toFind.length()) {
          return i - (toFind.length() - 1);
        } 
      } else if ((*location) == '\0') {
        // End of String reached and Not Found.
        return -1;
      } else {
        toFindIndex = 0;
      }
      location += 1;
    } 
    return -1;
  }

  bool Compare (char* location, size_t length, const std::string& str) {
    return strncmp(location, str.c_str(), length) == 0;
  }

  std::string Retrieve (char* location, size_t length) {
    DEPRECATED_FUNC("std::string ctor std::string(char*, length)");
    return std::string(location, length);
  }

  std::string CaptureBetween(const std::string& src, unsigned startOffset,
                             const std::string& endStr) {
    size_t foundPos = src.find(endStr, startOffset);
    if (foundPos == std::string::npos) {
      LOG_warn << "CaptureBetween could not find the endStr." << endl;
      return "";
    }

    std::string captured = src.substr(startOffset, foundPos);
    DEBUG_result << "CaptureBetween captured " << captured << endl;
    return captured;
  }

  std::string RetrieveBetween(char* startLocation, const std::string& endStr,
                              size_t lengthMax) {
    size_t endStrIndex = 0;
    std::string between;
    between.reserve(lengthMax);

    for (size_t i = 0; lengthMax > i; ++i) {
      if ((*startLocation) == endStr[endStrIndex]) {
        ++endStrIndex;
        if (endStrIndex == endStr.length()) {
          // Done. Return between.
          between.shrink_to_fit();
          return between;
        } 
      } else {
        between += (*startLocation);
        endStrIndex = 0;
      }
      ++startLocation;
    } 

    return "";
  }
  
  bool ToUpperFly (std::string& value) {
    for (auto& c : value) {
      c = static_cast<char>(std::toupper(c));
    }
    return true;
  }

  bool ToUpperFly (char* ptr, size_t length) {
    LOG_info << "ToUpperFly for " << length << " chars." << std::endl;
    for (unsigned int i = 0; length > i; ++i) {
      (*ptr) = static_cast<char>( std::toupper(*ptr));
      ptr++;
    }
    return true;
  }

  bool ToLowerFly (std::string& value) {
    for (auto& c : value) {
      c = static_cast<char>(std::tolower(c));
    }
    return true;
  }

  bool ToLowerFly (char* ptr, size_t length) {
    LOG_info << "ToLowerFly for " << length << " chars." << std::endl;

    for (unsigned int i = 0; length > i; ++i) {
      (*ptr) = static_cast<char>( std::tolower(*ptr));
      ptr++;
    }
    return true;
  }


  std::string ToUpper (const std::string& value) {
    std::string upperCase;
    upperCase.reserve(value.length());

    for (const auto& c : value) {
      upperCase += static_cast<char>(std::toupper(c));
    }

    return upperCase;
  }

  std::string ToLower(const std::string& value) {
    std::string converted;
    converted.reserve(value.length());

    for (const auto& c : value) {
      converted += static_cast<char>(std::tolower(c));
    }

    return converted;
  }


  std::vector<std::string> &Split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }


  std::vector<std::string> Split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    Util::String::Split(s, delim, elems);
    return elems;
  }

  std::vector<std::string> Tokenize(const std::string& source, const std::string& delimiter) {
    DEPRECATED_FUNC("Util::String::Split(std::string src, char delim)")
    std::vector<std::string> result;

    if (delimiter.empty()) {
      return result;
    }
    size_t delimiterPosition = 0;
    size_t prevDelimiterPosition = 0;

    //int count = 0;

    while (true) {
      size_t sectionLength = 0;

      delimiterPosition = source.find(delimiter, prevDelimiterPosition);
      if (delimiterPosition == std::string::npos) {
        // CASE A: No Delimiter
        // CASE B: After Last Delimiter
        if (prevDelimiterPosition == 0) {
        //if (count == 0) {
          // CASE A: No delimiter
          // Do nothing
        } else {
          // CASE B: After last delimiter
          prevDelimiterPosition += delimiter.length() - 1;
          sectionLength = source.length() - prevDelimiterPosition;
          if (sectionLength > 0) {
            // Only add section if the length is greater than 0.
            std::string section = source.substr(prevDelimiterPosition, sectionLength);
            result.push_back(section);
          }

        }
        break;
      } else {
        // CASE A: Delimiter at very First
        // CASE B: Delimiter Normal
        // CASE C: Delimiter right after another delimiter
        // CASE D: Delimiter at the very end.
        
        if (delimiterPosition == 0) {
          // CASE A
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        }

        sectionLength = delimiterPosition - prevDelimiterPosition;
        
        if (delimiterPosition + delimiter.length() == source.length()) {
          // CASE D
          std::string section = source.substr(prevDelimiterPosition, sectionLength);
          result.push_back(section);
          break;
        } else if (delimiterPosition + delimiter.length() > source.length()) {
          std::cerr << "Umm..... nonono this shouldn't happen" << endl;
          break; 
        }

        if (sectionLength > 0) {
          // CASE B
          std::string section = source.substr(prevDelimiterPosition, sectionLength);
          result.push_back(section);
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        } else if (sectionLength == 0) {
          // CASE C
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        } else {
          std::cerr << "Umm this shouldn't happen." << endl;
          break;
        }

      }
    }
    return result;
  }

  bool CaseInsensitiveCompare (const std::string& rvalue, const std::string& lvalue) {
    if (rvalue.length() != lvalue.length()) {
      return false;
    }

    std::string lowerRvalue = ToLower(rvalue);
    std::string lowerLvalue = ToLower(lvalue);
    if (lowerRvalue == lowerLvalue) {
      return true;
    } else {
      return false;
    }
  }

  static const char random_char_alphanum_special[] =
    "0123456789"
    "_-!@#$^*&|=+;:`~"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  static const char random_char_alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  static const char random_char_alpha[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    
  static const char random_char_alphauppernum[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";

  static const char random_char_alphaupper[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";


  static const char random_char_numbers[] =
    "0123456789";

  std::string RandomString(const size_t length, const unsigned int complexity) {
    const char* pool = _getRandomStringPool(complexity);
    const unsigned int strPoolLength = strlen(pool);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; length > i; i++) {
      result += pool[rand() % strPoolLength];
    } 

    return result;
  }

  void RandomString(char* dest, const size_t length, const unsigned int complexity) {
    const char* pool = _getRandomStringPool(complexity);
    const unsigned int strPoolLength = strlen(pool);
    for (size_t i = 0; length > i; i++) {
      dest[i] =  pool[rand() % strPoolLength];
    } 
  }

  const char* _getRandomStringPool(const unsigned complexity) {
    switch (complexity) {
      case 0: return random_char_numbers;
      case 1: return random_char_alphaupper;
      case 2: return random_char_alphauppernum;
      case 3: return random_char_alpha;
      case 4: return random_char_alphanum;
      case 5: return random_char_alphanum_special;
      default:
        LOG_warn << "Complexity cannot be greater than 4, less than 0. Complexity Set to 3." << endl; 
        return random_char_alpha;
    } 
    assert(false && "Should Never Reach this point.");
    return nullptr;
  }


  bool UriDecode (const void* address, const size_t length, std::string* decoded) {
    decoded->clear();
    decoded->reserve(length);

    char* c = (char*) address;
    char two[2];
    char f;

    for (size_t i = 0; length > i; i++) {
      bool isValidUri = true;
      switch ((*c)) {
       case '%':
        for (uint8_t j = 0; 2 > j; j++) {
          c += 1;
          i += 1;
          //DEBUG_cout << "j: " << j << " c: " << (*c) << endl;
          // read next two chars
          if ((*c) >= 'a' && (*c) <= 'z') {
            // To Upper Case
            two[j] = (*c) - 32;
          } else {
            two[j] = (*c);
          }

          if (two[j] >= 'A' && two[j] <= 'F') {
            // Hex Char to Int
            two[j] = two[j] - 65 + 10;
          } else if (two[j] >= '0' && two[j] <= '9') {
            two[j] = two[j] - 48;
          } else {
            // ERROR
            isValidUri = false;
            break;
          }
        }

        if (isValidUri == true) {
          f = two[0] * 16 + two[1];
        } else {
          f = ' ';
        }

        break;
       case '+':
        f = ' ';
        break;
       default:
        f = (*c);
        break;
      }
      //DEBUG_cout << "f: " << f << endl;

      (*decoded) += f;

      c += 1;
    }
    //DEBUG_cout << "UriDecoded result: " << (*decoded) << endl;

    decoded->shrink_to_fit();
    return true;
  }

  std::string* UriDecode (const void* address, const size_t length) {
    DEPRECATED_FUNC("UriDecode(conststd::string&)");

    std::string* decoded = new std::string();
    decoded->reserve(length);

    char* c = (char*) address;
    char two[2];
    char f;

    for (size_t i = 0; length > i; i++) {
      bool isValidUri = true;
      switch ((*c)) {
       case '%':
        for (uint8_t j = 0; 2 > j; j++) {
          c += 1;
          i += 1;
          //DEBUG_cout << "j: " << j << " c: " << (*c) << endl;
          // read next two chars
          if ((*c) >= 'a' && (*c) <= 'z') {
            // To Upper Case
            two[j] = (*c) - 32;
          } else {
            two[j] = (*c);
          }


          if (two[j] >= 'A' && two[j] <= 'F') {
            // Hex Char to Int
            two[j] = two[j] - 65 + 10;
          } else if (two[j] >= '0' && two[j] <= '9') {
            two[j] = two[j] - 48;
          } else {
            // ERROR
            isValidUri = false;
            break;
          }
        }

        if (isValidUri == true) {
          f = two[0] * 16 + two[1];
        } else {
          f = ' ';
        }

        break;
       case '+':
        f = ' ';
        break;
       default:
        f = (*c);
        break;
      }
      //DEBUG_cout << "f: " << f << endl;

      (*decoded) += f;

      c += 1;
    }
    //DEBUG_cout << "UriDecoded result: " << (*decoded) << endl;

    decoded->shrink_to_fit();
    return decoded;
  }

  std::string UriDecode (const std::string& uri) {
    std::string decoded;
    decoded.reserve(uri.length());

    char* c = const_cast<char*>(uri.c_str());
    char two[2];
    char f;

    for (size_t i = 0; uri.length() > i; i++) {
      bool isValidUri = true;
      switch ((*c)) {
       case '%':
        for (uint8_t j = 0; 2 > j; j++) {
          c += 1;
          i += 1;
          //DEBUG_cout << "j: " << j << " c: " << (*c) << endl;
          // read next two chars
          if ((*c) >= 'a' && (*c) <= 'z') {
            // To Upper Case
            two[j] = (*c) - 32;
          } else {
            two[j] = (*c);
          }

          if (two[j] >= 'A' && two[j] <= 'F') {
            // Hex Char to Int
            two[j] = two[j] - 65 + 10;
          } else if (two[j] >= '0' && two[j] <= '9') {
            two[j] = two[j] - 48;
          } else {
            // ERROR
            isValidUri = false;
            break;
          }
        }

        if (isValidUri == true) {
          f = two[0] * 16 + two[1];
        } else {
          f = ' ';
        }

        break;
       case '+':
        f = ' ';
        break;
       default:
        f = (*c);
        break;
      }
      //DEBUG_cout << "f: " << f << endl;

      decoded += f;

      c += 1;
    }
    //DEBUG_cout << "UriDecoded result: " << (*decoded) << endl;

    decoded.shrink_to_fit();
    return decoded;
  }

  ssize_t UriDecodeFly (char* address, size_t length) {

    ssize_t count = 0;
    char* ins = address;
    char* cur = address;
    char two[2];
    char f;

    for (size_t i = 0; length > i; i++) {
      bool isValidUri = true;
      switch ((*cur)) {
       case '%':
        for (int j = 0; 2 > j; j++) {
          cur += 1;
          i += 1;
          //DEBUG_cout << "j: " << j << " c: " << (*c) << endl;
          // read next two chars
          if ((*cur) >= 'a' && (*cur) <= 'z') {
            // To Upper Case
            two[j] = (*cur) - 32;
          } else {
            two[j] = (*cur);
          }

          if (two[j] >= 'A' && two[j] <= 'F') {
            // Hex Char to Int
            two[j] = two[j] - 65 + 10;
          } else if (two[j] >= '0' && two[j] <= '9') {
            two[j] = two[j] - 48;
          } else {
            // ERROR
            isValidUri = false;
            break;
          }
        }

        if (isValidUri == true) {
          f = two[0] * 16 + two[1];
        } else {
          f = ' ';
        }

        break;
       case '+':
        f = ' ';
        break;
       default:
        f = (*cur);
        break;
      }
      //DEBUG_cout << "f: " << f << endl;

      (*ins) = f;
      ins += 1;
      cur += 1;
      count += 1;
    }
    // Sets rest of data to 0.
    memset(ins, ' ', length - count);

    return count;
  }

  size_t TrimIncompleteUTF8(std::string& str) {
    // Scans backward from the end of std::string.
    //const char* begptr = &str.front();
    const char* cptr = &str.back();
    int num = 1;
    int numBytesToTruncate = 0;

    const int NUMLOOPS = str.length() > 4 ? 4 : str.length();

    for (int i = 0; i < NUMLOOPS; ++i) {
      if ((*cptr & 0x80) == 0x80) { // If char bit starts with 1xxxxxxx
        numBytesToTruncate += 1;
        // It's a part of unicode character!
        // Find the first byte in the unicode character!
        
        //if (((*cptr ^ 0xFC) & 0xFE) == 0x00) { if (num == 6) { return 0; } break; }
        //if (((*cptr ^ 0xF8) & 0xFC) == 0x00) { if (num == 5) { return 0; } break; }
        
        // If char binary is 11110000, it means it's a 4 bytes long unicode.
        if (((*cptr ^ 0xF0) & 0xF8) == 0x00) { if (num == 4) { return 0; } break; }
        if (((*cptr ^ 0xE0) & 0xF0) == 0x00) { if (num == 3) { return 0; } break; }
        if (((*cptr ^ 0xC0) & 0xE0) == 0x00) { if (num == 2) { return 0; } break; }  

        num += 1;
      } else {
        // If char bit does not start with 1, nothing to truncate!
        break;
      }

      cptr -= 1;
    }
    str.resize(str.length() - numBytesToTruncate);
    DEBUG_result << "Trimmed " << numBytesToTruncate << " incomplete UTF-8 bytes."<< endl;
    return numBytesToTruncate;
  }

  std::string JsonEncode(const std::string& str) {
    DEBUG_cerr << "JsonEncode is NOT FULLY IMPLEMENTED and TESTES. DONT USE IT FOR NOW." << endl;
    // #REF: http://stackoverflow.com/a/27516892/4694036
    std::string escaped;
    escaped.reserve(str.length() * 2);

    for (const auto& c : str) {
      switch (c) {
        case '\r':
        case '\n': escaped += "\\n";
                   break;
        case '\"': escaped += "\\\"";
                   break;
        case '\t': escaped += "\\t";
                   break;
        case '\\': escaped += "\\\\";
                   break;
        case '\b': break; // IGNORED
        case '\f': break; // IGNORED
        default:
          escaped += c;
          break;
      } 
    } 

    escaped.shrink_to_fit();
    return escaped;
  }

  bool IsSafeForJson(const std::string& str) {
    // Rename this function IsSafeJsonString() later
    // #REF: http://www.ietf.org/rfc/rfc4627.txt
#if 0
    if (str.back() == '\\') {
      DEBUG_cerr << "Cannot end with Backslash." << endl;
      return false;
    }
#endif

    bool result = true;
    for (size_t i = 0; i < str.length(); ++i) {
      const char* c = &str[i];
      switch (*c) {
        case '\n': 
        case '\r':
          DEBUG_result << "Detected newLine" << endl;
          result = false;
          break;
        case '"':
          DEBUG_result << "Detected double quotes" << c << endl;
          result = false;
          break;
        case '\t':
          DEBUG_result << "Detected tab." << endl;
          result = false;
          break;
        case '\b':
          DEBUG_result << "Detected \\b." << endl;
          result = false;
          break;
        case '\f':
          DEBUG_result << "Detected \\f" << c << endl;
          result = false;
          break;
        case '\\':
          switch (*(++c)) {
            case '\\':
            case 'n':
            case 'r':
            case 't':
            case '"':
              ++i;
              // Well escaped chars so Continue
              continue;
            default:
              DEBUG_result << "Invalid escaped char" << endl;
              result = false;
              break;
          }

        default:
          break;
      } 
      if (result == false) {
        break;
      }
    }

    if (result == false) {
      LOG_secwarn << "Json Validation has failed." << endl;
    }
    return result;
  }

  bool IsUserInputSafe(const std::string& str) {
    if (str.find("<") != std::string::npos) {
      return false;
    }

    if (str.find(">") != std::string::npos) {
      return false;
    }

    return true;
  }

  // THIS IS REALLY BAD AND DANGEROUS FUNCTION.
  // DON'T USE IT OTHER THAN TEMPORARY DEVELOPMENT PURPOSE.
  size_t Append(char* ptr, const std::string& str, size_t pos) {
    DEBUG_cerr << "This function is DANGEROUS and should not be used for Production." << endl; 
    ptr += pos;
    strncpy(ptr, str.c_str(), str.length());
    return pos + str.length();
  }
}

}


#if _UNIT_TEST

#include <iostream>

using namespace lio;

int main() {

  srand (time(NULL));
  std::string testStr = "abcABC .,;/";
  
  std::string upper = Util::String::ToUpper(testStr);
  std::string lower = Util::String::ToLower(testStr);

  UnitTest::Test<string>(upper, "ABCABC .,;/", "Upper Case");
  UnitTest::Test<string>(lower, "abcabc .,;/", "Lower Case");

  int negNumber = Util::String::To<int> ("-2000");
  cout << "negNumber: " << negNumber << endl;
  UnitTest::Test<int>(negNumber, -2000, "Negative Number Conversion");

  int8_t int8 = Util::String::To<int8_t>("10");
  cout << "int8: " << int8 << endl;
  UnitTest::Test<int8_t>(int8, -1, "int8_t");

  DataBlock<char*> db((char*)testStr.c_str(), 2, 7);
  ssize_t foundIndex = Util::String::Find("cAB", (char*) testStr.c_str(), 7);
  cout << "foundIndex: " << foundIndex << endl;
  UnitTest::Test<ssize_t>(foundIndex, 5, "FindString");

  cout << Util::String::RandomString(10) << endl;;
  cout << Util::String::RandomString(10) << endl;;

  UnitTest::ReportTestResult();

  cout << Util::TimeToString(std::chrono::system_clock::now());


  return 0;
}
#endif
#undef _DEBUG
#undef _UNIT_TEST


