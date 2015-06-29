#include "MemoryPool.hpp"

namespace lio {


// ===== Exception Implementation =====
const char* const
MemoryPoolException::exceptionMessages_[] = {
  MEMORY_POOL_EXCEPTION_MESSAGES
};
#undef MEMORY_POOL_EXCEPTION_MESSAGES

MemoryPoolException::MemoryPoolException(MemoryPoolExceptionType exceptionType) {
  this->exceptionType_ = exceptionType;
}

const char*
MemoryPoolException::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const MemoryPoolExceptionType
MemoryPoolException::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====



const bool TOZERO = true;
const char MemoryPool::ChunkHeader::MAGIC_CHAR = 'C';


MemoryPool::MemoryPool (MemoryPool::Config& config)
  : mode_(Mode::LOCAL),
    poolLocation_(nullptr) {
  
  //#TODO: validate Block Size and Pool Size
  if (config.blockSize > config.poolSize) {
    // #TODO: Throw InvalidParameterException
    throw MemoryPoolException();
  }

  this->config_.blockSize = config.blockSize;

  PoolHeader poolHeader;

  poolHeader.blockSize = config.blockSize;
  poolHeader.blockBitMapSize = config.poolSize / config.blockSize / 8;

  poolHeader.poolSize = config.blockSize * poolHeader.blockBitMapSize * 8;
  poolHeader.freeSize = poolHeader.poolSize;

  poolHeader.poolSize += sizeof(PoolHeader);
  poolHeader.poolSize += poolHeader.blockBitMapSize;

  void* poolStartLocation = malloc(poolHeader.poolSize);
  if (poolStartLocation == nullptr) {
    throw MemoryPoolException(MemoryPoolExceptionType::INIT_FAIL);
  }
  this->poolLocation_ = poolStartLocation;
  
  std::memset(poolStartLocation, 0, poolHeader.poolSize);


  poolHeader.poolHeaderPtr = poolStartLocation;
  poolHeader.blockBitMapPtr = (void *) ((uintptr_t) poolStartLocation + sizeof(PoolHeader));
  poolHeader.poolBodyPtr = (void *) ((uintptr_t) poolHeader.blockBitMapPtr + poolHeader.blockBitMapSize);
  poolHeader.poolEndPtr = (void *) ((uintptr_t) poolHeader.poolHeaderPtr + poolHeader.poolSize);

  std::memcpy(poolStartLocation, &poolHeader, sizeof(PoolHeader));

  this->poolHeader_ = static_cast<PoolHeader*>(poolStartLocation);
}

MemoryPool::MemoryPool (const void* poolLocation, const MemoryPool::Config& config)
  : mode_(Mode::SHARED_MEMORY),
    poolLocation_(poolLocation) {

  DEBUG_FUNC_START;
  DEBUG_cout << "PoolHeader: " << std::hex << poolLocation << std::dec << endl; 

  if (this->poolLocation_ == nullptr) {
    DEBUG_cerr << "poolLocation cannot be nullptr." << endl;
    throw MemoryPoolException(MemoryPoolExceptionType::INIT_FAIL);
  }

  //#TODO: validate Block Size and Pool Size
  if (config.blockSize > config.poolSize) {
    // #TODO: Throw InvalidParameterException
    throw MemoryPoolException();
  }

  this->config_.blockSize = config.blockSize;

  PoolHeader poolHeader;

  poolHeader.blockSize = config.blockSize;
  poolHeader.blockBitMapSize = config.poolSize / config.blockSize / 8;

  poolHeader.poolSize = config.blockSize * poolHeader.blockBitMapSize * 8;
  poolHeader.freeSize = poolHeader.poolSize;

  poolHeader.poolSize += sizeof(PoolHeader);
  poolHeader.poolSize += poolHeader.blockBitMapSize;

  void* poolStartLocation = const_cast<void*>(poolLocation);
  
  std::memset(poolStartLocation, 0, poolHeader.poolSize);

  poolHeader.poolHeaderPtr = poolStartLocation;
  poolHeader.blockBitMapPtr = (void *) ((uintptr_t) poolStartLocation + sizeof(PoolHeader));
  poolHeader.poolBodyPtr = (void *) ((uintptr_t) poolHeader.blockBitMapPtr + poolHeader.blockBitMapSize);
  poolHeader.poolEndPtr = (void *) ((uintptr_t) poolHeader.poolHeaderPtr + poolHeader.poolSize);

  std::memcpy(poolStartLocation, &poolHeader, sizeof(PoolHeader));

  this->poolHeader_ = static_cast<PoolHeader*>(poolStartLocation);
}

MemoryPool::~MemoryPool () {
  DEBUG_FUNC_START;
  if (this->mode_ == Mode::LOCAL) {
    free (this->poolHeader_->poolHeaderPtr);
  } else {
    // SHARED MEMORY Module will Detach Shared Memory.
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
  // Get Chunk Header which contatins Allocated Memory Chunk Information
  const ChunkHeader* chunkHead = this->getChunkHeader(chunkLocation);
  return (size_t) chunkHead->numBlocksUsed;
}


//#TODO: OPTIMIZE THIS FUNCTION
void* const MemoryPool::Mpalloc(size_t allocSize) {
  DEBUG_cout << "Mpalloc() Started. " << "allocSize: " << allocSize << endl;
  if (this->poolHeader_->poolHeaderPtr == nullptr) {
    // ERROR
    throw MemoryPoolException(MemoryPoolExceptionType::NOT_INITIALIZED);
  }
  // 0. Add header size to to toAllocSize
  allocSize += sizeof (ChunkHeader);
  // 1. Check if the Pool has enough Space.
  if (this->poolHeader_->freeSize < allocSize) {
    throw MemoryPoolException(MemoryPoolExceptionType::ALLOC_FAIL_NOT_ENOUGH_SPACE);
  }
  
  size_t numBlocksNeeded = (allocSize / this->poolHeader_->blockSize);
  if ((allocSize % this->poolHeader_->blockSize) > 0) {
    numBlocksNeeded++;
  }

  DEBUG_cout << "mpalloc:: numBlocksNeed: " << numBlocksNeeded << endl;

  const int NOT_FOUND = -1;
  int blockStartIndex = this->findSpaceForChunk(numBlocksNeeded);
  if (blockStartIndex == NOT_FOUND) {
    DEBUG_cout << "Alloc Fail" << endl; 
    throw MemoryPoolException(MemoryPoolExceptionType::ALLOC_FAIL_NO_CHUNK);
  }
  
  //void* allocatedPtr = _allocate ( blockStartIndex, numOfBlocksNeeded );
  
  try {
    this->markBlockMap(blockStartIndex, numBlocksNeeded);
  } catch (std::exception& e) {
    // Marking Block map failed. Try to undo marking.
    // 
    this->markBlockMap(blockStartIndex, numBlocksNeeded, TOZERO);
  }

  // Deduct Free Space
  this->poolHeader_->freeSize -= numBlocksNeeded * this->poolHeader_->blockSize;
  
  // Save to-be-returned PTR;
  uintptr_t allocatedMemPtr = ((uintptr_t)(this->poolHeader_->poolBodyPtr)) +
              (blockStartIndex - 1) * this->poolHeader_->blockSize;

  // Move Ptr
  //allocatedMemPtr += (blockStartIndex - 1) * this->blockSize;

  // Insert Chunk Header
  ChunkHeader chunkHeader;
  chunkHeader.firstBlockLocation = blockStartIndex; 
  chunkHeader.numBlocksUsed = numBlocksNeeded;
  chunkHeader.contentSize = allocSize - sizeof(ChunkHeader);
  memcpy((void*)allocatedMemPtr, &chunkHeader, sizeof(ChunkHeader));
  
  // Move Ptr by Chunk Header Size
  allocatedMemPtr += sizeof(ChunkHeader);

  return (void*) allocatedMemPtr;
}



int MemoryPool::findSpaceForChunk (const size_t numBlocksNeeded) {
  DEBUG_cout << "findSpaceForChunk Started." << endl;
  //  Start scanning from the beginning? Not most efficient. Develop better algo.
  int currentIndex = 1;
  unsigned int possibleStartIndex = 0;
  unsigned int count = 0;
  
  DEBUG_cout << "blockBitMapPtr Address: " << std::hex << this->poolHeader_->blockBitMapPtr << std::dec << endl; 
  
  char* mapPtr = (char *)(this->poolHeader_->blockBitMapPtr);
  DEBUG_cout << "mapPtr Address: " << std::hex << (void*) mapPtr << std::dec << endl; 

  
  char allBitsAreOne = ~0; // ALL Bits ON
  DEBUG_POINT;
  for (size_t nthByte_i = 1; nthByte_i <= this->poolHeader_->blockBitMapSize; nthByte_i++) {
    DEBUG_POINT;
    //#TODO: Does this "if" statement improve performance or not? Test that!
    DEBUG_cout << "*mapPtr: " << (int)(*mapPtr) << endl; 
    DEBUG_cout << "allBitsAreOne: " << (int)allBitsAreOne << endl; 
    if ( (*mapPtr) == allBitsAreOne ) { // if all 8 blocks are taken in the byte
    DEBUG_POINT;
      possibleStartIndex = 0;
    DEBUG_POINT;
      mapPtr++;
    DEBUG_POINT;
      currentIndex += sizeof((*mapPtr)) * 8;
    DEBUG_POINT;
    } else {
    DEBUG_POINT;
      for (int nthBit_j = 1; nthBit_j <= 8; nthBit_j++) {
        char cmpBit = 1 << (8 - nthBit_j);
    DEBUG_POINT;
        
        if ( ((*mapPtr) & cmpBit) == cmpBit ) { // Block Taken...
          possibleStartIndex = 0; // Reset possibleStartIndex
        } else { // Block Available
          if (possibleStartIndex == 0) { //If PossibleStartIndex is not yet found.
            possibleStartIndex = currentIndex;
            count = 0;
          }
          count++;
          
          if (count == numBlocksNeeded) { // BINGO. Found Chunk
            DEBUG_cout << "Found Start Index: " << possibleStartIndex << endl;
            // Return 
            size_t finalStartBlockIndex = possibleStartIndex;
            return finalStartBlockIndex;
          }
        }
    DEBUG_POINT;
        currentIndex++;
      }
      mapPtr++;
    }
        DEBUG_cout << "LOOPING B" << endl; 
  }
  DEBUG_cout << "could not find chunk." << endl; 
  return -1;
}


size_t MemoryPool::Mpfree(const void* freePtr) {
  DEBUG_cout << "Mpfree(const uintprt_t) Started." << endl;
  if (this->poolHeader_->poolHeaderPtr == 0) {
    throw MemoryPoolException(MemoryPoolExceptionType::NOT_INITIALIZED);
  }

  void* validRangeStart = this->poolHeader_->poolBodyPtr;
  void* validRangeEnd = this->poolHeader_->poolEndPtr;

  if(freePtr < validRangeStart || validRangeEnd < freePtr ) { 
    throw MemoryPoolException(MemoryPoolExceptionType::INVALID_POINTER_OUT_OF_RANGE);  
  }
  
  
  // Get Chunk Header which contatins Allocated Memory Chunk Information
  ChunkHeader* chunkHead = (ChunkHeader*)((uintptr_t) freePtr - sizeof(ChunkHeader));

  // #TODO: Check if chunkHead is valid.
  // #TODO: When random freePtr is given, it gets random values for chunkHeader and breaks the whole memory pool.
  DEBUG_cout << "chunkHead.magicChar:" << chunkHead->magicChar << endl;
  DEBUG_cout << "chunkHead.firstBlockLocation:" << chunkHead->firstBlockLocation << endl;
  DEBUG_cout << "chunkHead.blocksUsed:" << chunkHead->numBlocksUsed << endl;

  if (chunkHead->magicChar != MemoryPool::ChunkHeader::MAGIC_CHAR) {
    throw MemoryPoolException(MemoryPoolExceptionType::INVALID_POINTER_NO_CHUNK);
  }

  // unmap
  this->markBlockMap(chunkHead->firstBlockLocation, chunkHead->numBlocksUsed, TOZERO);

  size_t freedSize = chunkHead->numBlocksUsed * this->poolHeader_->blockSize;
  this->poolHeader_->freeSize += freedSize;
  // #MAYBE: Check FreeSize. If freesize is bigger than mempoolsize, it's an obvious error.

  DEBUG_cout << "Mpfree(const uintprt_t) Ended." << endl;
  return freedSize;
}



bool MemoryPool::markBlockMap (const int startBlockIndex, const int numBlocks, const bool toZero) { // First index for start is 1 not 0.
  DEBUG_cout << "markBlockMap Started." << endl;
  int tmpCount = numBlocks;
  int start = startBlockIndex - 1;

  int ptrShifter = 0;
  if (start > 0){
    ptrShifter = start / 8;
  }
  int bitOffset = start % 8;
  int solidBlockCount = (numBlocks - (8 - bitOffset)) / 8; // solid block = one byte == all one or zero
  
  char* mapPtr = static_cast<char *>(this->poolHeader_->blockBitMapPtr);
  mapPtr += ptrShifter;

  // #MAYBE: I may be able to improve readability of this function by using std::bitset.
  
  //First Block
  for (int i = 0; i < (8 - bitOffset) && i < 8; i++) {
    if (toZero == false ) {
      // MARK to ONE
      (*mapPtr) |= (1 << (7 - i - bitOffset));
    } else {
      // MARK to Zero
      (*mapPtr) ^= (1 << (7 - i - bitOffset));
    }
    tmpCount--;
    if (tmpCount <= 0) break;
  }

  for (int i = 0; i < solidBlockCount; i++) {
    mapPtr++;
    if (toZero == false) {
      // Mark all 8 bits in one byte to One  
      (*mapPtr) |= ~0;
    } else {
      // Mark all 8 bits in one byte to Zero
      (*mapPtr) &= 0;
    }
    tmpCount -= 8;
  }
  mapPtr++;

  // Last Block
  while (tmpCount > 0) {
    if (toZero == false) {
      // Mark to One
      (*mapPtr) |= (1 << (8 - tmpCount));
    } else {
      // Mark to Zero
      (*mapPtr) ^= (1 << (8 - tmpCount));
    }
    tmpCount--;
  }

  DEBUG_cout << "markBlockMap Ended." << endl;
  return true;
}



void MemoryPool::_PrintPoolInfo () const {
  cout << std::hex << "poolHeaderPtr" << "\t\t" << this->poolHeader_->poolHeaderPtr << endl;
  cout << std::hex << "blockBitMapPtr" << "\t\t" << this->poolHeader_->blockBitMapPtr << endl;
  cout << std::hex << "poolBodyPtr" << "\t\t" << this->poolHeader_->poolBodyPtr << endl;
  cout << std::dec << "blockBitMapSize" << "\t\t" << this->poolHeader_->blockBitMapSize << endl;  
  cout << std::dec << "blockSize" << "\t\t" << this->poolHeader_->blockSize << endl;
  cout << std::dec << "poolSize" << "\t\t" << this->poolHeader_->poolSize << endl;
  cout << std::dec << "freeSize" << "\t\t" << this->poolHeader_->freeSize << endl;

  cout << ""  << endl;
}

void MemoryPool::_PrintBlockMap() const {
  if (this->poolHeader_->poolHeaderPtr == nullptr) {
    // ERROR
    throw MemoryPoolException(MemoryPoolExceptionType::NOT_INITIALIZED);
    return;
  }
  
  char* map = static_cast<char*>(this->poolHeader_->blockBitMapPtr);
    
  for (unsigned int i = 1; i <= this->poolHeader_->blockBitMapSize; i++) {
    std::bitset<8> bitSetToPrint( *map );
    cout << bitSetToPrint <<" ";
    if (i%8 == 0) cout << endl;
    map++;
  }
  
  cout << endl;
}

MemoryPool::ChunkHeader* MemoryPool::getChunkHeader (const void* chunkLocation) const {
  uintptr_t validRangeStart = (uintptr_t) this->poolHeader_->poolBodyPtr;
  uintptr_t validRangeEnd = (uintptr_t) this->poolHeader_->poolHeaderPtr + (uintptr_t) this->poolHeader_->poolSize;

  if ((uintptr_t) chunkLocation < validRangeStart ||
      validRangeEnd < (uintptr_t) chunkLocation ) { // [ETL] Pay attention to the order of variables written.  
    throw MemoryPoolException(MemoryPoolExceptionType::INVALID_POINTER_OUT_OF_RANGE);  
  }

  // Get Chunk Header which contatins Allocated Memory Chunk Information
  uintptr_t chunkHeadLocation = (uintptr_t) chunkLocation - sizeof(ChunkHeader);

  return (ChunkHeader*) chunkHeadLocation;
}


}

#define _UNIT_TEST false
#if _UNIT_TEST

#include "Test.hpp"

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
  
