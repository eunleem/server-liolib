#ifndef _COLORS_HPP_
#define _COLORS_HPP_

#include <string>

namespace TCOLOR {
  const std::string BLACK    = "\033[1;30m";
  const std::string RED      = "\033[1;31m";
  const std::string GREEN    = "\033[1;32m";
  const std::string BROWN    = "\033[1;33m";
  const std::string YELLOW   = "\033[1;33m";
  const std::string BLUE     = "\033[1;34m";
  const std::string MAGENTA  = "\033[1;35m";
  const std::string CYAN     = "\033[1;36m";

  const std::string REDBG    = "\033[1;37;41m";
  const std::string YELLOWBG    = "\033[1;37;43m";
  


  const std::string RESET = "\033[0m";
  const std::string END = "\033[0m";
}

#endif

