#pragma once
/*
  Name
    Serializable

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Apr 13, 2015
  
  History
    April 12, 2015
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


#include <string>
#include <ostream>
#include <istream>
#include <fstream>

//#include "liolib/DataBlock.hpp"

namespace lio {

class String : public std::string {
public:
  String() { }
  virtual
  ~String() { }

  friend std::fstream& operator<<(std::fstream& out, const Serializable& obj) {
    obj.Serialize(out);
    return out;
  }

  friend std::fstream& operator>>(std::fstream& in, Serializable& obj) {
    obj.Deserialize(in);
    return in;
  }

  virtual std::ostream& Serialize(std::ostream& stream) const = 0;
  virtual std::istream& Deserialize(std::istream& stream) = 0;


protected:
  
private:
  
};

}
