#ifndef _SERIALIZABLE_HPP_
#define _SERIALIZABLE_HPP_
/*
  Name
    Serializable

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    May 07, 2015
  
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


#include <ostream>
#include <istream>
#include <fstream>

//#include "liolib/DataBlock.hpp"

namespace lio {


class Serializable {
public:
  Serializable() { }
  virtual
  ~Serializable() { }

  friend std::ostream& operator<<(std::ostream& os, const Serializable& obj) {
    return obj.Serialize(os);
  }

  friend std::istream& operator>>(std::istream& is, Serializable& obj) {
    return obj.Deserialize(is);
  }

  virtual std::ostream& Serialize(std::ostream& stream) const = 0;
  virtual std::istream& Deserialize(std::istream& stream) = 0;
protected:
  
private:
  
};

}

#endif
