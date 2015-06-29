#include "HttpConnection.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
HttpConnection::Exception::exceptionMessages_[] = {
  HTTPCONNECTION_EXCEPTION_MESSAGES
};
#undef HTTPCONNECTION_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


HttpConnection::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
HttpConnection::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const HttpConnection::ExceptionType
HttpConnection::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


size_t HttpConnection::MAX_BUFFER_SIZE = 1024 * 16;

HttpConnection::HttpConnection(MemoryPool* mp) :
  status(Status::NEW),
  isKeepAlive(false),
  currentWork(new HttpWork()),
  fd(0),
  mpBuffer_(mp)
{
  const std::chrono::duration<int, std::ratio<1>> timeout(5);
  this->openedTime = system_clock::now();
  this->expirationTime = this->openedTime + timeout;
}

HttpConnection::~HttpConnection() {
  if (this->currentWork != nullptr) {
    delete currentWork;
  } 
}


int HttpConnection::GetFd() const {
  return this->fd;
}

void HttpConnection::SetFd(const int fd) {
  this->fd = fd;
  this->currentWork->fd = fd;
  this->status = Status::READY;
}

HttpConnection::Status HttpConnection::ReadRequest() {
  DataBlock<char*> buffer;
  if (this->status == Status::READY) {
    buffer.SetObject((char*) this->mpBuffer_->Mpalloc(this->MAX_BUFFER_SIZE));
  } 

  return Status::DONE_READING;
  
}

HttpWork* HttpConnection::PopWork() {
  if (this->currentWork == nullptr) {
    DEBUG_cerr << "Tried to Handout work that is null." << endl; 
  } 

  if (this->status != Status::DONE_READING) {
    DEBUG_cerr << "Popping work that is not done." << endl; 
    throw Exception(ExceptionType::GENERAL);
  } 

  HttpWork* work = this->currentWork;
  this->currentWork = new HttpWork();

  this->status = Status::READY;
  work->request->SetBuffer(work->buffer);

  return work;
}

void HttpConnection::Close() {
  DEBUG_cout << "Connection Closed." << endl; 
  close(this->fd);
  this->status = Status::CLOSED;
}


//HttpConnection::
//HttpConnection::
//HttpConnection::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockHttpConnection : public HttpConnection {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(HttpConnection, TESTNAME) {
  MockHttpConnection mockHttpConnection;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

