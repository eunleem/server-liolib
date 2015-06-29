#include "FileLoader.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
FileLoader::Exception::exceptionMessages_[] = {
  FILELOADER_EXCEPTION_MESSAGES
};
#undef FILELOADER_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


FileLoader::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
FileLoader::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const FileLoader::ExceptionType
FileLoader::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


FileLoader::FileLoader() {
  DEBUG_FUNC_START; // Prints out function name in yellow

}

FileLoader::~FileLoader() {
  DEBUG_FUNC_START;

}

DataBlock<> FileLoader::LoadFile(const string& filePath) {

  if (filePath.find("..") != string::npos) {
    // Dangerous file request. Cancel it and record it.
    DEBUG_cerr << "Dangerous Request has been received." << endl; 
    return DataBlock<>();
  } 
  
  DEBUG_cout << "FilePath: " << filePath << endl;

  ifstream fileStream;

  fileStream.open(filePath.c_str());
  if(fileStream.is_open()) {
    DEBUG_cout << "File Opened!" << endl; 
    fileStream.seekg(0, std::ios::end);
    size_t fileSize = fileStream.tellg();
    
    DEBUG_cout << "FileSize found. fileSize: " << fileSize << endl; 
    if (fileSize > 1024 * 1024 * 20) {
      // Loading too big file can cause serious security hazard.
      DEBUG_cerr << "File size is too big. This module is to load small files only." << endl; 
      return DataBlock<>();
    } 

    void* memblock = malloc(fileSize);
    // HANDLE MEMORY POOL ERROR
   
    //memblock = this->mempool_->Mpalloc(fileSize);

    fileStream.seekg(0, std::ios::beg);
    fileStream.read((char *)memblock, fileSize);
    DEBUG_cout << "File Loaded to memory." << endl; 
    fileStream.close();

    //this->fileNoti_.AddToWatch(filePath.c_str(), IN_MOVE | IN_MODIFY | IN_DELETE);

    return DataBlock<>(memblock, 0, fileSize);
  } else {
    DEBUG_cerr << "Could not open file. Errno: " << errno << endl; 
    return DataBlock<>(); // Null DataBlock
  }

  return DataBlock<>(); // Null DataBlock
}

//FileLoader::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockFileLoader : public FileLoader {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(FileLoader, TESTNAME) {
  MockFileLoader mockFileLoader;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

