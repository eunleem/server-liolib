#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <cassert>
#include <cstring>

#include <sys/types.h> // pid_t
#include <unistd.h> // getpid();

//#include <errno.h>

#include "liolib/Colors.hpp"

namespace lio {
/*
 *  LOG: Need to be printed no matter the situation.
 *  LOG_INFO: Function return result, status,
 *  LOG_WARN: May happen time to time. Might be a sign of failure in the future. Function returning fail.
 *  LOG_SEC: Security warning. Someone trying to mess up with the system.
 *  LOG_ERR: Should not happen and it may cause a task to fail but program can continue running.
 *  LOG_ALERT: Needs Attention (Not really sure where to use this)
 *  LOG_FATAL: Crashes the program right away.
 */

  class Logger {
  public:
    enum class Type {
      LOG,
      INFO,
      SECURITY_INFO,
      WARNING,
      SECURITY_WARNING,
      ERROR,
      ALERT,
      FATAL
    };

    static
    std::ostream& Log(Type type = Type::INFO) {
      return getStream(type) << getColor(type) << Logger::toString(type)
                             << TCOLOR::RESET << TCOLOR::BLUE << " "
                             << Logger::Timestamp("%F %T %Z", false)
                             << std::setw(6) << std::right << getpid() << " ";
    }

    static
    bool RedirecTo(const std::string& filePath) {
      Logger::file.open(filePath);
      if (Logger::file.is_open() == false) {
        Logger::Log(Type::ERROR) << "Logger::RedirectTo() Could not open file." << std::endl;
        return false;
      }

      Logger::Log(Type::INFO)
          << "Redirecting Log to file. filePath: " << filePath << std::endl;

      std::cout.rdbuf(Logger::file.rdbuf());
      std::cerr.rdbuf(Logger::file.rdbuf());
      std::clog.rdbuf(Logger::file.rdbuf());

      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

      Logger::Log(Type::INFO) << "Log Redirected to file." << std::endl;

      return true;
    }

  private:
    static
    std::ofstream file;

    inline static
    std::ostream& getStream(Type type) {
       switch (type) {
         case Type::LOG: return std::clog;
         case Type::INFO: return std::cout;
         case Type::SECURITY_INFO: return std::clog;
         case Type::WARNING: 
         case Type::SECURITY_WARNING: return std::cout;
         case Type::ERROR:
         case Type::ALERT:
         case Type::FATAL: return std::cerr;

       }
       return std::cout;
    }

    inline static
    std::string getColor(Type type) {
       switch (type) {
         case Type::LOG:
         case Type::INFO:
         case Type::SECURITY_INFO:
           return TCOLOR::BLUE;
         case Type::WARNING: 
         case Type::SECURITY_WARNING:
           return TCOLOR::YELLOWBG;
         case Type::ERROR:
           return TCOLOR::RED;
         case Type::ALERT:
         case Type::FATAL:
           return TCOLOR::REDBG;
         default: break;

       }
       return TCOLOR::BLUE;
    }


    inline static
    std::string toString(Type type) {
      switch (type) {
        case Type::LOG: return "LOG";
        case Type::INFO: return "INF";
        case Type::SECURITY_INFO: return "SEC";
        case Type::WARNING: return "WRN";
        case Type::SECURITY_WARNING: return "SCW";
        case Type::ERROR: return "ERR";
        case Type::ALERT: return "ALR";
        case Type::FATAL: return "FAT";
      }
      return "LOG ";
    }

  public:
    static
    std::string Timestamp(const std::string& format = "%F %T %Z", bool in_localtime = false) {
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      std::time_t rawTime = std::chrono::system_clock::to_time_t(now);

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

      // Add milliseconds?
      return buffer;
    }
  };
}


#ifndef _FILE_LINE_INFO
#define _FILE_LINE_INFO                                                        \
  std::setw(30) << std::left << __FILE__ << ":" << std::setw(5) << std::right  \
                << __LINE__ << " : " << TCOLOR::END
#endif

#define _LOGFUNC(x) lio::Logger::Log(x)

#define LOG_log _LOGFUNC(lio::Logger::Type::LOG) << _FILE_LINE_INFO

#define LOG_info _LOGFUNC(lio::Logger::Type::INFO) << _FILE_LINE_INFO

#define LOG_secinfo _LOGFUNC(lio::Logger::Type::SECURITY_INFO) << _FILE_LINE_INFO

#define LOG_warn _LOGFUNC(lio::Logger::Type::WARNING) << _FILE_LINE_INFO

#define LOG_secwarn                                                            \
  _LOGFUNC(lio::Logger::Type::SECURITY_WARNING) << _FILE_LINE_INFO

#define LOG_err _LOGFUNC(lio::Logger::Type::ERROR) << _FILE_LINE_INFO

#define LOG_alert _LOGFUNC(lio::Logger::Type::ALERT) << _FILE_LINE_INFO

#define LOG_fatal _LOGFUNC(lio::Logger::Type::FATAL) << _FILE_LINE_INFO


#endif
