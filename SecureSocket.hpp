#ifndef _SECURESOCKET_HPP_
#define _SECURESOCKET_HPP_
/*
  Name
    SecureSocket

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    Dec 02, 2014
  
  History
    November 11, 2014
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
#define _DEBUG true

#include "liolib/Debug.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "liolib/Socket.hpp"
 
#define RSA_SERVER_CERT     "server.crt"
#define RSA_SERVER_KEY          "server.key"
 
#define RSA_SERVER_CA_CERT "server_ca.crt"
#define RSA_SERVER_CA_PATH   "sys$common:[syshlp.examples.ssl]"
 
#define ON   1
#define OFF        0
 
#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }


//#include "liolib/DataBlock.hpp"

namespace lio {


class SecureSocket {
public:

// ******** Exception Declaration *********
enum class ExceptionType : std::uint8_t {
  GENERAL
};
#define SECURESOCKET_EXCEPTION_MESSAGES \
  "SecureSocket Exception has been thrown."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char*         what() const noexcept;
  virtual const               ExceptionType type() const noexcept;
  
private:
  const ExceptionType         exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********

static
void init();
static
SSL_CTX* newCtx(SSL_METHOD* method = const_cast<SSL_METHOD*>(SSLv3_method()));
SSL* newSSL(SSL_CTX* ctx, int fd);

// int SSL_read(ssl, buf, sizeof(buf) - 1);
// int SSL_write(ssl, buf, strlen(buf));
//




protected:
  
private:
  
};

}

#endif

