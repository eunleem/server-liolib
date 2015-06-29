#ifndef _SEMAPHORE_HPP_
#define _SEMAPHORE_HPP_

/*
  Name
    Semaphore

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description


  Last Modified Date
    Mar 2, 2013

  Learning Resources
    Semaphores Tutorial
      http://beej.us/guide/bgipc/output/html/multipage/semaphores.html
    semget
      http://linux.die.net/man/2/semget
  
  Copyright (c) All Rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false

#include "Debug.hpp" //DEBUG_cout
#include <errno.h> // errno
#include <sys/types.h>

#include <sys/ipc.h> // ftok()
#include <linux/sem.h> // semun SEMMSL
#include <sys/sem.h> // semget() semctl() semop()

#include <string>
#include "Util.hpp"


namespace lio {

using std::string;


/*union semun {
  int val;         // used for SETVAL only 
  struct semid_ds *buf;   used for IPC_STAT and IPC_SET 
  ushort *array;      used for GETALL and SETALL
};
*/



class Semaphore {
public:

enum class ExceptionType {
  GENERAL,
  INVALID_KEY,
  SEM_INIT_FAILED,
  SEM_EXISTS,
  SEM_ACCESS_DENIED,
  SEM_GET_FAILED,
  SEM_UNSET_FAILED
};

#define SEMAPHORE_EXCEPTION_MESSAGES \
  "Semaphore Exception has been thrown.", \
  "Invalid Keyfile path.", \
  "Failed to initialize Semaphore.", \
  "Semaphore already exists.", \
  "Access to the semaphore is denied.", \
  "Failed to get semaphore.", \
  "Unset Semaphore has failed."

class Exception : public std::exception {
public:
  Exception (ExceptionType exceptionType = ExceptionType::GENERAL);

  virtual const char* what() const throw();
  virtual const ExceptionType type() const throw();
private:
  ExceptionType               exceptionType_;
  static const char* const    exceptionMessages_[];
};

enum class Mode {
  AUTO, // Try to Create, if already exists, LOAD.
  CREATE, // initialize new one
  LOAD // load already existing one
};

struct Config {
  Config(string semKeyDir = "./",
         string semKeyName = "semKey",
         Mode semMode = Mode::AUTO,
         int semPermission = 0666,
         uint8_t semCount = 5
         ) 
    : semKeyDir(semKeyDir),
      semKeyName(semKeyName), 
      semMode (semMode),
      semPermission(semPermission),
      semCount (semCount) {}
  string semKeyDir;
  string semKeyName;
  Mode semMode;
  int semPermission;
  uint8_t semCount;
};

  /**
   * semCount: total number of semaphores to create. Multiple semaphores are created using one key. Determine how many you need. Max Number is Defined in SEMMSL
   *
   */
  Semaphore (Config& config);

  virtual
  ~Semaphore();
  
  bool          Lock(int semNum = 0);
  bool          Release(int semNum = 0);
  
  // When set to destroy, Semaphore will be delete when desctructor is called. (Semaphore is shared between processes and if not set to destroy, Semaphore will continue.)
  void          SetToDestroySemOnDelete();
   
  // #DEPRECATED: use SetToDestroySemOnDelete
  int           DestroySem();
  
  // Test Functions
  void          _PrintKey();
  void          _PrintSemId();
protected:
private:
  Config        config_;

  key_t         semKey_;

  int           semId_;
  
  bool          isSetToDestroySem_;

   
  // Initiate Semaphores
  int           initSem();

  void          createSem();
  void          loadSem();
  
};

}

#endif
