#ifndef _GZIP_HPP_
#define _GZIP_HPP_
/*
  Name
    Gzip

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    Feb 19, 2015
  
  History
    November 23, 2013
      Created

  ToDos
    02-19-2015
      Add Config and Option not to use memory pool.

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

#include <iostream>

#include <cstdio>
#include <cstring>
#include <cassert>

#include "liolib/MemoryPool.hpp"

#include "liolib/DataBlock.hpp" // DataBlock

#include "include/zlib.h"

namespace lio {

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

class Gzip {
public:
// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL,
  COMPRESSION_FAIL,
  DECOMPRESSION_FAIL
};
#define GZIP_EXCEPTION_MESSAGES \
  "Gzip Exception has been thrown.", \
  "Failed to compress.", \
  "Failed to decompress"

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType       exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********

  struct Config {
    Config() :
      useMemoryPool(false),
      memoryPoolSize(1024 * 1024 * 3)
    {

    }
    bool useMemoryPool;
    size_t memoryPoolSize;
  };

  Gzip(Config config = Config());
  ~Gzip();

  DataBlock<> Compress(const void* source, size_t length, int level = Z_DEFAULT_COMPRESSION);
  DataBlock<> Decompress(const void* source, size_t length);

  ssize_t Compress(const void* source, size_t length, char* dest, size_t maxSize,
      int level = Z_DEFAULT_COMPRESSION);
  ssize_t Decompress(const void* source, size_t length, char* dest, size_t maxSize);



protected:
  
private:
  Config config_;

  MemoryPool* mp_;

  static
  const size_t CHUNK_SIZE;

  static
  const int WINDOWS_BITS;

  static
  const int GZIP_ENCODING;
  
};

}

#endif

