#ifndef _MACRO_HPP_
#define _MACRO_HPP_

#include <string>

using std::string;

namespace COLOR {
  const string BLACK    = "\033[1;30m";
  const string RED      = "\033[1;31m";
  const string GREEN    = "\033[1;32m";
  const string BROWN    = "\033[1;33m";
  const string YELLOW   = "\033[1;33m";
  const string BLUE     = "\033[1;34m";
  const string MAGENTA  = "\033[1;35m";
  const string CYAN     = "\033[1;36m";

  const string END = "\033[0m";
}

#endif

