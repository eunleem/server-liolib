#ifndef _JSONSTRING_HPP_
#define _JSONSTRING_HPP_
/*
  Name
    JsonString

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Feb 23, 2015
  
  History
    February 21, 2015
      Created

  ToDos
    


  Milestones
    1.0
      

  Learning Resources
    http://
  
  Copyright (c) All rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <string>
#include <chrono>


#include "liolib/Util.hpp"
//#include "liolib/DataBlock.hpp"

namespace lio {

typedef std::chrono::system_clock::time_point datetime;

class JsonString : public std::string {
public:
  JsonString(size_t reserveSize = 100) {
    this->reserve(reserveSize);
  }

  JsonString& AddKey(const string& key) {
    this->append("\"" + key + "\":");
    return *this;
  }
  template<class T>
  JsonString& AddValue(T val) {
    this->append(std::to_string(val));
    return *this;
  }

  JsonString& AddStringValue(const string& value) {
    this->append("\"" + value + "\"");
    return *this;
  }

  JsonString& AddDatetime(const datetime& tp) {
    uint64_t millisecs = Util::Time::GetMillisecondsSinceEpoch(tp);
    this->append(std::to_string(millisecs));
    return *this;
  }


protected:
  
private:

  
};

}

#endif

