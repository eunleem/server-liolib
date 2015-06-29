#ifndef _STRINGMAP_HPP_
#define _STRINGMAP_HPP_
/*
  Name
    StringMap

  Authors
    [ETL] Eun T. Leem (eunleem@gmail.com)

  Description

  Last Modified Date
    Jan 01, 2013
  
  History

  ToDos
    


  Milestones
    1.0

  Learning Resources
    http://
  
  Copyright (c) All Rights reserved to LIFEINO.
*/

#ifdef _DEBUG
  #undef _DEBUG
#endif
#define _DEBUG false

#include "liolib/Debug.hpp"

#include <string>
#include <map>
#include <stdexcept> // std::out_of_range

#include "liolib/Consts.hpp"
#include "liolib/DataBlock.hpp" // DataBlock



namespace lio {

using std::string;
using std::map;
using std::pair;


using lio::DataBlock;

// ******** Exception Declaration *********
enum class StringMapExceptionType : std::uint8_t {
  GENERAL,
  INIT_FAIL,
  KEY_NOT_FOUND,
  KEY_ALREADY_EXISTS,
  KEYDATA_ERROR,
  NO_VALUE_FOUND,
  APPEND_CONTENT
};
#define STRINGMAP_EXCEPTION_MESSAGES \
  "StringMap Exception has been thrown.", \
  "StringMap cannot be initialized with Empty String.", \
  "Key Not Found.", \
  "Key Already Exists.", \
  "Key Data contains error.", \
  "Value does not exists for the given key.", \
  "Failed to append content."


class StringMapException : public std::exception {
public:
  StringMapException (StringMapExceptionType exceptionType = StringMapExceptionType::GENERAL);

  virtual const char*         what() const noexcept;

  virtual const               StringMapExceptionType type() const noexcept;
  
private:
  StringMapExceptionType      exceptionType_;
  static const char* const    exceptionMessages_[];
};
// ******** Exception Declaration END*********


template<class MAPKEY_T>
class StringMap {

typedef typename map<MAPKEY_T, PartialString>::iterator MapLocation;

public:
  StringMap(string* content) :
    content_(content),
    isSetToDeleteContentOnDelete_(false) 
  {
    DEBUG_FUNC_START;
    if (content == nullptr || content->empty()) {
      DEBUG_cerr << "String map cannot be initialized with nullptr or empty string." << endl; 
      throw StringMapException(StringMapExceptionType::INIT_FAIL);
    }
    //assert(content->empty() == false && "Content Cannot be empty.");
  }
  virtual
  ~StringMap() {
    DEBUG_FUNC_START;
    if (isSetToDeleteContentOnDelete_ == true) {
      DEBUG_cout << "Deleting the content_." << endl; 
      delete content_;
    } 
  }

  void SetToDeleteString() {
    this->isSetToDeleteContentOnDelete_ = true;
  }

  const string* GetContent() const {
    return this->content_;
  }

  bool AddKeyValue(const MAPKEY_T key, const PartialString& keyData) {
    DEBUG_FUNC_START;

    if (this->keys_.find(key)!= this->keys_.end()) {
      // KEY ALREADY EXISTS
      throw StringMapException(StringMapExceptionType::KEY_ALREADY_EXISTS);
    }

    if (keyData.index + keyData.length > this->content_->length()) {
      // keyData is pointing at something out of range.
      DEBUG_cerr << "keyData is pointing at something out of range." << endl; 
      throw StringMapException(StringMapExceptionType::KEYDATA_ERROR);
    }

    this->keys_.insert(pair<MAPKEY_T, PartialString>(key, keyData));

    return true;
  }

  bool AddKeyValue(const MAPKEY_T key, const size_t index, const size_t length) {
    DEBUG_FUNC_START;
    DEBUG_cout << "AddKeyValue(key): " << key << "index: " << index << "length: " << length << endl;

    return this->AddKeyValue(key, PartialString (this->content_, index, length));
  }



  // More efficient at handling large data since it passes DataBlock data only.
  PartialString GetDataBlockByKey(const MAPKEY_T key) const {
    DEBUG_cout << "GetDataBlockByKey(key): " << key << endl;
    try {
      const PartialString block = this->keys_.at(key);
      return block;
    } catch (const std::out_of_range& e) {
      throw StringMapException(StringMapExceptionType::NO_VALUE_FOUND);
    }
  }

