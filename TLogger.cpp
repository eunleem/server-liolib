#include <chrono>
#include <iostream>
#include <iomanip>

#include <string>
#include <cstring>

#include "Colors.hpp"

#include <sys/types.h>
#include <unistd.h>


class Logger {
public:
  enum class Type {
    INFO,
    WARNING,
    ERROR,
    FATAL
  };
  static
  std::ostream& Log(Type type = Type::INFO) {

    
    return std::cout << TCOLOR::BLUE << Logger::ToString(type)
                     << Logger::Timestamp("%F %T %Z", false) << std::setw(6)
                     << std::right << getpid() << " ";
  }

  inline static
  std::string ToString(Type type) {
     switch (type) {
       case Type::INFO: return "INF ";
       case Type::WARNING: return "WRN ";
       case Type::ERROR: return "ERR ";
       case Type::FATAL: return "FAT ";
     }
     return "UNF ";
  }

  static
  std::string Timestamp(const std::string& format = "%F %T %Z", bool in_localtime = true) {
    std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    struct tm* time;
    std::string buffer(28, '\0');
    if (in_localtime == true) {
      time = std::localtime(&rawTime);
    } else {
      time = std::gmtime(&rawTime);
    }

    // Time to Formatted String
    //   http://www.cplusplus.com/reference/ctime/strftime/   
    //   %F = yyyy-MM-dd
    //   %T = HH:mm:ss
    //   %Z = GMT or PST (Time Zone)
    strftime ((char*)buffer.c_str(), 28, format.c_str(), time);

    const std::size_t len = std::strlen(buffer.c_str());
    buffer.resize(len);
    return buffer;
  }
};

#define LOG                                                                    \
  Logger::Log() << std::setw(30) << std::left << __FILE__ << ":"               \
                << std::setw(5) << std::right << __LINE__ << " : "             \
                << TCOLOR::END

int main() {
  LOG << "DEBUG <ESSAGe" << std::endl;
  std::cout << LOG_ERR << std::endl;
  return 0;
}
