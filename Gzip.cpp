#include "Gzip.hpp"

#define _UNIT_TEST false
#if _UNIT_TEST
  #include "liolib/Test.hpp"
#endif

namespace lio {

// ===== Exception Implementation =====
const char* const
Gzip::Exception::exceptionMessages_[] = {
  GZIP_EXCEPTION_MESSAGES
};
#undef GZIP_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


Gzip::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
Gzip::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const Gzip::ExceptionType
Gzip::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End =====

const size_t Gzip::CHUNK_SIZE = 1024 * 1024;
const int Gzip::WINDOWS_BITS = 15;
const int Gzip::GZIP_ENCODING = 16;

Gzip::Gzip(Config config) :
  config_(config),
  mp_(nullptr)
{
  if (config.useMemoryPool == true) {
    this->mp_ = new MemoryPool(config.memoryPoolSize);
  } 
}

Gzip::~Gzip() {
  if (this->mp_ != nullptr) {
    delete mp_;
  } 
}

DataBlock<> Gzip::Compress(const void* source, size_t length, int level) {
  DEBUG_FUNC_START;
  if (this->config_.useMemoryPool == false) {
    DEBUG_cerr <<
      "This function requires use of MemoryPool but Gzip is not set to use MP." << endl; 
    return DataBlock<>();
  } 
  DEBUG_cout << "Original size: " << length << endl;
  int ret, flush;
  unsigned have;
  z_stream strm;

  unsigned char* in;// = new unsigned char[length];
  //unsigned char* out = new unsigned char[CHUNK_SIZE];

  unsigned char* out = nullptr;
  try {
    out = (unsigned char*) this->mp_->Mpalloc(CHUNK_SIZE);

  } catch (MemoryPool::Exception& ex) {
    DEBUG_cerr << "Memory Allocation has failed. " << ex.what() << endl; 
    throw Exception(ExceptionType::COMPRESSION_FAIL);
  } 

  in = (unsigned char*) source;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = in;


  ret = deflateInit2(&strm, level, Z_DEFLATED, WINDOWS_BITS | GZIP_ENCODING, 8, Z_TEXT);
  //ret = deflateInit(&strm, level);
  if (ret != Z_OK) {
    DEBUG_cerr << "deflateInit failed." << endl;
    throw Exception(ExceptionType::COMPRESSION_FAIL);
  }

  strm.avail_in = length;

  flush = Z_FINISH;


  do {
    strm.avail_out = CHUNK_SIZE;
    strm.next_out = out;
    ret = deflate(&strm, flush);
    assert (ret != Z_STREAM_ERROR);
    have = CHUNK_SIZE - strm.avail_out;
  } while (strm.avail_out == 0);


  if (flush != Z_FINISH) {
    DEBUG_cerr << "Not finished" << endl;
  }

  have = strm.total_out;
  (void) deflateEnd(&strm);
  DEBUG_cout << "Compressed size: " << have << endl;
  
  this->mp_->MpAllocFit((void*)out, have);

  return DataBlock<>((void*)out, 0, have);
}

DataBlock<> Gzip::Decompress(const void* source, size_t length) {
  DEBUG_FUNC_START;
  if (this->config_.useMemoryPool == false) {
    DEBUG_cerr <<
      "This function requires use of MemoryPool but Gzip is not set to use MP." << endl; 
    return DataBlock<>();
  } 
  int ret;
  unsigned have;
  z_stream strm;

  unsigned char* in;// = new unsigned char[length];
  //unsigned char* out = new unsigned char[CHUNK_SIZE];
  unsigned char* out = (unsigned char*) this->mp_->Mpalloc(CHUNK_SIZE);

  in = (unsigned char*) source;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  ret = inflateInit2(&strm, WINDOWS_BITS | GZIP_ENCODING);
  if (ret != Z_OK) {
    throw Exception(ExceptionType::DECOMPRESSION_FAIL);
  }

  strm.avail_in = length;
  strm.next_in = in;

  strm.avail_out = CHUNK_SIZE;
  strm.next_out = out;
  ret = inflate(&strm, Z_NO_FLUSH);
  assert (ret != Z_STREAM_ERROR);

  switch (ret) {
   case Z_NEED_DICT:
    ret = Z_DATA_ERROR;
   case Z_DATA_ERROR:
   case Z_MEM_ERROR:
    (void)inflateEnd(&strm);
    DEBUG_cerr << "decomp fail." << endl;
    throw Exception(ExceptionType::DECOMPRESSION_FAIL);
  }

  have = strm.total_out;


  (void) inflateEnd(&strm);
  DEBUG_cout << "Decompressed size: " << have << endl;

  this->mp_->MpAllocFit((void*)out, have);


  return DataBlock<>((void*)out, 0, have);
}

ssize_t Gzip::Compress(const void* source, size_t length, char* dest, size_t maxSize, int level) {
  DEBUG_FUNC_START;
  DEBUG_cout << "Original size: " << length << endl;
  int ret, flush;
  unsigned have;
  z_stream strm;

  unsigned char* in;// = new unsigned char[length];
  //unsigned char* out = new unsigned char[CHUNK_SIZE];

  unsigned char* out = (unsigned char*) dest;

  in = (unsigned char*) source;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = in;

  ret = deflateInit2(&strm, level, Z_DEFLATED, WINDOWS_BITS | GZIP_ENCODING, 8, Z_TEXT);
  //ret = deflateInit(&strm, level);
  if (ret != Z_OK) {
    DEBUG_cerr << "deflateInit failed." << endl;
    throw Exception(ExceptionType::COMPRESSION_FAIL);
  }

  strm.avail_in = length;

  flush = Z_FINISH;
  do {
    strm.avail_out = maxSize;
    strm.next_out = out;
    ret = deflate(&strm, flush);
    assert (ret != Z_STREAM_ERROR);
    have = maxSize - strm.avail_out;
  } while (strm.avail_out == 0);


  have = strm.total_out;
  (void) deflateEnd(&strm);

  if (flush != Z_FINISH) {
    DEBUG_cerr << "Not finished" << endl;
    return -1;
  }

  DEBUG_cout << "Compressed size: " << have << endl;
  
  return have;
}

ssize_t Gzip::Decompress(const void* source, size_t length, char* dest, size_t maxSize) {
  DEBUG_FUNC_START;
  int ret;
  unsigned have;
  z_stream strm;

  unsigned char* in;// = new unsigned char[length];
  //unsigned char* out = new unsigned char[CHUNK_SIZE];
  unsigned char* out = (unsigned char*) dest;

  in = (unsigned char*) source;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  ret = inflateInit2(&strm, WINDOWS_BITS | GZIP_ENCODING);
  if (ret != Z_OK) {
    throw Exception(ExceptionType::DECOMPRESSION_FAIL);
  }

  strm.avail_in = length;
  strm.next_in = in;

  strm.avail_out = maxSize;
  strm.next_out = out;
  ret = inflate(&strm, Z_NO_FLUSH);
  assert (ret != Z_STREAM_ERROR);

  switch (ret) {
   case Z_NEED_DICT:
    ret = Z_DATA_ERROR;
    (void)inflateEnd(&strm);
    DEBUG_cerr << "Z_NEED_DICT" << endl; 
    return -1;
   case Z_DATA_ERROR:
    (void)inflateEnd(&strm);
    DEBUG_cerr << "Z_DATA_ERROR" << endl; 
    return -1;
   case Z_MEM_ERROR:
    (void)inflateEnd(&strm);
    DEBUG_cerr << "Z_MEM_ERROR" << endl; 
    DEBUG_cerr << "Decomp failed." << endl;
    return -1;
  }

  have = strm.total_out;


  (void) inflateEnd(&strm);
  DEBUG_cout << "Decompressed size: " << have << endl;

  return have;
}
//Gzip::
//Gzip::

}

#if _UNIT_TEST


#include <iostream>

using namespace lio;

int main() {
  Gzip gzip;
  string t = "Text to compreesdgsgdsgsss is here. I'm gonna make this string much longer to see how the compression rates differ. Now it only ssaves abodut 3~5% but I expect it tos be way ways way shigher compression rate. :) hehe let's sdgsee how much it will compress the data. :)";
  t += "Text to empress is here. I'm gonna make this string much longer to see how the comvpression rates difefer. xNowe it odasgnly seaves aboutd 3~5% buvt I expesgsgcadfvsadft it to be way way wday hvsigher compression rate. :) hehes let'ds seex how msuch it will compress the data. :)\0";
  DataBlock<> compressed = gzip.Compress((const void*) t.c_str(), t.length());
  DataBlock<> decompressed = gzip.Decompress((const void*) compressed.object, compressed.length);
  
  char* org = (char*)decompressed.object;
  std::cout << org << endl;
  return 0;
}
#endif
#undef _UNIT_TEST

