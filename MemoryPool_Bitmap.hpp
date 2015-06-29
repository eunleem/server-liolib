#ifndef _MEMORYPOOL_HPP_
#define _MEMORYPOOL_HPP_

/**
 *  Class Name
 *    MemoryPool
 *   
 *  Class Designers
 *    [ELE] Eun Leem (eunleem@gmail.com)
 *    [JDO] John Doe (johndoe@example.com)
 *  
 *  Description
 *    Memory Pool.
 *      It reduces system calls such as malloc()
 *      It reduces memory leaks.
 *
 *    
 *
 *  
 *  History
 *    05/14/2013 - [ETL] Initial
 *    
 *  ToDos
 *
 *
 *  Milestones
 *    05/20/2013
 *    
 *
 *  Note
 *    chunk_header_t.exactSize is not really needed.
 *    Go ahead and remove it if you don't want to use it to reduce overhead cost for mpalloc.
 *
 *  Aliases Used
 *    ContentType = MIME Type
 *    Chunk
 *      Each chunk is created by each alloc request.
 *      Each chunk has chunk header.
 *      One chunk consists of multiple blocks.
 *       
 *    Chunk header
 *      Chunk header is located at the beginning of each chunk
 *      Contains information needed for 'free' operation.
 *      * Exact content size field is added to provide content size if needed.
 *  
 *  Learning Resources
 *    HTTP Protocol
 *      Http Request/Response Structure
 *        http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
 *
 *    MIME Type
 *      #REF: http://en.wikipedia.org/wiki/MIME_type
 *
 *    Http Response Status Code
 *      #REF: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
 */

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG true 

#include "Debug.hpp"

#include <iostream>
#include <bitset> // std::bitset<>

#include <cstdlib> // malloc(), free()
#include <cstdint> // uintptr_t, std::int8_t 
#include <cstring> // memcpy

#include "Util.hpp"

#include <exception>

namespace lio {
using std::cout;
using std::endl;

// #LEARN: Strongly Typed enums
//  This uses 1 byte int instead of 4 bytes int. Saves memory but max number of items is 128.
enum class MemoryPoolExceptionType : std::int8_t {
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

class MemoryPoolException : public std::exception {
public:
  MemoryPoolException(MemoryPoolExceptionType exceptionType = MemoryPoolExceptionType::GENERAL);

  virtual const char* what() const noexcept;
  virtual const MemoryPoolExceptionType type() const noexcept;
  
private:
  MemoryPoolExceptionType     exceptionType_;
  static const char* const    exceptionMessages_[];
};


// #LEARN: ptrdiff_t is to represent difference of two pointers
class MemoryPool {
public:

enum class Mode : std::uint8_t {
  LOCAL, // DEFAULT
  SHARED_MEMORY
};

struct Config {
  Config() 
    : mode(Mode::LOCAL),
      poolSize(1024*1024*10),
      blockSize(1024) {}
  Config (size_t poolSize,
          size_t blockSize = 1024,
          Mode mode = Mode::LOCAL)
    : mode(mode),
      poolSize(poolSize),
      blockSize(blockSize) {}

  Mode      mode;
  size_t    poolSize;
  size_t    blockSize;
};

// #RFCT: Ptr to Location? 
// Do not use special notation for pointers or references
// Prefixes like p for pointers or r for references, suffixes like Ptr for pointers et al. are discouraged as they emphasize technical nature of implementation and obscure the logical side. 
struct PoolHeader {
  size_t    poolSize;
  size_t    blockSize;
  size_t    freeSize;
  void*     blockBitMapPtr;
  size_t    blockBitMapSize;
  void*     poolHeaderPtr;
  void*     poolBodyPtr;
  void*     poolEndPtr;
};

/**
 * For maximum efficiency, unsigned int can be switched to something smaller.
 *  Left as unsigned int for maximum safety.
 */
struct ChunkHeader {
  static const char MAGIC_CHAR;

  ChunkHeader() : magicChar(MAGIC_CHAR) { }
  unsigned int firstBlockLocation; // Start index in nth Block. First block has index of 1, not 0.
  unsigned int numBlocksUsed; // Number of Blocks used

  // exactSize can be removed depending on how MemoryPool is used.
  size_t     contentSize; // Exact number of bytes used by the content without overhead space.

  const char magicChar;
};
 
  // IF MEMORY POOL IS DIRECTLY CREATED, IT IS MEANT TO BE RUNNING IN LOCAL MODE ONLY.
  MemoryPool (MemoryPool::Config& config);
  MemoryPool (const void* poolLocation, const Config& config);

  // PoolSize in Byte, BlockSize in Byte
  // #DEPRECATED
  //MemoryPool (const size_t poolSize, const size_t blockSize = 1024); 

  // #LEARN: Virtual Destructor
  //    http://www.programmerinterview.com/index.php/c-cplusplus/virtual-destructors/
  virtual
  ~MemoryPool ();
  
  void* const       Mpalloc(size_t allocSize);

  /**
   * Returns
   *  size of space freed in memory pool.
   *  Not really needed but better than returning uselee info.
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
  Mode              mode_; 
  Config            config_;

  const void*       poolLocation_;

  // poolHeader is placed on the beginning of newly allocated memory (at poolStartPtr).
  PoolHeader*       poolHeader_;
  
  void*             allocate (const int firstBlockLocation, const size_t numBlocksNeeded);
  int               findSpaceForChunk(const size_t numBlocksNeeded);
  bool              markBlockMap (const int firstBlockLocation,
                                  const int numBlocksToMark,
                                  const bool isMarkingToZero = false);

  // #TODO: ChunkHeader magic number to validate if its a right chunk header?
  ChunkHeader*      getChunkHeader(const void* chunkLocation) const;
  
};

}

#endif
