#ifndef _RADIXTREE_HPP_
#define _RADIXTREE_HPP_

#ifdef _DEBUG
  #undef _DEBUG
  #define _DEBUG true
#endif

#include "Debug.hpp" // DEBUG_cout
#include <iostream>
#include <string>

#include <list> // list<T>

#include <exception>
// Exception Cost Resource
//  Throwing Exception can be costly. Exceptions must be used when it's exceptional enough. (Only happens rarely.
// #REF: http://stackoverflow.com/a/3745092

namespace lio {
using std::cout;
using std::endl;
using std::string;

using std::list;



template <typename T>
class RadixTreeNode {
public:
  RadixTreeNode() : name_(""), parent_(nullptr) {
    //value_ = value;
  }
  RadixTreeNode(const string& name, RadixTreeNode<T>* parent) :
    name_(name), parent_(parent) { }
  RadixTreeNode(const string& name, const T value, RadixTreeNode<T>* parent) :
    name_(name), value_(value), parent_(parent) { }

  ~RadixTreeNode() {
    DEBUG_cout << "~RadixTreeNode() is called.\n";
    DEBUG_cout << "Node " << name_ << " has been deleted.\n";
  }

/*** GET ***/
  const string& GetName() const {
    return name_;
  }

  T GetValue() const {
    return value_;
  }

  RadixTreeNode<T>* GetParent() const {
    return parent_;
  }
  
  list<RadixTreeNode<T>*>* GetChildren() const {
    return &children_;
  }

/*** SET ***/
  void SetNode(const string& name, const T value, RadixTreeNode<T>* parent) {
    name_ = name;
    value_ = value;
    parent_ = parent;
  }

  void SetName(const string& name) {
    name_ = name;
  }

  void SetValue(const T value) {
    value_ = value;
  }

  void SetParent(RadixTreeNode<T>* parent) {
    parent_ = parent;
  }

/*** OTHER ***/
  
  void AddChild(RadixTreeNode<T>* childNode) {
/*    DEBUG_cout << "Adding Child: "
           << childNode->GetName()->c_str() << " " << childNode->GetValue() << "\n";*/

    children_.push_back(childNode);
  }

  RadixTreeNode<T>* PopChild (const string& name) {
    // typename is needed before list<type>::iterator.
    //  #REF: http://stackoverflow.com/questions/6571381/dependent-scope-and-nested-templates
    for (typename list<RadixTreeNode<T>*>::iterator it = children_.begin();
        it != children_.end();
        ++it)
    {
      RadixTreeNode<T>* currentNode = (RadixTreeNode<T>*) (*it);
      const string *nodeName = currentNode->GetName();
      if (nodeName->compare(name) == 0) { // if nodeName == name. #REF: http://www.cplusplus.com/reference/string/string/compare/
        RadixTreeNode<T>* foundNode = (RadixTreeNode<T>*) (*it);
        children_.erase(it);
        DEBUG_cout << name << " has been removed from Children List and returned.\n";
        return foundNode;
      }
    }
    DEBUG_cerr << name << " is not found.\n";
    throw std::exception();
  }

  bool RemoveChild(const string& name) {
    // #THINK: what if child had children?
    for (typename list<RadixTreeNode<T>*>::iterator it = children_.begin();
         it != children_.end();
         ++it)
    {
      RadixTreeNode<T>* currentNode = (RadixTreeNode<T>*) (*it);
      const string *nodeName = currentNode->GetName();
      if (nodeName->compare(name) == 0) { // if nodeName == name. #REF: http://www.cplusplus.com/reference/string/string/compare/
        children_.erase(it);
        DEBUG_cout << name << " has been removed from Children List.\n";
        return true;
      }
    }
    DEBUG_cerr << name << " is not found.\n";
    return false;
  }
protected:

private:
  string name_;
  T value_;
  RadixTreeNode<T>* parent_;
  list<RadixTreeNode<T>*> children_;
};



template <typename T>
class RadixTree {
/*
public:
  RadixTree();
  ~RadixTree();
  
  RadixTreeNode<T>* GetNodeByName(const string &name);
  T GetValueByName(const string &name) {
  void Insert(const string &name, const T value) {
  void _PrintChildren(const string &name) {
  int _CompareName (const string &name, const string &childName) {

private:
  RadixTreeNode<T>* root_;
  unsigned int nodeCount_;

  int _compareName(const string &name, const string &childName) {

 */
public:
  RadixTree() {
    root_ = new RadixTreeNode<T>();
    nodeCount_ = 0;
  }
  ~RadixTree() {
    DEBUG_cout << "~RadixTree() has been called.\n";
  }