  // Simple but inefficient for large data.
  string GetValueByKey(const MAPKEY_T key) const {
    DEBUG_cout << "GetValueByKey(key): " << key << endl;
    try {
      const PartialString data = this->keys_.at(key);
      return data.GetValue();
    } catch (const std::out_of_range& e) {
      throw StringMapException(StringMapExceptionType::NO_VALUE_FOUND);
    }
  }

  PartialString GetSectionByIndex(const unsigned int index) const {
    DEBUG_cout << "GetSectionByIndex(uint) : " << index << endl;
    try {
      const PartialString block = this->sections_.at(index);
      return block;
    } catch (const std::out_of_range& ex) {
      throw StringMapException(StringMapExceptionType::NO_VALUE_FOUND);
    }
  }


  ssize_t AppendContent(const string* contentToAppend) {
    size_t contentLength = this->content_->length();
    size_t appendContentLength = contentToAppend->length();

    this->content_->append(*contentToAppend);

    ssize_t newContentLength = this->content_->length();
    if (newContentLength != contentLength + appendContentLength) {
      // Append Failed
      DEBUG_cerr << "Failed to append to content." << endl;  
      throw StringMapException(StringMapExceptionType::APPEND_CONTENT);
      return consts::ERROR;
    }
    return newContentLength;
  }

  // ETL: Not sure if this function is reaaallllly needed in this class.
  int Tokenize(const string delimiter) {
    if (delimiter.empty()) {
      return 0;
    }
    size_t delimiterPosition = 0;
    size_t prevDelimiterPosition = 0;

    int count = 0;

    while (true) {
      size_t sectionLength = 0;

      delimiterPosition = this->content_->find(delimiter, prevDelimiterPosition);
      if (delimiterPosition == string::npos) {
        // CASE A: No Delimiter
        // CASE B: After Last Delimiter
        if (prevDelimiterPosition == 0) {
        //if (count == 0) {
          // CASE A: No delimiter
          // Do nothing
        } else {
          // CASE B: After last delimiter
          prevDelimiterPosition += delimiter.length();
          sectionLength = this->content_->length() -  prevDelimiterPosition;
          if (sectionLength > 0) {
            // Only add section if the length is greater than 0.
            PartialString block(this->content_, prevDelimiterPosition, sectionLength);
            this->AddSection(count++, block);
          }
        }
        break;
      } else {
        // CASE A: Delimiter at very First
        // CASE B: Delimiter Normal
        // CASE C: Delimiter right after another delimiter
        // CASE D: Delimiter at the very end.
        
        if (delimiterPosition == 0) {
          // CASE A
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        }

        sectionLength = delimiterPosition - prevDelimiterPosition;
        
        if (delimiterPosition + delimiter.length() == this->content_->length()) {
          // CASE D
           PartialString block(this->content_, prevDelimiterPosition, sectionLength);
          this->AddSection(count++, block);
          break;
        } else if (delimiterPosition + delimiter.length() > this->content_->length()) {
          DEBUG_cerr << "Umm..... nonono this shouldn't happen" << endl;
          break; 
        }

        if (sectionLength > 0) {
          // CASE B
           PartialString block(this->content_, prevDelimiterPosition, sectionLength);
          this->AddSection(count++, block);
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        } else if (sectionLength == 0) {
          // CASE C
          prevDelimiterPosition = delimiterPosition + delimiter.length();
          continue;
        } else {
          DEBUG_cerr << "Umm this shouldn't happen." << endl;
          break;
        }

      }
    }

    return count;
  }
  
  bool AddSection(const unsigned int index, const PartialString keyData) {
    if (this->sections_.find(index) != this->sections_.end()) {
      DEBUG_cout << "Section" << index << " already exists." << endl; 
      throw StringMapException(StringMapExceptionType::KEY_ALREADY_EXISTS);
      return false;
    }
    
    // NOT FOUND. OK to insert new value
    if (keyData.index > this->content_->length()) {
      DEBUG_cerr << "Data is pointing out of range." << endl; 
      // keyData is pointing at something out of range.
      throw StringMapException(StringMapExceptionType::KEYDATA_ERROR);
      return false;
    }
    this->sections_.insert(pair<unsigned int, PartialString>(index, keyData));

    return true;
  }

protected:

  
private:
  string*                       content_;
  map<MAPKEY_T, PartialString>      keys_;
  map<unsigned int, PartialString>  sections_;

  bool isSetToDeleteContentOnDelete_;
  
};

}

#endif

