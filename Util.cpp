#include "Util.hpp"

#define _UNIT_TEST false

#include "liolib/Test.hpp"

namespace Util {
using std::string;

bool IsDirectoryExistent( const string& dirPath ) {
  DEBUG_cerr << "DEPRECATED! Use Util::File::IsDirectoryExisting instead." << endl; 
  return File::_isDirectoryExisting( dirPath.c_str() );
}

bool IsDirectoryExisting( const char* dirPath ) {
  DEBUG_cerr << "DEPRECATED! Use Util::File::IsDirectoryExisting instead." << endl; 
  return File::_isDirectoryExisting( dirPath );
}

bool IsDirectoryExisting ( const string& dirPath ) {
  DEBUG_cerr << "DEPRECATED! Use Util::File::IsDirectoryExisting instead." << endl; 
  return File::_isDirectoryExisting( dirPath.c_str() );
}

bool IsFileExistent (const std::string& filePath, bool ifNotCreate) {
  DEBUG_cerr << "DEPRECATED! Use Util::File::IsFileExisting instead." << endl; 
  return File::_isFileExisting(filePath.c_str(), ifNotCreate);
}

bool IsFileExisting ( const char* filePath, bool ifNotCreate ) {
  DEBUG_cerr << "DEPRECATED! Use Util::File::IsFileExisting instead." << endl; 
  return File::_isFileExisting(filePath, ifNotCreate);
}

bool IsFileExisting ( const string& filePath, bool ifNotCreate ) {
  DEBUG_cerr << "DEPRECATED! Use Util::File::IsFileExisting instead." << endl; 
  return File::_isFileExisting(filePath.c_str(), ifNotCreate);
}


/*

  http://linux.die.net/man/3/ftok
  2nd parameter of ftok() accepts INT type but only uses least significant 8 bits (1 byte).
  
*/
key_t GenerateUniqueKey ( string& keyFilePath, int projId ) {
  
  // #TODO: Check whether the file exists or not.
  if ( projId > 128 || projId <= 0) {
    // Minor Error. Show Warning or leave a warning Log.
    // Doesn't have to stop the program tho.
    DEBUG_cerr << "ProjId must be less than 128 and greater than 0." << endl; 
    throw;
  }
  return ftok ( keyFilePath.c_str(), projId );
}

std::string Timestamp() {
  DEBUG_cerr << "DEPRECATED. Use Util::Time::Timestamp() instead." << endl; 
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
 
  return string(buffer);
}

std::string TimeToString(std::chrono::system_clock::time_point tp) {
  DEBUG_cerr << "DEPRECATED. Use Util::Time::TimeToString(arg) instead." << endl; 
  return Util::Time::TimeToString(tp);
}

namespace Test {
  size_t RandomNumber(size_t max, size_t min) {
    return rand() % max + min;
  }
}

namespace File {
  ssize_t GetSize(std::fstream& file) {
    if (file.is_open() == false) {
      DEBUG_cerr << "Error" << endl; 
      return -1;
    } 

    auto orgPos = file.tellg();
    file.seekg(0, std::ios::end);
    auto pos = file.tellg();
    file.seekg(orgPos, std::ios::beg);
    return static_cast<ssize_t>(pos);
  }

  bool IsDirectoryExisting ( const string& dirPath ) {
    return File::_isDirectoryExisting( dirPath.c_str() );
  }

