#include "MapStorage.hpp"

#define _UNIT_TEST false
#include "liolib/Test.hpp"


namespace lio {

}

#if _UNIT_TEST

#include <iostream>
#include <string>
#include <map>

using namespace lio;

using std::cout;
using std::endl;
using std::map;
using std::string;

int main (int argc, char** argv) {
  map<string, string> ssmap;
  ssmap["Hello"] = "World!";
  ssmap["What the"] = "Hell!!";
  lio::saveMap<string, string>(ssmap, "./testmap.data");


  map<string, string> newSsmap;
  lio::loadMap<string, string>(&newSsmap, "./testmap.data");

  for (auto& elem : newSsmap) {
    cout << elem.first << " - " << elem.second << endl;
  } 

  map<int, string> ismap;
  ismap[0] = "World World!";
  ismap[100] = "Nice hundred world";
  lio::saveMap<int, string>(ismap, "./testimap.data");


  map<int, string> newIsmap;
  lio::loadMap<int, string>(&newIsmap, "./testimap.data");

  for (auto& elem : newIsmap) {
    cout << elem.first << " - " << elem.second << endl;
  } 
  return 0;
}

#else
// Executable File's Main Comes here.


#endif

#undef _UNIT_TEST

