#ifndef _CUSTOMEXCEPTION_HPP_
#define _CUSTOMEXCEPTION_HPP_

#include <exception>

using std::exception;
/**
 * Custom Exceptions Created by LifeInO ETL
 *
 * Reference: http://www.cplusplus.com/doc/tutorial/exceptions/
 *
 * TODOs
 *
 */


class NotYetImplementedException : public exception {
public:
  virtual const char* what() const noexcept {
    return "This method is not yet implemented.";
  }
};

class InvalidArgumentException : public exception {
public:
  virtual const char* what() const noexcept {
    return "Invalid Parameter has been used.";
  }
};

class OverSizeException : public exception {
public:
  virtual const char* what() const noexcept {
    return "Size is over limit.";
  }
};

class OutOfRangeException : public exception {
public:
  virtual const char* what() const noexcept {
    return "Out of range.";
  }
};

class FileNotFoundException : public exception {
public:
  virtual const char* what() const noexcept {
    return "File Not Found.";
  }
};

class FileOpenFailedException : public exception {
public:
  virtual const char* what() const noexcept {
    return "Failed to open file.";
  }
};

class ItemNotFoundException : public exception {
public:
  virtual const char* what() const noexcept {
    return "Item Not Found.";
  }
};


#endif
