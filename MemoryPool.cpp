#include "MemoryPool.hpp"

namespace lio {

#define _UNIT_TEST false
#include "liolib/Test.hpp"

// ===== Exception Implementation =====
const char* const
MemoryPool::Exception::exceptionMessages_[] = {
  MEMORY_POOL_EXCEPTION_MESSAGES
};
#undef MEMORY_POOL_EXCEPTION_MESSAGES

MemoryPool::Exception::Exception(ExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
MemoryPool::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const MemoryPool::ExceptionType
MemoryPool::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


const bool TOZERO = true;
const char MemoryPool::MAGIC_CHAR = 'C';


MemoryPool::MemoryPool (size_t poolSize, size_t blockSize, MemoryPool* prev) :
  prev_(prev),
  next_(nullptr)
{
  DEBUG_FUNC_START;

  this->createPoolHeader(poolSize, blockSize);
}

MemoryPool::MemoryPool (void* poolAddress, size_t poolSize, size_t blockSize)
{
  DEBUG_FUNC_START;
  DEBUG_cout << "PoolHeader: " << std::hex << poolAddress << std::dec << endl; 

  this->createPoolHeader(poolSize, blockSize, Mode::SHARED_MEMORY, poolAddress);
}

MemoryPool::~MemoryPool () {
  DEBUG_FUNC_START;

  if (this->next_ != nullptr) {
    delete this->next_;
  } 

  if (this->poolHeader_ != nullptr) {
    if (this->poolHeader_->mode == Mode::LOCAL) {
      free (this->poolHeader_->poolHeaderAddress);
    } else {
      // SHARED MEMORY Module will Detach Shared Memory.
    }
  } 
}


size_t MemoryPool::GetPoolSize() const {
  return this->poolHeader_->poolSize;
}

size_t MemoryPool::GetFreeSize() const {
  return this->poolHeader_->freeSize;
}

size_t MemoryPool::GetContentSize(const void* chunkLocation) const {
  // Get Chunk Header which contatins Allocated Memory Chunk Information
  const ChunkHeader* chunkHead = this->getChunkHeader(chunkLocation);
  
  return (size_t) chunkHead->contentSize;
}

size_t MemoryPool::GetChunkSize(const void* chunkLocation) const {
  uintptr_t validRangeStart = (uintptr_t) this->poolHeader_->poolBodyAddress;
  uintptr_t validRangeEnd = (uintptr_t) this->poolHeader_->poolHeaderAddress + (uintptr_t) this->poolHeader_->poolSize;

  if ((uintptr_t) chunkLocation < validRangeStart ||
      validRangeEnd < (uintptr_t) chunkLocation) // [ETL] Pay attention to the order of variables written.  
  { 
    if (this->next_ != nullptr) {
      return this->GetChunkSize(chunkLocation);
    } 
    throw MemoryPool::Exception(ExceptionType::INVALID_POINTER_OUT_OF_RANGE);  
  }
  // Get Chunk Header which contatins Allocated Memory Chunk Information
  const ChunkHeader* chunkHead = this->getChunkHeader(chunkLocation);
  return (size_t) chunkHead->numBlocksUsed * this->poolHeader_->blockSize;
}


//#TODO: OPTIMIZE THIS FUNCTION
void* const MemoryPool::Mpalloc(size_t allocSize) {
  DEBUG_FUNC_START;
  DEBUG_cout << "AllocSize: " << allocSize << endl;
  if (this->poolHeader_->poolHeaderAddress == nullptr) {
    // ERROR
    throw MemoryPool::Exception(ExceptionType::NOT_INITIALIZED);
  }
  // 0. Add header size to to toAllocSize
  allocSize += sizeof (ChunkHeader);

  if (this->poolHeader_->poolSize < allocSize) {
    // TOO BIG and it is never possible to allocate memory.
    throw MemoryPool::Exception(ExceptionType::ALLOC_FAIL_NOT_ENOUGH_SPACE);
  } 

  // 1. Check if the Pool has enough Space.
  if (this->poolHeader_->freeSize < allocSize) {
    if (this->next_ == nullptr) {
      this->next_ = this->createNextMp();
    } 
    return this->next_->Mpalloc(allocSize);
    throw MemoryPool::Exception(ExceptionType::ALLOC_FAIL_NOT_ENOUGH_SPACE);
  }
  
  size_t numBlocksNeeded = (allocSize / this->poolHeader_->blockSize);
  if ((allocSize % this->poolHeader_->blockSize) > 0) {
    numBlocksNeeded++;
  }

  DEBUG_cout << "mpalloc:: numBlocksNeed: " << numBlocksNeeded << endl;

  const int NOT_FOUND = -1;
  ssize_t blockStartIndex = this->findSpaceForChunk(numBlocksNeeded);
  if (blockStartIndex == NOT_FOUND) {
    DEBUG_cout << "Alloc Fail" << endl; 
    throw MemoryPool::Exception(ExceptionType::ALLOC_FAIL_NO_CHUNK);
  }
  
  //void* allocatedPtr = _allocate ( blockStartIndex, numOfBlocksNeeded );
  
  try {
    this->markBlockMap(blockStartIndex, numBlocksNeeded);
  } catch (std::exception& e) {
    // Marking Block map failed. Try to undo marking.
    DEBUG_cerr << "Marking Bitmap has failed. Reverting marking." << endl; 
    this->markBlockMap(blockStartIndex, numBlocksNeeded, TOZERO);
    throw MemoryPool::Exception(ExceptionType::ALLOC_FAIL);
  }

  // Deduct Free Space
  this->poolHeader_->freeSize -= numBlocksNeeded * this->poolHeader_->blockSize;
  
  // Save to-be-returned PTR;
  uintptr_t allocatedMemPtr = ((uintptr_t)(this->poolHeader_->poolBodyAddress)) +
              blockStartIndex * this->poolHeader_->blockSize;

  // Move Ptr
  //allocatedMemPtr += (blockStartIndex - 1) * this->blockSize;

  // Insert Chunk Header
  ChunkHeader chunkHeader;
  chunkHeader.firstBlockIndex = blockStartIndex; 
  chunkHeader.numBlocksUsed = numBlocksNeeded;
  chunkHeader.contentSize = allocSize - sizeof(ChunkHeader);
  memcpy((void*)allocatedMemPtr, &chunkHeader, sizeof(ChunkHeader));
  
  // Move Ptr by Chunk Header Size
  allocatedMemPtr += sizeof(ChunkHeader);

  return (void*) allocatedMemPtr;
}

ssize_t MemoryPool::findSpaceForChunk (const size_t numBlocksNeeded) {
  DEBUG_FUNC_START;

  uintptr_t lastopAddr = (uintptr_t) this->poolHeader_->lastOperationBlockAddress;
  uintptr_t bitmapAddr = (uintptr_t) this->poolHeader_->blockBitmapAddress;
  size_t bitmapSize = this->poolHeader_->blockBitmapSize;
  uintptr_t bitmapEndAddr = bitmapAddr + bitmapSize;
  size_t lastopAddrToEndSize = bitmapEndAddr - lastopAddr;
  
  ssize_t foundChunkStartIndex = this->findSpaceForChunk(numBlocksNeeded,
                                                 (void*) lastopAddr,
                                                 lastopAddrToEndSize);

  if (foundChunkStartIndex == -1) {
    //DEBUG_cerr << "NOT FOUND" << endl;
    // Scan the whole pool again. This must be very rare.
    foundChunkStartIndex = this->findSpaceForChunk(numBlocksNeeded,
                                                 (void*) bitmapAddr,
                                                 bitmapSize);
  } 

  return foundChunkStartIndex;
}

ssize_t MemoryPool::findSpaceForChunk(const size_t numBlocksNeeded,
    void* searchStartBitmapBlockAddress, size_t numBitmapBlocksToScan)
{
  DEBUG_FUNC_START;
  // Start scanning from the beginning? Not most efficient. Develop better algo.
  //   Implementation of LastOpIndex improved about 8~10% performance increase.

  ssize_t possibleChunkStartIndex = -1;
  size_t foundEmptyChunkBlocks = 0;
  size_t searchStartBitmapBlockIndex = (uintptr_t)searchStartBitmapBlockAddress -
                                       (uintptr_t)this->poolHeader_->blockBitmapAddress;

  int currentIndex = searchStartBitmapBlockIndex * 8;

  uint8_t* mapPtr = (uint8_t *)(searchStartBitmapBlockAddress);

  for (size_t i = 0; numBitmapBlocksToScan > i; ++i) {
    for (size_t nBit = 1; nBit <= 8; ++nBit) {
      if ( (*mapPtr) & (1 << (8 - nBit)) ) { // Block Taken...
        possibleChunkStartIndex = -1; // Reset possibleChunkStartIndex
      } else { // Block Available
        if (possibleChunkStartIndex == -1) { //If PossibleStartIndex is not yet found.
          possibleChunkStartIndex = currentIndex;
          foundEmptyChunkBlocks = 0;
        }
        ++foundEmptyChunkBlocks;
        
        if (foundEmptyChunkBlocks == numBlocksNeeded) { // BINGO. Found Chunk
          DEBUG_cout << "Found Start Index: " << possibleChunkStartIndex << endl;
          // Return 
          this->poolHeader_->lastOperationBlockAddress = (void*) mapPtr;
          return possibleChunkStartIndex;
        }
      }
      ++currentIndex;
    }
    ++mapPtr;
  }
  
  DEBUG_cout << "could not find chunk." << endl; 
  return -1;
}

size_t MemoryPool::MpAllocFit(const void* flexAllocedChunk, size_t contentSize) {
  // Get ChunkHeader
  // Get Total Size without ChunkHeader.
  // Compare with contentSize
  // find out how many blocks can be freeed.
  // Find the index and numBlocks to be Freeed
  // Mark map.
  // Updated FresSize
  
  DEBUG_FUNC_START;
  if (this->poolHeader_->poolHeaderAddress == nullptr) {
    throw MemoryPool::Exception(ExceptionType::NOT_INITIALIZED);
  }
  DEBUG_cout << "contentSize: " << contentSize << endl; 

  void* validRangeStart = this->poolHeader_->poolBodyAddress;
  void* validRangeEnd = this->poolHeader_->poolEndAddress;

  if(flexAllocedChunk < validRangeStart || validRangeEnd < flexAllocedChunk ) { 
    if (this->next_ != nullptr) {
      return this->next_->MpAllocFit(flexAllocedChunk, contentSize);
    } 
    DEBUG_cerr << "Invalid chunk pointer." << endl; 
    throw MemoryPool::Exception(ExceptionType::INVALID_POINTER_OUT_OF_RANGE);  
  }
  
  
  // Get Chunk Header which contatins Allocated Memory Chunk Information
  ChunkHeader* chunk = (ChunkHeader*)((uintptr_t) flexAllocedChunk - sizeof(ChunkHeader));

  // #TODO: Check if chunk is valid.
  // #TODO: When random freePtr is given, it gets random values for chunker and breaks the whole memory pool.
  DEBUG_cout << "chunk.magicChar:" << chunk->magicChar << endl;
  DEBUG_cout << "chunk.firstBlockIndex:" << chunk->firstBlockIndex << endl;
  DEBUG_cout << "chunk.blocksUsed:" << chunk->numBlocksUsed << endl;

  if (chunk->magicChar != MemoryPool::MAGIC_CHAR) {
    DEBUG_cerr << "Invalid chunk pointer. Magicchar mismatch." << endl; 
    throw MemoryPool::Exception(ExceptionType::INVALID_POINTER_NO_CHUNK);
  }


  size_t totalChunkSize = chunk->numBlocksUsed * this->poolHeader_->blockSize;
  if (contentSize > totalChunkSize - sizeof(ChunkHeader)) {
    DEBUG_cerr << "contentSize is bigger than allocated size." << endl; 
    return 0;
  } 
  DEBUG_cout << "totalChunkSize: " << totalChunkSize << endl; 
  size_t actualUsedSize = sizeof(ChunkHeader) + contentSize;
  size_t freeSize = totalChunkSize - actualUsedSize;
  DEBUG_cout << "freeSize: " << freeSize << endl; 

  size_t numBlocksToFree = freeSize / this->poolHeader_->blockSize;

  DEBUG_cout << "freeBlocks: " << numBlocksToFree << endl; 

  if (numBlocksToFree > 0) {
    // unmap
    this->markBlockMap(chunk->firstBlockIndex + (chunk->numBlocksUsed - numBlocksToFree), numBlocksToFree, TOZERO);

    chunk->numBlocksUsed -= numBlocksToFree;

    size_t freedSize = numBlocksToFree * this->poolHeader_->blockSize;
    this->poolHeader_->freeSize += freedSize;
    // #MAYBE: Check FreeSize. If freesize is bigger than mempoolsize, it's an obvious error.

    DEBUG_cout << "FreedSize: " << freedSize << endl; 
    return freedSize;
  } else {
    return 0;
  }
}


size_t MemoryPool::Mpfree(const void* freePtr) {
  DEBUG_FUNC_START;

  if (this->poolHeader_->poolHeaderAddress == nullptr) {
    throw MemoryPool::Exception(ExceptionType::NOT_INITIALIZED);
  }

  void* validRangeStart = this->poolHeader_->poolBodyAddress;
  void* validRangeEnd = this->poolHeader_->poolEndAddress;

  if(freePtr < validRangeStart || validRangeEnd < freePtr ) { 
    if (this->next_ != nullptr) {
      return this->next_->Mpfree(freePtr);
    } 
    throw MemoryPool::Exception(ExceptionType::INVALID_POINTER_OUT_OF_RANGE);  
  }
  
  
  // Get Chunk Header which contatins Allocated Memory Chunk Information
  ChunkHeader* chunk = (ChunkHeader*)((uintptr_t) freePtr - sizeof(ChunkHeader));

  // #TODO: Check if chunk is valid.
  // #TODO: When random freePtr is given, it gets random values for chunker and breaks the whole memory pool.
  DEBUG_cout << "chunk.magicChar:" << chunk->magicChar << endl;
  DEBUG_cout << "chunk.firstBlockIndex:" << chunk->firstBlockIndex << endl;
  DEBUG_cout << "chunk.blocksUsed:" << chunk->numBlocksUsed << endl;

  if (chunk->magicChar != MemoryPool::MAGIC_CHAR) {
    throw MemoryPool::Exception(ExceptionType::INVALID_POINTER_NO_CHUNK);
  }

  // unmap
  this->markBlockMap(chunk->firstBlockIndex, chunk->numBlocksUsed, TOZERO);

  // Mark it with some other char to prevent accidental access.
  chunk->magicChar = 'F'; 

  // Update last op address
  size_t bitmapOffset = chunk->firstBlockIndex / 8;
  uintptr_t lastop = (uintptr_t)this->poolHeader_->blockBitmapAddress + bitmapOffset;
  // Maybe update this only when Big Enough chunk has been freed.
  this->poolHeader_->lastOperationBlockAddress = (void*) lastop;  
  
  // Add Freesize to memorypool
  size_t freedSize = chunk->numBlocksUsed * this->poolHeader_->blockSize;
  this->poolHeader_->freeSize += freedSize;
  // #MAYBE: Check FreeSize. If freesize is bigger than mempoolsize, it's an obvious error.

  return freedSize;
}



bool MemoryPool::markBlockMap (const int startBlockIndex, const int numBlocks, const bool toZero) {
  DEBUG_FUNC_START;
  int tmpCount = numBlocks;

  int ptrShifter = 0;
  if (startBlockIndex > 0){
    ptrShifter = startBlockIndex / 8;
  }
  int bitOffset = startBlockIndex % 8;
  int solidBlockCount = (numBlocks - (8 - bitOffset)) / 8; // solid block = one byte == all one or zero
  
  char* mapPtr = static_cast<char *>(this->poolHeader_->blockBitmapAddress);
  mapPtr += ptrShifter;

  // #MAYBE: I may be able to improve readability of this function by using std::bitset.
  
  //First Block
  for (int i = 0; i < (8 - bitOffset) && i < 8; ++i) {
    if (toZero == false ) {
      // MARK to ONE
      (*mapPtr) |= (1 << (7 - i - bitOffset));
    } else {
      // MARK to Zero
      (*mapPtr) ^= (1 << (7 - i - bitOffset));
    }
    --tmpCount;
    if (tmpCount <= 0) break;
  }

  for (int i = 0; i < solidBlockCount; ++i) {
    ++mapPtr;
    if (toZero == false) {
      // Mark all 8 bits in one byte to One  
      (*mapPtr) |= ~0;
    } else {
      // Mark all 8 bits in one byte to Zero
      (*mapPtr) &= 0;
    }
    tmpCount -= 8;
  }
  ++mapPtr;

  DEBUG {
    if (tmpCount >= 8) {
      DEBUG_cerr << "tmpCount cannot be greater than 7" << endl; 
    } 
  }
  // Last Block
  while (tmpCount > 0) {
    if (toZero == false) {
      // Mark to One
      (*mapPtr) |= (1 << (8 - tmpCount));
    } else {
      // Mark to Zero
      (*mapPtr) ^= (1 << (8 - tmpCount));
    }
    --tmpCount;
  }

  return true;
}


void MemoryPool::_PrintPoolInfo () const {
  cout << std::hex << "poolHeaderAddress" << "\t\t" << this->poolHeader_->poolHeaderAddress << endl;
  cout << std::hex << "blockBitmapAddress" << "\t\t" << this->poolHeader_->blockBitmapAddress << endl;
  cout << std::hex << "poolBodyAddress" << "\t\t" << this->poolHeader_->poolBodyAddress << endl;
  cout << std::dec << "blockBitmapSize" << "\t\t" << this->poolHeader_->blockBitmapSize << endl;  
  cout << std::dec << "blockSize" << "\t\t" << this->poolHeader_->blockSize << endl;
  cout << std::dec << "poolSize" << "\t\t" << this->poolHeader_->poolSize << endl;
  cout << std::dec << "freeSize" << "\t\t" << this->poolHeader_->freeSize << endl;
  cout << std::hex << "lastOpAddress" << "\t\t" << this->poolHeader_->lastOperationBlockAddress << endl;

  cout << std::dec << endl;
}

void MemoryPool::_PrintBlockMap() const {
  if (this->poolHeader_->poolHeaderAddress == nullptr) {
    // ERROR
    throw MemoryPool::Exception(ExceptionType::NOT_INITIALIZED);
    return;
  }
  
  char* map = static_cast<char*>(this->poolHeader_->blockBitmapAddress);
    
  for (unsigned int i = 1; i <= this->poolHeader_->blockBitmapSize; i++) {
    std::bitset<8> bitSetToPrint( *map );
    cout << bitSetToPrint <<" ";
    if (i%8 == 0) cout << endl;
    map++;
  }
  
  cout << endl;
}

bool MemoryPool::createPoolHeader(size_t poolSize, size_t blockSize, Mode mode, void* poolAddress) {

  if (blockSize > poolSize) {
    DEBUG_cerr << "Invalid Configruation for MemoryPool." << endl; 
    throw MemoryPool::Exception(ExceptionType::INIT_FAIL);
  }

  PoolHeader poolHeader;

  poolHeader.blockSize = blockSize;
  poolHeader.blockBitmapSize = poolSize / blockSize / 8;

  poolHeader.poolSize = blockSize * poolHeader.blockBitmapSize * 8;
  poolHeader.freeSize = poolHeader.poolSize;

  poolHeader.poolSize += sizeof(PoolHeader);
  poolHeader.poolSize += poolHeader.blockBitmapSize;

  if (mode == Mode::LOCAL) {
    poolAddress = malloc(poolHeader.poolSize);
  }

  if (poolAddress == nullptr) {
    DEBUG_cerr << "Could not allocate memory for MemoryPool." << endl; 
    throw MemoryPool::Exception(ExceptionType::INIT_FAIL);
    return false;
  }
  
  std::memset(poolAddress, 0, poolHeader.poolSize);

  poolHeader.poolHeaderAddress = poolAddress;
  poolHeader.blockBitmapAddress = (void *) ((uintptr_t) poolAddress + sizeof(PoolHeader));
  poolHeader.poolBodyAddress = (void *) ((uintptr_t) poolHeader.blockBitmapAddress + poolHeader.blockBitmapSize);
  poolHeader.poolEndAddress = (void *) ((uintptr_t) poolHeader.poolHeaderAddress + poolHeader.poolSize);
  
  poolHeader.lastOperationBlockAddress = poolHeader.blockBitmapAddress;

  std::memcpy(poolAddress, &poolHeader, sizeof(PoolHeader));

  this->poolHeader_ = static_cast<PoolHeader*>(poolAddress);
  return true;
}

MemoryPool::ChunkHeader* MemoryPool::getChunkHeader (const void* chunkLocation) const {

  // Get Chunk Header which contatins Allocated Memory Chunk Information
  uintptr_t chunkHeadLocation = (uintptr_t) chunkLocation - sizeof(ChunkHeader);

  return (ChunkHeader*) chunkHeadLocation;
}

MemoryPool* MemoryPool::createNextMp() {
  MemoryPool* mp = new MemoryPool(this->poolHeader_->poolSize, this->poolHeader_->blockSize);
  return mp;
}


}

#if _UNIT_TEST

/*
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockMemoryPool : public MemoryPool {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(MemoryPool, TESTNAME) {
  MockMemoryPool mockMemoryPool(1024 * 64);
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

*/

#include <iostream>
#include <string>

#include <exception>

using namespace lio;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::pair;

int main() {
 
  MemoryPool* newMp = nullptr;
  try {
    newMp = new MemoryPool (1024 * 128 * 1, 42);
  
    newMp->_PrintPoolInfo();
    for (int i = 0; 4000 > i; ++i) {
      void* mpA = newMp->Mpalloc(5000);
      void* mpB = newMp->Mpalloc(2000);
      newMp->Mpfree(mpA);
      void* mpC = newMp->Mpalloc(50);
      void* mpD = newMp->Mpalloc(6000);
      newMp->Mpfree(mpB);
      void* mpE = newMp->Mpalloc(3000);
      newMp->Mpfree(mpD);
      newMp->Mpfree(mpE);
    } 
    
  } catch (MemoryPool::Exception &ex) {
    cout << "ex: " << ex.what() << endl;
  } catch (std::exception &ex) {
    cout << "EX:" << ex.what() << endl;
  }
  newMp->_PrintBlockMap();
    newMp->_PrintPoolInfo();
    delete newMp;

  return 0;
}
#else
// Executable File's Main Comes here.

/*
*/
#endif
#undef _UNIT_TEST
