#ifndef _OPENABLE_HPP_
#define _OPENABLE_HPP_
/*
  Name
    Openable

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Jun 26, 2015
  
  History
    May 29, 2015
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
#define _DEBUG false

#include "liolib/Debug.hpp"


//#include "liolib/DataBlock.hpp"

namespace lio {


class Openable {
public:
  enum class Status : uint8_t {
    CLOSE,
    OPEN
  };

  Openable() :
    status_(Status::CLOSE)
  { }

  virtual
  ~Openable() {
    if (this->status_ == Status::OPEN) {
      DEBUG_cerr << "WARNING! Trying to Close the Openable class before Closing it properly." << endl; 
    } 

  }

  bool Open() {
    if (this->status_ == Status::OPEN) {
      DEBUG_cout << "Already Open." << endl; 
      return true;
    }

    bool result = this->open();
    if (result == true) {
      this->status_ = Status::OPEN;
      DEBUG_cout << "Openable object has been opened." << endl; 
    } else {
      DEBUG_cerr << "Coult not open." << endl; 
    }

    return result;
  }

  bool Close() {
    if (this->status_ == Status::CLOSE) {
      DEBUG_cout << "Already Closed." << endl; 
      return true;
    } 

    bool result = this->close();
    if (result == true) {
      this->status_ = Status::CLOSE;
      DEBUG_cout << "Openable object has been closed." << endl; 
    } else {
      DEBUG_cerr << "Coult not open." << endl; 
    }

    return result;
  }

protected:
  virtual
  bool open() = 0;

  virtual
  bool close() = 0;

  Status status_;

private:

};

}

#endif