  RadixTreeNode<T>* GetNodeByName(const string& name) {
    unsigned int nameLength = name.length();
    unsigned int nameCurrentIndex = 0;

    RadixTreeNode<T>* currentNode = root_;
    RadixTreeNode<T>* prevNode = nullptr;

    while (currentNode != prevNode && currentNode != nullptr) {
      list<RadixTreeNode<T>*>* children = currentNode->GetChildren();
      if (children->size() <= 0) {
        //throw std::exception();
        return nullptr; // Temp Retrun value;
        break;
      }

      prevNode = currentNode;
      // Loop through children
      for(typename list<RadixTreeNode<T>*>::iterator it = children->begin();
        it != children->end();
        ++it) {
        
        // compare name
        //  compare result cases 
        RadixTreeNode<T>* childNode = (RadixTreeNode<T>*) (*it);
        const string* childName = childNode->GetName();
        
        string nameToCompare = name.substr(nameCurrentIndex);

        // _compareName returns number of chars matched from the beginning. if nothing found, -1
        int nameCompareResult = _compareName(nameToCompare, (*childName));

        DEBUG_cout << "ChildNameLength: " << childName->length() << endl;
        DEBUG_cout << "NameLength: " << nameToCompare.length() << endl;
        if (nameCompareResult > 0) {
          nameCurrentIndex += nameCompareResult;

          if (childName->length() > nameToCompare.length()) {
            // No match found
            DEBUG_cout << "Not found " << name << endl;
            return nullptr;
          } else if (childName->length() < nameToCompare.length()) {
            currentNode = childNode;
            break;
          } else if (childName->length() == nameToCompare.length()) {
            if(nameToCompare.length() == (unsigned int) nameCompareResult) {
              // BINGO
              DEBUG_cout << "Found node " << name << endl;
              return childNode;
            } else if (nameToCompare.length() > (unsigned int) nameCompareResult){
              // go to child.
              currentNode = childNode;
              break;
            }
          }
        }
      }
    }
    DEBUG_cout << "Not found " << name << endl;
    return nullptr;
  }


  /*
  T GetValueByName(const string& name) {
    DEBUG_cout << "RadixTree::GetValueByName(string) started." << endl;
    RadixTreeNode<T>* foundNode = GetNodeByName(name);
    if (foundNode != nullptr) {
      return foundNode->GetValue();
    } else {
    }
    
    return T();
  }
  */
   
