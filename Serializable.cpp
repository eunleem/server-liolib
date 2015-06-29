#include "Serializable.hpp"

#include <iostream>

using namespace lio;

class TestSerializable : public Serializable {
public:
  TestSerializable() { }
  ~TestSerializable() { }

  std::ostream& Serialize(std::ostream& stream) const {
    std::cout << "SERIALIZING F" << std::endl;
    stream.write((char*)&this->number, sizeof(this->number));
    uint32_t size = text.size();
    stream.write((char*)&size, sizeof(size));
    stream.write(text.c_str(), size);
    return stream;
  }

  std::istream& Deserialize(std::istream& stream) {
    std::cout << "DESERIALIZING F" << std::endl;
    stream.read((char*)&this->number, sizeof(this->number));
    uint32_t size = 0;
    stream.read((char*)&size, sizeof(size));
    text.resize(size);
    stream.read((char*)text.c_str(), size);
    return stream;
  }

  uint32_t number;
  std::string text;
private:
};


int main() {
  {
    TestSerializable serial;
    std::fstream file;

    serial.number = 10;
    serial.text = "GOOGLE SUCKS! MY COMPANY RULES!";


    file.open("serial.txt", std::ios::out | std::ios::binary);
    if (file.is_open() == false) {
      std::cout << "ERROR" << std::endl;
      return -1;
    } 
    file << serial;
    file.close();
  }

  {
    TestSerializable read;
    std::fstream file;
    file.open("serial.txt", std::ios::in | std::ios::binary);
    if (file.is_open() == false) {
      std::cout << "ERROR2" << std::endl;
      return -1;
    } 

    file >> read;
    file.close();

    std::cout << "Read Number: " << read.number << std::endl;
    std::cout << "Read Text: " << read.text << std::endl;
  }

  return 0;
}
