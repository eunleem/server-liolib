#ifndef _MAPSTORAGE_HPP_
#define _MAPSTORAGE_HPP_
/*
  Name
    MapStorage

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Last Modified Date
    May 28, 2014
  
  History
    May 25, 2014
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

#include <fstream>
#include <string>
#include <map>

#include "liolib/Util.hpp"

//#include "liolib/DataBlock.hpp"

namespace lio {

using std::string;
using std::map;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::ios;

template<class KEY_T, class DATA_T>
int saveMap(const map<KEY_T, DATA_T>& m, const string& path) {
  static_assert(true, "NOT IMPLEMENTED YET.");
  return 0;
}

template<>
int saveMap<string, string>(const map<string, string>& map, const string& path) {
  // Validate Path first.
  
  bool isFileExisting = Util::IsFileExisting(path.c_str());
  if (isFileExisting == false) {
    DEBUG_cout << "File does not exist " << endl; 

  } else {
    DEBUG_cout << "File Exists." << endl; 
  }

  ofstream file;
  file.open(path.c_str(), ios::out);
  if (file.is_open() == false) {
    DEBUG_cerr << "Failed to open a file." << endl; 
    return -1;
  }

  // File Open
  // Save each element to file.
  for (auto& elem : map) {
    // key string\n
    // data string\n
    // and so on...
    DEBUG_cout << "key data " << elem.first << " - " << elem.second << endl; 
    file << elem.first << "\n" << elem.second << "\n";
  } 

  file.close();

  DEBUG_cout << "Map Saved." << endl; 

  
  return 0;
}


template<>
int saveMap<int, string>(const map<int, string>& map, const string& path) {
  // Validate Path first.
  
  bool isFileExisting = Util::IsFileExisting(path.c_str());
  if (isFileExisting == false) {
    DEBUG_cout << "File does not exist " << endl; 

  } else {
    DEBUG_cout << "File Exists." << endl; 
  }

  ofstream file;
  file.open(path.c_str(), ios::binary | ios::out);
  if (file.is_open()) {
    // File Open
    // Save each element to file.
    for (auto& elem : map) {
      // key int
      // data int length   string\n
      // and so on...
      DEBUG_cout << "Start;" << endl; 
      file.write(reinterpret_cast<const char*>(&elem.first), sizeof(int));
      DEBUG_cout << "first written." << endl; 
      size_t length = elem.second.length();
      file.write(reinterpret_cast<const char*>(&length), sizeof(length));
      DEBUG_cout << "second written." << endl; 
      file << elem.second << "\n";
    } 
    file.close();
    DEBUG_cout << "Map Loaded." << endl; 

  } else {
    DEBUG_cerr << "Failed to open a file." << endl; 
  }
  
  return 0;
}





template<class KEY_T, class DATA_T>
int loadMap(map<KEY_T, DATA_T>* m, const string& path) {
  static_assert(true, "NOT IMPLEMENTED YET.");
  return 0;
}

template<>
int loadMap<string, string>(map<string, string>* map, const string& path) {
  // Validate Path first.
  
  bool isFileExisting = Util::IsFileExisting(path.c_str());
  if (isFileExisting == false) {
    DEBUG_cerr << "File does not exist " << endl; 
    return -1;

  }

  int count = 0;

  fstream file;
  file.open(path.c_str(), ios::in);
  if (file.is_open() == false) {
    DEBUG_cerr << "Failed to open file." << endl; 
    return -1;
  } 

  // File Open
  string key, data;
  while (std::getline(file, key)) {
    DEBUG_cout << "string key: " << key << endl; 
    if (std::getline(file, data)) {
      DEBUG_cout << "string data: " << data << endl; 
      (*map)[key] = data;
      count += 1;
    } else {
      DEBUG_cerr << "KEY-DATA not matching." << endl; 
    }
  } 
  DEBUG_cout << "Done reading file. Read " << count << " rows." << endl; 
  file.close();
  
  return count;
}

template<>
int loadMap<int, string>(map<int, string>* map, const string& path) {
  // Validate Path first.
  
  bool isFileExisting = Util::IsFileExisting(path.c_str());
  if (isFileExisting == false) {
    DEBUG_cerr << "File does not exist " << endl; 
    return -1;

  }

  int count = 0;
 
  fstream file;
  file.open(path.c_str(), ios::binary | ios::in);
  if (file.is_open()) {
    DEBUG_cerr << "Failed to open a file." << endl; 
    return -1;
  } 

  // File Open
  while (true) {
    int key = 0;
    size_t dataLength = 0;

    file.read(reinterpret_cast<char*>(&key), sizeof(key));
    //DEBUG_cout << "Read key: " << key << endl; 

    if (file.eof()) {
      DEBUG_cout << "End of file." << endl; 
      break;
    } 

    file.read(reinterpret_cast<char*>(&dataLength), sizeof(dataLength));
    //DEBUG_cout << "Read dataLength: " << dataLength << endl; 
    
    char buf[dataLength];
    memset(buf, 0, dataLength + 1);
    file.read(buf, dataLength);
    //DEBUG_cout << "Read Data: " << buf << endl; 
    char endline;
    file.read(&endline, 1);
    if (endline != '\n') {
      DEBUG_cerr << "Invalid file format." << endl; 
      return -1;
    } 

    (*map)[key] = string(buf);
    count += 1;
  } 
  DEBUG_cout << "Done reading file. Read " << count << " rows." << endl; 
  file.close();
  
  return count;
}

}

#endif