  bool IsFileExisting ( const string& filePath, bool ifNotCreate ) {
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
          DEBUG_cout << "File Created." << endl; 
          return true;
        } else {
          DEBUG_cerr << "Could not create file." << endl; 
          return false;
        }
      } 
    } 

    return isExistent;
  }

  bool CreateDirectory(std::string path) {
    int result = mkdir(path.c_str(), S_IRWXU | S_IRWXG);
    if (result == -1) {
      DEBUG_cerr << "Failed to create directory. errno: " << errno << " path: " << path << endl;
      return false;
    } 

    return true;
  }

  bool Rename(std::string oldName, std::string newName) {
    bool isExisting = File::_isFileExisting(oldName.c_str());
    if (isExisting == false) {
      DEBUG_cerr << "File does not exist. Rename Failed." << endl; 
      return false;
    } 

    int result = std::rename(oldName.c_str(), newName.c_str());
    if (result != 0) {
      DEBUG_cerr << "Rename Failed. errno: " << errno << endl; 
      return false;
    } 

    return true;
  }

  bool Remove(std::string filePath) {
    bool isExisting = File::_isFileExisting(filePath.c_str());
    if (isExisting == false) {
      DEBUG_cerr << "File does not exist. Remove Failed." << endl; 
      return false;
    } 

    int result = std::remove(filePath.c_str());
    if (result != 0) {
      DEBUG_cerr << "Remove failed. errno: " << errno << endl; 
      return false;
    } 

    return true;
  }

  ssize_t WriteString(std::ostream& os, const std::string& str) {
    uint32_t strlen = str.length();
    os.write((char*)&strlen, sizeof(strlen));
    os.write(str.c_str(), strlen);

    return strlen;
  }

  ssize_t ReadString(std::istream& is, std::string& str) {
    uint32_t strlen = 0;
    is.read((char*)&strlen, sizeof(strlen));
    str.resize(strlen);
    is.read((char*)str.c_str(), strlen);

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
      DEBUG_cerr << "Could not convert to datetime." << endl; 
      throw std::exception();
    } 

    return std::chrono::system_clock::from_time_t(rawTime);
  }

  std::string Timestamp(const string& format) {
    return TimeToString(std::chrono::system_clock::now(), format);
  }

  std::string TimestampNum() {
    return TimeToString(std::chrono::system_clock::now(), "%Y%m%d%H%M%S");
  }

  std::string TimeToString(const steadytime tp, const string& format) {
    std::time_t rawTime = Time::steady_clock_to_time_t(tp);
    return TimeToString(rawTime, format);
  }

  time_t steady_clock_to_time_t(const std::chrono::steady_clock::time_point t ) {
      return std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now() +
          (t - std::chrono::steady_clock::now()));
  }

  std::string TimeToString(const datetime tp, const std::string& format) {
    std::time_t rawTime = std::chrono::system_clock::to_time_t(tp);
    return TimeToString(rawTime, format);
  }

  std::string TimeToString(const time_t rawTime, const std::string& format) {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&rawTime), format.c_str());
    return ss.str();
  }

}

namespace String {


  unsigned int ToUInt (const string &value) {
    return To<unsigned int>(value);
  }

  unsigned int ToUInt (const char* value) {
    return To<unsigned int>(value);
  }