  void Insert(const string& name, const T value) {
    //DEBUG_cout << "Inserting " << name << " " << value << endl;

    unsigned int nameCurrentIndex = 0;
    RadixTreeNode<T>* parentNode = root_;
    RadixTreeNode<T>* prevNode = nullptr;

    // Tree Loop
    while (parentNode != nullptr) {

      list<RadixTreeNode<T>*>* children = parentNode->GetChildren();

      string nameToCompare = name.substr(nameCurrentIndex);

      bool flag = false;

      // Children List Loop
      for (typename list<RadixTreeNode<T>*>::iterator it = children->begin();
         it != children->end();
         ++it) {

        RadixTreeNode<T>* childNode = (RadixTreeNode<T>*) (*it);
        const string* childName = childNode->GetName();

        unsigned int nameCompareResult = _compareName(nameToCompare, (*childName));
        if (nameCompareResult > 0) {
          if (nameToCompare.length() == nameCompareResult) {
            // if All same,
            // Do nothing.
            DEBUG_cout << "Node already exists." << endl;
            /* THIS SECTION IS REMOVED 10/23/2013. CHECK IF AFFECTS THE PROGRAM
            if (childNode->GetValue() == T()) { // DataBlock() == nullptr
              // if node's value is NULL
              DEBUG_cout << "Node value is NULL. New value is set." << endl;
              childNode->SetValue(value);
            } else {
              DEBUG_cout << "Node value is Not Null. Insertion ignored." << endl;
            }
            */
            return; 
          } else if (nameToCompare.length() > nameCompareResult) {
            if (nameCompareResult == childName->length()) {
              flag = true;
              parentNode = childNode;
              nameCurrentIndex += nameCompareResult;
              break;
            }
            // if partially same
            //  Create common parent.
            //  Pop Child Node from currentNode's children list
            //  newCommonParent->AddChild(poppedNode)
            //  newCommonParent->AddChild(newNode)
            //  PoppedNode->SetParent(newCommonParent)
            //  ParentNode->AddChild(newCommonParent)

            nameCurrentIndex += nameCompareResult;

            string newCommonParentName = nameToCompare.substr(0, nameCompareResult);
            RadixTreeNode<T>* newCommonParentNode = new RadixTreeNode<T>(newCommonParentName, NULL, childNode->GetParent());
            RadixTreeNode<T>* newNode = new RadixTreeNode<T>(nameToCompare.substr(nameCompareResult), value, newCommonParentNode);

            RadixTreeNode<T>* poppedNode = childNode;


            poppedNode->SetName(poppedNode->GetName()->substr(nameCompareResult));
            poppedNode->SetParent(newCommonParentNode);
            
            newCommonParentNode->AddChild(poppedNode);
            newCommonParentNode->AddChild(newNode);
            
            newCommonParentNode->SetParent(parentNode);

            parentNode->AddChild(newCommonParentNode);

            children->erase(it);
            DEBUG_cout << "Node Inserted after creating CommonParent " << "'" << newCommonParentName << "'" << endl;

            nodeCount_++;
            return;
          }
          // same but there are still remaining chars
          parentNode = childNode;
          flag = true;
          break;
        } else {
          // Go to next child. Continue "Children List" Looping
        }
      }

      if (flag == false) {
        // if no matching child is found
        RadixTreeNode<T>* newNode = new RadixTreeNode<T>(name.substr(nameCurrentIndex), value, parentNode);
        parentNode->AddChild(newNode);
        DEBUG_cout << "Inserted." << endl;
        nodeCount_++;
        break;
      }
      //continue loop;
    }
  }


  void _PrintChildren(const string& name) {
    cout << "Print Children of " << name << endl;
    RadixTreeNode<T>* foundNode = GetNodeByName(name);
    if (foundNode != nullptr) {
      list<RadixTreeNode<T>*>* children = foundNode->GetChildren();

      for (typename list<RadixTreeNode<T>*>::iterator it = children->begin();
         it != children->end();
         ++it) {

        RadixTreeNode<T>* childNode = (RadixTreeNode<T>*) (*it);
        const string* childName = childNode->GetName();
        T childValue = childNode->GetValue();

        cout << childName->c_str() << ": " << childValue << endl;
      }
    }
  }

  int _CompareName (const string& name, const string& childName) {
    return _compareName(name, childName);
  }
protected:
  
private:
  RadixTreeNode<T>* root_;
  unsigned int nodeCount_;

  int _compareName(const string& name, const string& childName) {
    unsigned int i, count = 0;
    for (i = 0; childName.length() > i && name.length() > i; i++) {
      if (name[i] == childName[i]) {
        count += 1; // Count increases when they match.
      } else {
        break;
      }
    }
    DEBUG_cout << "_compareName Result: " << count << endl;
    return count;
  }
};


} // namespace lio close
#endif
