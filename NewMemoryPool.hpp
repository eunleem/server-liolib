#ifndef _NEWMEMORYPOOL_HPP_
#define _NEWMEMORYPOOL_HPP_
/*
  Name
    NewMemoryPool

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    Mar 27, 2014
  
  History
    March 08, 2014
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
#define _UNIT_TEST false

//#include "Debug.hpp"
#include "liolib/Debug.hpp"
#include "liolib/Test.hpp"

#include <iostream>
#include <vector>
#include <list>

#include <cstdlib> // malloc(), free()
#include <cstring> // memcpy, memset

namespace lio {

using std::vector;
using std::list;

enum class MpType {
  BITMAP,
  LIST
};

template<MpType TYPE = MpType::BITMAP>
class NewMemoryPool {
public:

  enum class Mode : std::uint8_t {
    LOCAL,
    SHARED_MEMORY
  };

  struct Config {
    Config(size_t poolSize = 1024 * 1024, Mode mode = Mode::LOCAL)
      : mode(mode),
        poolSize(poolSize) {}

    Mode mode;
    size_t poolSize;
  };

  enum class ChunkType : std::uint8_t {
    FREE,
    USED
  };

  struct ChunkHeader {
    ChunkHeader()
      : magic_number(37173),
        type(ChunkType::FREE),
        size(0),
        prev(nullptr),
        next(nullptr) {}
    uint16_t magic_number;
    ChunkType type;
    //ptrdiff_t location;
    size_t size;
    ChunkHeader* prev;
    ChunkHeader* next;
  };

  struct PoolHeader {
    PoolHeader() {}
    Config config;
    size_t freeSize;
    uintptr_t poolBeginning;
    uintptr_t poolEnd;
    ChunkHeader* freeChunksHead;
    ChunkHeader* usedChunksHead;
  };



  NewMemoryPool(const Config& config, const void* poolLocation = nullptr);
  virtual
  ~NewMemoryPool();

  void* Mpalloc(size_t size);
  bool Mpfree(void* location);
  
  void printFreeChunks() const;
  void printUsedChunks() const;
protected:
  
private:
  PoolHeader* poolHeader_;


};

}

#endif

