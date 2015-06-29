#ifndef _CONSTS_HPP_
#define _CONSTS_HPP_

#include <cstdlib> // ssize_t
#include <string> // string::npos

namespace lio {
  namespace consts {
    static const ssize_t ERROR = -1;
    static const ssize_t NOT_FOUND = -1;
    static const ssize_t OK = 0;
    static const size_t STRING_NOT_FOUND = std::string::npos;
  }
}


#endif
