#include "NewMemoryPool.hpp"

namespace lio {

// ===== Exception Implementation =====
const char* const
NewMemoryPool::Exception::exceptionMessages_[] = {
  NEWMEMORYPOOL_EXCEPTION_MESSAGES
};
#undef NEWMEMORYPOOL_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


NewMemoryPool::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
NewMemoryPool::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const NewMemoryPool::ExceptionType
NewMemoryPool::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====


NewMemoryPool::NewMemoryPool(const Config& config, const void* poolLocation) {
  DEBUG_FUNC_START; // Prints out function name in yellow

  PoolHeader header;

  header.config = config;
  header.freeSize = config.poolSize - sizeof(header);

  if (config.mode == Mode::LOCAL) {
    header.poolBeginning = (uintptr_t)(malloc(header.config.poolSize));
  } else {
    if (poolLocation == nullptr) {
      throw Exception(ExceptionType::INVALID_PARAM);
    }
    header.poolBeginning = (uintptr_t) poolLocation;
  }
  header.poolEnd = header.poolBeginning + header.config.poolSize;

  TEST {
    UnitTest::Test<uintptr_t>((uintptr_t)header.poolEnd, header.poolBeginning + config.poolSize, "MP size");
    DEBUG_cout << "PoolBegLoc: " << header.poolBeginning << endl;
    DEBUG_cout << "PoolEndLoc: " << header.poolEnd << endl; 
    DEBUG_cout << "FirstChunkLoc: " << header.poolBeginning + sizeof(header) << endl; 
    DEBUG_cout << "FreeSize: " << header.freeSize << endl; 
  }
  

  ChunkHeader freeChunkHeader;
  freeChunkHeader.size = header.freeSize;
  
  memcpy((void*)(header.poolBeginning + sizeof(header)), &freeChunkHeader, sizeof(freeChunkHeader));

  header.freeChunksHead = (ChunkHeader *) (header.poolBeginning + sizeof(header));

  memcpy((void*)header.poolBeginning, &header, sizeof(header));

  this->poolHeader_ = (PoolHeader*) header.poolBeginning;
    
  TEST {
    UnitTest::Test<uintptr_t>((uintptr_t)this->poolHeader_, header.poolBeginning, "MP");
    UnitTest::Test<uintptr_t>((uintptr_t)this->poolHeader_->freeSize, this->poolHeader_->freeChunksHead->size, "MP FreeSize");
  }
}

NewMemoryPool::~NewMemoryPool() {
  DEBUG_FUNC_START;

}

void* NewMemoryPool::Mpalloc (size_t size) {

  size_t realAllocSize = size + sizeof(ChunkHeader);

  ChunkHeader* currentChunk = this->poolHeader_->freeChunksHead;
  if (currentChunk == nullptr) {
    DEBUG_cerr << "freeChunksHead cannot be null." << endl; 
    return nullptr;
  } 

  while (currentChunk != nullptr) {
    DEBUG_POINT;
    if (currentChunk->size < realAllocSize) {
      currentChunk = currentChunk->next;
      DEBUG_POINT;
      continue;

    } else if (currentChunk->size == realAllocSize) {
      DEBUG_POINT;
      // if Size is same, just convert currentChunk from Free Chunk to Used Chunk
      currentChunk->type = ChunkType::USED;
      // Remove currentChunk from Free Chunks List.
      if (currentChunk->prev != nullptr)
        currentChunk->prev->next = currentChunk->next;
      if (currentChunk->next != nullptr)
        currentChunk->next->prev = currentChunk->prev;

      currentChunk->next = nullptr;

      ChunkHeader* lastUsedChunk = this->poolHeader_->usedChunksHead;
      if (lastUsedChunk == nullptr) {
        // If First Used Chunk,
        currentChunk->prev = nullptr;
        this->poolHeader_->usedChunksHead = currentChunk;
      } else {
        while (lastUsedChunk != nullptr) {
          if (lastUsedChunk->next != nullptr) {
            lastUsedChunk = lastUsedChunk->next;
            continue;
          }
          break;
        }
        currentChunk->prev = lastUsedChunk;
        lastUsedChunk->next = currentChunk;
      }

      break;


    } else if (currentChunk->size > realAllocSize) {
      DEBUG_POINT;
      // If Larger free chunk is found,
      // 1. Copy old Free Chunk
      ChunkHeader oldFreeChunk = (*currentChunk);
      // 2. Overwrite old free chunk with USED one.
      currentChunk->type = ChunkType::USED;
      currentChunk->size = realAllocSize;
      currentChunk->next = nullptr;
      // Remove currentChunk from Free Chunks List.
      if (currentChunk->prev != nullptr)
        currentChunk->prev->next = currentChunk->next;
      if (currentChunk->next != nullptr)
        currentChunk->next->prev = currentChunk->prev;


      // 3. Create New Free Chunk. Use info from old free chunk
      size_t remainingFreeSize = oldFreeChunk.size - realAllocSize;
      
      ChunkHeader newFreeChunkHeader;
      newFreeChunkHeader.size = remainingFreeSize;
      newFreeChunkHeader.prev = oldFreeChunk.prev;
      newFreeChunkHeader.next = oldFreeChunk.next;
      DEBUG_POINT;

      uintptr_t newChunkLocation = (uintptr_t)currentChunk + realAllocSize;

      memcpy((void*)newChunkLocation, &newFreeChunkHeader, sizeof(ChunkHeader));

      if (oldFreeChunk.prev != nullptr)
        oldFreeChunk.prev->next = (ChunkHeader*)newChunkLocation;
      else
        this->poolHeader_->freeChunksHead = (ChunkHeader*)newChunkLocation;
     
      // Add it to used chunks list

      ChunkHeader* lastUsedChunk = this->poolHeader_->usedChunksHead;
      if (lastUsedChunk == nullptr) {
        this->poolHeader_->usedChunksHead = currentChunk;
        currentChunk->prev = nullptr;

      } else {
        while (lastUsedChunk != nullptr) {
          if (lastUsedChunk->next != nullptr) {
            lastUsedChunk = lastUsedChunk->next;
            continue;
          }
          break;
        }
        currentChunk->prev = lastUsedChunk;
        lastUsedChunk->next = currentChunk;
      }


      break;

    } else {
      DEBUG_cout << "ERROR" << endl; 
      return nullptr;
    }
  } 
  return (void*) ((uintptr_t)currentChunk + sizeof(ChunkHeader));
}

bool NewMemoryPool::Mpfree(void* location) {
  DEBUG_FUNC_START;

  ChunkHeader* chunk = (ChunkHeader*) location;
  if (chunk->magic_number != 37173) {
    DEBUG_cerr << "Invalid Chunk Location." << endl; 
    return false;
  } 

  if (chunk->type != ChunkType::USED) {
    DEBUG_cout << "Chunk is not in use. Cannot free." << endl; 
    return false;
  }

  // 1. get size and check if next chunk is free or in use.
  // ****************************************** START WORK AGAIN FROM HERE!!!!!
}

void NewMemoryPool::printFreeChunks() const {
  DEBUG_cout << "Printing Free Chunks" << endl; 
  int i = 0;
  ChunkHeader* chunk = this->poolHeader_->freeChunksHead;
  while (chunk != nullptr) {
    DEBUG_cout << i << " chunkLocation: " << chunk << " size: " << chunk->size << endl;
    chunk = chunk->next;
    i += 1;
  } 
  DEBUG_cout << "Done Printing." << endl; 
}

void NewMemoryPool::printUsedChunks() const {
  DEBUG_cout << "Printing Used Chunks" << endl; 
  int i = 0;
  ChunkHeader* chunk = this->poolHeader_->usedChunksHead;
  while (chunk != nullptr) {
    DEBUG_cout << i << " chunkLocation: " << chunk << " size: " << chunk->size << endl;
    
    chunk = chunk->next;
    i += 1;
  } 
  DEBUG_cout << "Done Printing." << endl; 
}
//NewMemoryPool::
//NewMemoryPool::

//NewMemoryPool::
//NewMemoryPool::
//NewMemoryPool::
//NewMemoryPool::

}

#if _UNIT_TEST

#include <iostream>

using namespace lio;

int main() {
  NewMemoryPool::Config config;
  NewMemoryPool* mp = new NewMemoryPool(config);

  mp->printFreeChunks();
  mp->printUsedChunks();
  mp->Mpalloc(1000);
  mp->printFreeChunks();
  mp->printUsedChunks();
  mp->Mpalloc(2000);
  mp->printFreeChunks();
  mp->printUsedChunks();
  delete mp;

  TEST UnitTest::ReportTestResult();
  return 0;
}
#endif
#undef _UNIT_TEST

