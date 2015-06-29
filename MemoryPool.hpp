#ifndef _MEMORYPOOL_HPP_
#define _MEMORYPOOL_HPP_

/**
  Class Name
    MemoryPool
   
  Class Designers
    [ETL] Eun T. Leem (eunleem@gmail.com)
    [JDO] John Doe (johndoe@example.com)
  
  Description
    Memory Pool - Bitmap
      It reduces system calls such as malloc()
      It reduces memory leaks.

  Last Modified Date
    Nov 13, 2014

  History
    Mar 28, 2014 - [ETL]
      Implemented lastOperationBlockAddress for faster allocation.
      It improved performance about 8 to 10%.
      Hmm but it creates huge fragmentation problem.
        Update lastOpAddress only when allocation helps with fragmentation problem.

    May 14, 2013 - [ETL]
      Created.
    
  ToDos


  Milestones
    05/20/2013
    

  Note
    chunk_header_t.exactSize is not really needed.
    Go ahead and remove it if you don't want to use it to reduce overhead cost for mpalloc.

  Aliases Used
    Chunk
      Each chunk is created by each alloc request.
      Each chunk has chunk header.
      One chunk consists of multiple blocks.
       
    Chunk header
      Chunk header is located at the beginning of each chunk
      Contains information needed for 'free' operation.
      * Exact content size field is added to provide content size if needed.
        It might be useful for containing raw char data.

  
  Learning Resources
    Profiling

 */

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false 

#include "liolib/Debug.hpp"

#include <bitset> // std::bitset<>
#include <exception> // std::out_of_range
#include <iostream>

#include <cstdint> // uintptr_t, std::int8_t 
#include <cstdlib> // malloc(), free()
#include <cstring> // memcpy

#include "liolib/Util.hpp"


namespace lio {
using std::cout;
using std::endl;


// #LEARN: ptrdiff_t is to represent difference of two pointers
class MemoryPool {
public:
// #LEARN: Strongly Typed enums
//  This uses 1 byte int instead of 4 bytes int. Saves memory but max number of items is 128.
enum class ExceptionType : std::int8_t {
  GENERAL,
  NOT_INITIALIZED,
  INIT_FAIL,
  ALLOC_FAIL,
  ALLOC_FAIL_NOT_ENOUGH_SPACE,
  ALLOC_FAIL_NO_CHUNK,
  FREE_FAIL, // Maybe not needed at all.
  INVALID_POINTER_OUT_OF_RANGE,
  INVALID_POINTER_NO_CHUNK
};
#define MEMORY_POOL_EXCEPTION_MESSAGES \
  "Memory Pool Exception has been thrown.", \
  "MemoryPool is not initialized. Call initMemoryPool first.", \
  "Initialization failed. System could not allocate memory for MemoryPool.", \
  "Allocation failed.", \
  "Allocation failed. Not enough space in the memory pool.", \
  "Allocation failed. Could not find large enough continuous space.", \
  "Free failed.", \
  "Invalid pointer. Out of memory pool range.", \
  "Invalid pointer. No chunk head found at given location."

class Exception : public std::exception {
public:
  Exception(ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char* what() const noexcept;
  virtual const ExceptionType type() const noexcept;
  
private:
  ExceptionType               exceptionType_;
  static const char* const    exceptionMessages_[];
};

enum class Mode : std::uint8_t {
  LOCAL, // DEFAULT
  SHARED_MEMORY
};

struct PoolHeader {
  Mode      mode;
  size_t    blockSize;
  size_t    poolSize;
  size_t    freeSize;
  void*     poolHeaderAddress;
  void*     blockBitmapAddress;
  size_t    blockBitmapSize;
  void*     poolBodyAddress;
  void*     poolEndAddress;
  void*     lastOperationBlockAddress; // Used to find free blocks fast.
};

static const char MAGIC_CHAR;

struct ChunkHeader {
  ChunkHeader() :
    firstBlockIndex(0),
    numBlocksUsed(0),
    contentSize(0),
    magicChar(MAGIC_CHAR)
  { }
  uint16_t firstBlockIndex; // Start index in nth Block. First block has index of 1, not 0.
  uint16_t numBlocksUsed; // Number of Blocks used

  // #OPTIMIZE: remove contentSize and magicChar later.
  uint32_t     contentSize; // Exact number of bytes used by the content without overhead space.

  char magicChar;
};
 
  // For Local Mode Only
  MemoryPool (size_t poolSize, size_t blockSize = 1024, MemoryPool* prev = nullptr);

  // For Shared Mode Only
  MemoryPool (void* poolLocation, size_t poolSize, size_t blockSize = 1024);

  // #LEARN: Virtual Destructor
  //  http://www.programmerinterview.com/index.php/c-cplusplus/virtual-destructors/
  virtual
  ~MemoryPool ();



  void* const       Mpalloc(size_t allocSize);

  // Alloced memory can be resized to smaller size later using AllocFit func.
  // It can be used to alloc memory for incoming data that you don't know the exact size.
  // First alloc a large enough memory, then after finish loading the data and figure out how much data actually uses the memory. Use fit function to return unused space to memory pool.
  size_t       MpAllocFit(const void* flexAllocedChunk, size_t contentSize);

  /* 
   * Returns
   *  size of space freed in memory pool.
   *  Not really needed but better than returning useless info.
   */
   
  size_t            Mpfree(const void* freePtr); // Returns 

  size_t            GetPoolSize() const;
  size_t            GetFreeSize() const;

  // Get size that is actually taking up in the memory pool.
  size_t            GetChunkSize(const void* chunkLocation) const;
  // Get exact size of the content.
  size_t            GetContentSize(const void* chunkLocation) const;

  // Test Functions  
  void              _PrintBlockMap() const;
  void              _PrintPoolInfo() const;
  
protected:
  
private:
  MemoryPool*       prev_;
  MemoryPool*       next_;

  MemoryPool*       getPrevMp();
  MemoryPool*       getNextMp();

  MemoryPool*       createNextMp();

  // poolHeader is placed on the beginning of newly allocated memory (at poolStartPtr).
  PoolHeader*       poolHeader_;

  bool              createPoolHeader(size_t poolSize, size_t blockSize, Mode mode = Mode::LOCAL, void* poolAddress = nullptr);
  
  //void*             allocate (const int index, const size_t count);
  ssize_t           findSpaceForChunk(const size_t numBlocksNeeded);
  ssize_t           findSpaceForChunk(const size_t numBlocksNeeded,
                                      void* searchStartBitmapBlockAddress,
                                      size_t numBitmapBlocksToScan);
  bool              markBlockMap (const int index,
                                  const int count,
                                  const bool isMarkingToZero = false);

  ChunkHeader*      getChunkHeader(const void* chunkLocation) const;
  
};

}

#endif