  ssize_t Find (const string& toFind, lio::DataBlock<char*>& block) {
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

  ssize_t Find (const string& toFind, char* location, size_t length) {
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

  bool Compare (char* location, size_t length, const string& str) {
    size_t toFindIndex = 0;
    for (size_t i = 0; length > i; ++i) {
      if ((*location) == str[toFindIndex]) {
        toFindIndex += 1;
        if (toFindIndex == str.length()) {
          return true;
        } 
      } else if ((*location) == '\0') {
        // End of String reached and Not Found.
        return false;
      } else {
        toFindIndex = 0;
      }
      location += 1;
    } 
    return false;
  }

  std::string Retrieve (char* location, size_t length) {
    DEBUG_cerr << "DEPRECATED. Use string constructor string(char*, length) instead." << endl; 
    return std::string(location, length);
  }

  std::string RetrieveBetween(char* startLocation, const string& endStr, size_t lengthMax)
  {
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
  
  bool ToUpperFly (string& value) {
    size_t length = value.length();
    for (unsigned int i = 0; length > i; i++) {
      if (value[i] >= 'a' && value[i] <= 'z') {
        value[i] = static_cast<char>( (int) value[i] - 32 );
      }
    }
    return true;
  }

  bool ToUpperFly (char* ptr, size_t length) {
    int count = 0;
    for (unsigned int i = 0; length > i; i++) {
      if ((*ptr) >= 'a' && (*ptr) <= 'z') {
        (*ptr) = static_cast<char>( (int) (*ptr) - 32 );
        ++count;
      }
      ptr += 1;
    }
    return count > 0;
  }

  bool ToLowerFly (string& value) {
    size_t length = value.length();
    for (unsigned int i = 0; length > i; i++) {
      if (value[i] >= 'A' && value[i] <= 'Z') {
        value[i] = static_cast<char>( (int) value[i] + 32 );
      }
    }
    return true;
  }

  bool ToLowerFly (char* ptr, size_t length) {
    for (unsigned int i = 0; length > i; i++) {
      if ((*ptr) >= 'A' && (*ptr) <= 'Z') {
        (*ptr) = static_cast<char>( (int) (*ptr) + 32 );
      }
      ptr += 1;
    }
    return true;
  }


  string ToUpper (const string& value) {
    size_t length = value.length();

    string upperCase;
    upperCase.reserve(length);

    for (unsigned int i = 0; length > i; i++) {
    if (value[i] >= 'a' && value[i] <= 'z') {
      upperCase += static_cast<char>( value[i] - 32 );
    } else {
      upperCase += value[i];
    }
    }

    return upperCase;
  }

  string ToLower (const string& value) {
    size_t length = value.length();

    string lowerCase;
    lowerCase.reserve(length);

    for (unsigned int i = 0; length > i; i++) {
      if (value[i] >= 'A' && value[i] <= 'Z') {
        lowerCase += static_cast<char>( value[i] + 32 );
      } else {
        lowerCase += value[i];
      }
    }

    return lowerCase;
  }

  vector<string> Tokenize(const string& source, const string& delimiter) {
    vector<string> result;

    if (delimiter.empty()) {
      return result;
    }
    size_t delimiterPosition = 0;
    size_t prevDelimiterPosition = 0;

    //int count = 0;

    while (true) {
      size_t sectionLength = 0;

      delimiterPosition = source.find(delimiter, prevDelimiterPosition);
      if (delimiterPosition == string::npos) {
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
            string section = source.substr(prevDelimiterPosition, sectionLength);
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
          string section = source.substr(prevDelimiterPosition, sectionLength);
          result.push_back(section);
          break;
        } else if (delimiterPosition + delimiter.length() > source.length()) {
          cerr << "Umm..... nonono this shouldn't happen" << endl;
          break; 
        }

        if (sectionLength > 0) {
          // CASE B
          string section = source.substr(prevDelimiterPosition, sectionLength);
          result.push_back(section);
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        } else if (sectionLength == 0) {
          // CASE C
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        } else {
          cerr << "Umm this shouldn't happen." << endl;
          break;
        }

      }
    }
    return result;
  }

  bool CaseInsensitiveCompare (const string& rvalue, const string& lvalue) {
    if (rvalue.length() != lvalue.length()) {
      return false;
    }

    string lowerRvalue = ToLower(rvalue);
    string lowerLvalue = ToLower(lvalue);
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
    const char* pool = nullptr;
    switch (complexity) {
      case 0:
        pool = random_char_numbers;
        break;
      case 1:
        pool = random_char_alphaupper;
        break;
      case 2:
        pool = random_char_alphauppernum;
        break;
      case 3:
        pool = random_char_alpha;
        break;
      case 4:
        pool = random_char_alphanum;
        break;
      case 5:
        pool = random_char_alphanum_special;
        break;
      default:
        DEBUG_cerr << "Complexity cannot be greater than 4, less than 0. Complexity Set to 3." << endl; 
        pool = random_char_alpha;
        break;
    } 

    const unsigned int stringPoolLength = strlen(pool);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; length > i; i++) {
      result += pool[rand() % stringPoolLength];
    } 

    return result;
  }

  void RandomString(char* dest, const size_t length, const unsigned int complexity) {
    const char* pool = nullptr;
    switch (complexity) {
      case 0:
        pool = random_char_numbers;
        break;
      case 1:
        pool = random_char_alphaupper;
        break;
      case 2:
        pool = random_char_alphauppernum;
        break;
      case 3:
        pool = random_char_alpha;
        break;
      case 4:
        pool = random_char_alphanum;
        break;
      case 5:
        pool = random_char_alphanum_special;
        break;
      default:
        DEBUG_cerr << "Complexity cannot be greater than 4, less than 0. Complexity Set to 3." << endl; 
        pool = random_char_alpha;
        break;
    } 
    const unsigned int stringPoolLength = strlen(pool);
    for (size_t i = 0; length > i; i++) {
      dest[i] =  pool[rand() % stringPoolLength];
    } 
  }


  bool UriDecode (const void* address, const size_t length, string* decoded) {
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

  string* UriDecode (const void* address, const size_t length) {
    string* decoded = new string();
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

  string UriDecode (const string& uri) {
    string decoded;
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

  // THIS IS REALLY BAD AND DANGEROUS FUNCTION.
  // DON'T USE IT OTHER THAN TEMPORARY DEVELOPMENT PURPOSE.
  size_t Append(char* ptr, const string& str, size_t pos) {
    PROD {
      //DEBUG_cerr << "This function is DANGEROUS and should not be used for Production." << endl; 
    }
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
  string testStr = "abcABC .,;/";
  
  string upper = Util::String::ToUpper(testStr);
  string lower = Util::String::ToLower(testStr);

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


