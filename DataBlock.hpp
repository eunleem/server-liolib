#ifndef _DATABLOCK_HPP_
#define _DATABLOCK_HPP_

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false

#include "liolib/Debug.hpp"

#include <iostream>
#include <string> // string

#include <cstring> // strlen() 
#include <cassert> // assert()


namespace lio {

using std::cout;
using std::cerr;
using std::endl;

using std::string;


template<class T = void*>
class DataBlock {
public:
  DataBlock() {
    DEBUG_cerr << "DataBlock should be used for pointer object only." << endl; 
    //assert(!"CANNOT BE USED.");
    throw std::exception();
  }

protected:
private:
};

template<>
class DataBlock<void*> {
public:
  friend bool operator== (DataBlock<void*>& lhs, DataBlock<void*>& rhs) {
    if (lhs.object == rhs.object &&
        lhs.index == rhs.index &&
        lhs.length == rhs.length) {
      return true;
    } else {
      return false;
    }
  }

  friend bool operator!= (DataBlock<void*>& lhs, DataBlock<void*>& rhs) {
    return !(lhs == rhs);
  }

  DataBlock<void*> & operator= (const DataBlock<void*>& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->object = rhs.object;
    this->index = rhs.index;
    this->length = rhs.length;

    return *this;
  }

  DataBlock<void*> & operator= (DataBlock<void*>&& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->object = rhs.object;
    this->index = rhs.index;
    this->length = rhs.length;

    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, const DataBlock<void*>& object) {
    os << "DataBlock<void*> object: " << std::hex << object.object << std::dec << " index: " << object.index << " length: " << object.length;
    return os;
  }

  DataBlock(const DataBlock<void*>& obj) :
    object(obj.GetObject()),
    index(obj.GetIndex()),
    length(obj.GetLength())
  { }

  DataBlock(void* baseObject = nullptr, size_t index = 0, size_t length = 0) :
    object(baseObject),
    index(index),
    length(length)
  { }
 
  void* GetObject() const {
    return this->object;
  }

  size_t GetIndex() const {
    return this->index;
  }

  size_t GetLength() const {
    return this->length;
  }


  bool SetObject(void* object) {
    this->object = object;
    return true;
  }

  bool SetIndex(size_t index) {
    this->index = index;
    return true;
  }

  bool SetLength(size_t length) {
    this->length = length;
    return true;
  }

  bool IsNull() const {
    if (this->object == nullptr &&
        this->index == 0 &&
        this->length == 0) {
        return true;
    } else {
      return false;
    }
  }

  void _PrintInfo() const {
    if (this->object == nullptr) {
      cout << "object: NULL" << endl;
    } else {
      cout << std::hex << "object: " << this->object << endl;
    }
    cout << std::dec << "index: " << this->index << endl;
    cout << std::dec << "length: " << this->length << endl;
    return;
  }

  void*   object;
  size_t  index; // off
  size_t  length;
protected:
private:
};

// Specialization for any pointer.
/*
template<class T>
class DataBlock<T *> : public DataBlock<void*> {
public:
  T GetObject() const override  {
    return this->object;
  }

  bool SetObject(T object) override {
    this->object = object;
    return true;
  }

  T object;
protected:
private:
};
*/

template<>
class DataBlock<string*> : public DataBlock<void*> {
public:
#if 0
  DataBlock<string*> & operator= (const DataBlock<string*>& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->object = rhs.object;
    this->index = rhs.index;
    this->length = rhs.length;

    return *this;
  }

  DataBlock<string*> & operator= (DataBlock<string*>&& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->object = rhs.object;
    this->index = rhs.index;
    this->length = rhs.length;

    return *this;
  }
#endif
  friend std::ostream& operator<<(std::ostream& os, const DataBlock<string*>& object) {
    os << (*object.object).substr(object.index, object.length);
    return os;
  }


  DataBlock(const DataBlock<string*>& obj) :
    DataBlock<void*>(nullptr, obj.index, obj.length),
    object(obj.GetObject())
  { }

  DataBlock(const string* baseObject = nullptr, size_t index = 0, size_t length = 0) :
    DataBlock<void*>(nullptr, index, length),
    object(baseObject)
  { }
  
  const string* GetObject() const {
    return this->object;
  }

  bool SetObject(const string* object) {
    this->object = object;
    return true;
  }

  bool IsNull() const {
    if (this->object == nullptr &&
        this->index == 0 &&
        this->length == 0) {
        return true;
    } else {
      return false;
    }
  }
  
  string GetValue() const {
    if (this->IsNull() == false) {
      size_t length = this->length;
      if (length == 0) {
        length = string::npos;
      } 
      return this->object->substr(this->index, this->length);
    } else {
      DEBUG_clog << "GetValue is called on empty DataBlock<string*>. Returning Empty string." << endl; 
      return "";
    }
  }

  bool IsSameString(const string& comparingStr) const {
    DEBUG_cerr << __func__ << " NOT FULLY IMPLEMENTED FUNCTION.." << endl; 
    if (this->length != comparingStr.length()) {
      return false;
    } 

    for (size_t i = 0; this->length > i; ++i) {
      if (object->at(i) != comparingStr.at(i)) {
        return false;
      }
    } 
    return true;

  }

  const string*       object;
protected:
private:

};


template<>
class DataBlock<char*> : public DataBlock<void*> {
public:
  friend std::ostream& operator<<(std::ostream& os, const DataBlock<char*>& object) {
    os << string(object.object).substr(object.index, object.length);
    return os;
  }

  DataBlock(const char* baseObject = nullptr, size_t index = 0, size_t length = 0) :
    DataBlock<void*>(nullptr, index, length),
    object(baseObject)
  { }

  const char* GetObject() const {
    return this->object;
  }

  bool SetObject(const char* object) {
    this->object = object;
    return true;
  }

  bool IsNull() const {
    if (this->object == nullptr &&
        this->index == 0 &&
        this->length == 0) {
        return true;
    } else {
      return false;
    }
  }
  
  string GetValue() const {
    if (this->IsNull() == false) {
      size_t length = this->length;
      if (length == 0) {
        length = string::npos;
      } 
      return string(this->object).substr(this->index, this->length);
    } else {
      DEBUG_clog << "GetValue is called on empty DataBlock<string*>. Returning Empty string." << endl; 
      return "";
    }
  }

  bool IsSameString(const char* comparingStr) const {
    DEBUG_cerr << __func__ << " NOT FULLY IMPLEMENTED FUNCTION.." << endl; 
    if (this->length != strlen(comparingStr)) {
      return false;
    } 

    for (size_t i = 0; this->length > i; ++i) {
      if (object[i] != comparingStr[i]) {
        return false;
      }
    } 
    return true;

  }

  const char*       object;
protected:
private:

};

typedef DataBlock<char*> PartialString;

}

#endif

