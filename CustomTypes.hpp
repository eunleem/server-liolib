#ifndef _CUSTOMTYPES_HPP_
#define _CUSTOMTYPES_HPP_

#include <iostream>
#include <string> // string

#include <cstddef> // ptrdiff_t


namespace lio {

using std::cout;
using std::cerr;
using std::endl;

using std::string;

class DataBlock {
 public:
  friend bool operator== (const DataBlock& lhs, const DataBlock& rhs) {
    if (lhs.address == rhs.address &&
        lhs.relativePosition == rhs.relativePosition &&
        lhs.length == rhs.length) {
      return true;
    } else {
      return false;
    }
  }

  friend bool operator!= (DataBlock& lhs, DataBlock& rhs) {
    return !(lhs == rhs);
  }

  /*DataBlock & operator= (const DataBlock& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->address = rhs.address;
    this->relativePosition = rhs.relativePosition;
    this->length = rhs.length;

    return *this;
  }*/

  DataBlock(void* address = nullptr, ptrdiff_t relativePosition = 0, size_t length = 0)
    : address(address),
      relativePosition(relativePosition),
      length(length) { }

  bool IsNull() {
    if (this->address == nullptr &&
        this->relativePosition == 0 &&
        this->length == 0) {
        return true;
    } else {
      return false;
    }
  }

  void _PrintInfo() {
    cout << std::hex << "Address: " << address << endl;
    cout << std::dec << "relativePosition: " << relativePosition << endl;
    cout << std::dec << "length: " << length << endl;
    return;
  }

  void*         address;
  ptrdiff_t  relativePosition;
  size_t  length;
protected:
private:
};

}

#endif