  MemoryPool* newMp;
  try {
    MemoryPool::Config config = MemoryPool::Config(1024*50);
    newMp = new MemoryPool (config);
  
    newMp->_PrintPoolInfo();
    newMp->_PrintBlockMap();
    
    void* mpA = newMp->Mpalloc(3503);
    void* mpB = newMp->Mpalloc(4640);
    newMp->_PrintPoolInfo();
    newMp->_PrintBlockMap();
    newMp->Mpfree(mpA);
    newMp->_PrintBlockMap();
    void* mpC = newMp->Mpalloc(1056);
    newMp->_PrintBlockMap();
    void* mpD = newMp->Mpalloc(7017);
    newMp->_PrintBlockMap();
    newMp->Mpfree(mpC); // THIS SHOULD FAIL

    //newMp->Mpfree(mpB - 3); // THIS SHOULD FAIL
    newMp->_PrintBlockMap();
    delete newMp;
    
  } catch (MemoryPoolException &ex) {
    cout << "ex: " << ex.what() << endl;
  } catch (std::exception &ex) {
    cout << "EX:" << ex.what() << endl;
  }

  return 0;
/*  UnitTest::Test(dba, dbb, "DBAB");

  string value = strMap.GetValueByKey("A");

  UnitTest::Test<string>(value, "Hello", "GetValueByKeyA");

  value = strMap.GetValueByKey("B");

  DataBlock returned = strMap.GetDataBlockByKey("C");

  UnitTest::Test<string>(value, "content", "GetValueByKeyB");
  
  UnitTest::Test<DataBlock>(returned, DataBlock(6, 0), "GetDataBlockByKeyC");

  UnitTest::ReportTestResult();


  return 0;*/
}
#endif
#undef _UNIT_TEST
