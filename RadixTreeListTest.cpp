#include "RadixTree.hpp"


#define _UNITTEST 1
#if _UNITTEST

/*void GetValueByNameTest(RadixTree<T>* rtree) {
    cout << "GetValueByNameTest" << endl;
    #define cout cout << "   "

    cout << "a: " << rtree->GetValueByName("a") << endl;
    //cout << "b: " << rtree->GetValueByName("b") << endl;
    //cout << "c: " << rtree->GetValueByName("c") << endl;
    cout << "abb: " << rtree->GetValueByName("abb") << endl;
    cout << "abc: " << rtree->GetValueByName("abc") << endl;
    cout << "abcd: " << rtree->GetValueByName("abcd") << endl;
    cout << "acc: " << rtree->GetValueByName("acc") << endl;
    cout << "add: " << rtree->GetValueByName("add") << endl;
    cout << "cb: " << rtree->GetValueByName("cb") << endl;
    cout << "bac: " << rtree->GetValueByName("bac") << endl;
    cout << "bb: " << rtree->GetValueByName("bb") << endl;
    cout << "bbb: " << rtree->GetValueByName("bbb") << endl;
    
    #undef cout
}*/

/*void CompareNameTest(RadixTree* rtree) {
    cout << "CompareNameTest" << endl;
    #define cout cout << "   "
    
    cout << "Result: " << rtree->_CompareName("ab", "cdb") << endl;
    cout << "Result: " << rtree->_CompareName("aa", "aabd") << endl;
    cout << "Result: " << rtree->_CompareName("aa", "a") << endl;
    cout << "Result: " << rtree->_CompareName("abc", "abc") << endl;
    cout << "Result: " << rtree->_CompareName("aaaa", "aac") << endl;
    cout << "Result: " << rtree->_CompareName("aaaa", "aaaa") << endl;
    #undef cout
}*/


/*void InsertTest(RadixTree* rtree) {
    rtree->Insert("a", 1);
    //rtree->Insert("b", 2);
    //rtree->Insert("c", 3);
    rtree->Insert("abb",122);
    rtree->Insert("abc",123);
    rtree->Insert("abcd",1234);
    rtree->Insert("acc",133);
    rtree->Insert("add",144);
    //rtree->Insert("bcd", 234);
}*/

int main() {
    RadixTree<int>* rtree = new RadixTree<int>();

    rtree->Insert("a", 1);
    rtree->Insert("aa", 1);
    rtree->Insert("b", 2);
    rtree->Insert("c", 3);
    rtree->Insert("abb",122);
    rtree->Insert("abc",123);
    rtree->Insert("abcd",1234);
    rtree->Insert("acc",133);
    rtree->Insert("add",144);
    rtree->_PrintChildren("a");
    rtree->_PrintChildren("ab");
    //CompareNameTest(rtree);

    


    //RadixTreeNode* newNode = new RadixTreeNode("a", 1);
}
#endif
