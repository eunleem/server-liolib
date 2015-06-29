#include "SecureSocket.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

// ===== Exception Implementation ===== 
const char* const
SecureSocket::Exception::exceptionMessages_[] = {
  SECURESOCKET_EXCEPTION_MESSAGES
};
#undef SECURESOCKET_EXCEPTION_MESSAGES // undef helps reducing unnecessary preprocessing work.


SecureSocket::Exception::Exception(ExceptionType exceptionType) :
  exceptionType_(exceptionType) { }

const char*
SecureSocket::Exception::what() const noexcept {
  return this->exceptionMessages_[(int) this->exceptionType_];
}

const SecureSocket::ExceptionType
SecureSocket::Exception::type() const noexcept {
  return this->exceptionType_;
}
// ===== Exception Implementation End ===== 


SecureSocket::SecureSocket(const Socket::SocketFamily family,
    const Socket::SocketType type) :
  socket_(nullptr)
{
  DEBUG_FUNC_START; // Prints out function name in yellow
  this->socket_ = new Socket(family, type);

}

SecureSocket::~SecureSocket() {
  DEBUG_FUNC_START;
  delete this->socket_;
}

void SecureSocket::Run() {
  int     err;
  int     verify_client = OFF; /* To verify a client certificate, set ON */

  int     listen_sock;
  int     sock;
  struct sockaddr_in sa_serv;
  struct sockaddr_in sa_cli;
  socklen_t client_len;
  char    *str;
  char     buf[4096];

  SSL_CTX         *ctx;
  SSL            *ssl;
  SSL_METHOD      *meth;

  X509            *client_cert = NULL;

  short int       s_port = 443;
  /*----------------------------------------------------------------*/
  /* Load encryption & hashing algorithms for the SSL program */
  SSL_library_init();

  /* Load the error strings for SSL & CRYPTO APIs */
  SSL_load_error_strings();

  /* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
  meth = const_cast<SSL_METHOD*>(SSLv3_method());

  /* Create a SSL_CTX structure */
  ctx = SSL_CTX_new(meth);

  if (!ctx) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  /* Load the server certificate into the SSL_CTX structure */
  if (SSL_CTX_use_certificate_file(ctx, RSA_SERVER_CERT, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  /* Load the private-key corresponding to the server certificate */
  if (SSL_CTX_use_PrivateKey_file(ctx, RSA_SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  /* Check if the server certificate and private-key matches */
  if (!SSL_CTX_check_private_key(ctx)) {
    fprintf(stderr,"Private key does not match the certificate public key\n");
    exit(1);
  }

  if(verify_client == ON) {
    /* Load the RSA CA certificate into the SSL_CTX structure */
    if (!SSL_CTX_load_verify_locations(ctx, RSA_SERVER_CA_CERT, NULL)) {
      ERR_print_errors_fp(stderr);
      exit(1);
    }

    /* Set to require peer (client) certificate verification */
    SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);

    /* Set the verification depth to 1 */
    SSL_CTX_set_verify_depth(ctx,1);
  }
  /* ----------------------------------------------- */
  /* Set up a TCP socket */

  listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);   

  RETURN_ERR(listen_sock, "socket");
  memset (&sa_serv, '\0', sizeof(sa_serv));
  sa_serv.sin_family      = AF_INET;
  sa_serv.sin_addr.s_addr = INADDR_ANY;
  sa_serv.sin_port        = htons (s_port);          /* Server Port number */
  err = bind(listen_sock, (struct sockaddr*)&sa_serv,sizeof(sa_serv));

  RETURN_ERR(err, "bind");

  /* Wait for an incoming TCP connection. */
  err = listen(listen_sock, 5);                    

  RETURN_ERR(err, "listen");
  client_len = sizeof(sa_cli);

  /* Socket for a TCP/IP connection is created */
 
  sock = accept(listen_sock, (struct sockaddr*)&sa_cli, &client_len);

  RETURN_ERR(sock, "accept");
  close (listen_sock);

  printf ("Connection from %lx, port %x\n", sa_cli.sin_addr.s_addr, sa_cli.sin_port);

  /* ----------------------------------------------- */
  /* TCP connection is ready. */
  /* A SSL structure is created */
  ssl = SSL_new(ctx);

  RETURN_NULL(ssl);

  /* Assign the socket into the SSL structure (SSL and socket without BIO) */
  SSL_set_fd(ssl, sock);

  /* Perform SSL Handshake on the SSL server */
  err = SSL_accept(ssl);

  RETURN_SSL(err);

  /* Informational output (optional) */
  printf("SSL connection using %s\n", SSL_get_cipher (ssl));

  if (verify_client == ON) {

    /* Get the client's certificate (optional) */
    client_cert = SSL_get_peer_certificate(ssl);
    if (client_cert != NULL) {
      printf ("Client certificate:\n");     
      str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
      RETURN_NULL(str);
      printf ("\t subject: %s\n", str);
      free (str);
      str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
      RETURN_NULL(str);
      printf ("\t issuer: %s\n", str);
      free (str);
      X509_free(client_cert);
    } else {
      printf("The SSL client does not have certificate.\n");
    }
  }

  /*------- DATA EXCHANGE - Receive message and send reply. -------*/
  /* Receive data from the SSL client */
  err = SSL_read(ssl, buf, sizeof(buf) - 1);
  RETURN_SSL(err);
  buf[err] = '\0';
  printf ("Received %d chars:'%s'\n", err, buf);

  /* Send data to the SSL client */
  err = SSL_write(ssl, "This message is from the SSL server", 
  strlen("This message is from the SSL server"));

  RETURN_SSL(err);

  /*--------------- SSL closure ---------------*/
  /* Shutdown this side (server) of the connection. */

  err = SSL_shutdown(ssl);
  RETURN_SSL(err);

  /* Terminate communication on a socket */
  err = close(sock);
  RETURN_ERR(err, "close");

  /* Free the SSL structure */
  SSL_free(ssl);
  /* Free the SSL_CTX structure */
  SSL_CTX_free(ctx);
}

//SecureSocket::

}

#if _UNIT_TEST

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lio;

class MockSecureSocket : public SecureSocket {
public:
  MOCK_METHOD0(Test, void());

};

using ::testing::AtLeast;

TEST(SecureSocket, TESTNAME) {
  MockSecureSocket mockSecureSocket;
}

int main (int argc, char** argv) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

