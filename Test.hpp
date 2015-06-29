#ifndef _TEST_HPP_
#define _TEST_HPP_

#ifndef _UNIT_TEST
#define _UNIT_TEST false
#endif

#if _UNIT_TEST

#define TEST if (true)

#include "liolib/Debug.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <chrono>

#include "Macro.hpp"

using std::string;
using std::vector;
using std::map;

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;


namespace lio {
namespace UnitTest {

struct TestResult {
  TestResult(const string& name, bool result) : name(name), result(result) {}

  const string name;
  bool result;
};

struct PerformanceTest {
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point end;
};


static unsigned int NumTest = 0;
static unsigned int NumTestSuccess = 0;
static unsigned int NumTestFail = 0;

static const bool MEANT_TO_BE_FAILED = true;

static vector<TestResult> Tests;
static map<string, PerformanceTest> PerformanceTests;


void PrintPerformanceTestResult(const string& testName);

template<class T>
bool Test(T testValue, T expectedValue,  const string testName = "Comparison Test", const bool meantToBeFailed = false) {
  NumTest += 1;

  bool result = (testValue == expectedValue);
  if (meantToBeFailed) {
    result = !result;
  }
  if (result) {
    cout << COLOR::GREEN << "PASS" << COLOR::END << "\t" << testName << endl;
    NumTestSuccess += 1;
  } else {
    cout << COLOR::RED << "FAIL" << COLOR::END << "\t" << testName << endl;
    NumTestFail += 1;
  }

  TestResult tr = TestResult(testName, result);
  Tests.push_back(tr);
  return result;
}



int ReportTestResult() {
  cout << "\n";
  cout << COLOR::BLUE << "================ TEST RESULT REPORT ==================" << COLOR::END << endl;
  for (auto& test : Tests) {
    string strResult = test.result ? COLOR::GREEN + "PASS" + COLOR::END : COLOR::RED + "FAIL" + COLOR::END;
    cout << strResult << "\t" << test.name << endl;
  }
  cout << "\n";
  cout << "Number of Tests: " << NumTest << endl;
  cout << "Number of Successful Tests: " << NumTestSuccess << endl;
  cout << "Number of Failed Tests: " << NumTestFail << endl;
  cout << "\n";

  for (auto& perfTest : PerformanceTests) {
    PrintPerformanceTestResult(perfTest.first);
  } 

  if (NumTestFail > 0) {
    return -1;
  } else {
    return NumTestSuccess;
  }
}



void PerformanceTestStart(const string& testName) {
  high_resolution_clock::time_point tpStart = high_resolution_clock::now();
  PerformanceTest test;
  test.start = tpStart;
  PerformanceTests[testName] = test;
}

void PerformanceTestEnd(const string& testName, const bool showResult = false) {
  high_resolution_clock::time_point tpEnd = high_resolution_clock::now();
  PerformanceTest* test = &(PerformanceTests[testName]);
  test->end = tpEnd;

  if (showResult)
    PrintPerformanceTestResult(testName);
}

std::chrono::nanoseconds GetTestTimeInNanoSeconds(const string& testName) {

  typedef typename map<string, PerformanceTest>::iterator mapit_t;

  mapit_t found = PerformanceTests.find(testName);
  if (found == PerformanceTests.end()) {
    cerr << COLOR::RED << "Performance Test Name Not Found." << COLOR::END << endl;
    return std::chrono::nanoseconds (0);
  }

  PerformanceTest test = PerformanceTests[testName];
  return std::chrono::duration_cast<nanoseconds>(test.end - test.start);
}

void PrintPerformanceTestResult(const string& testName) {

  typedef typename map<string, PerformanceTest>::iterator mapit_t;

  mapit_t found = PerformanceTests.find(testName);
  if (found == PerformanceTests.end()) {
    cerr << COLOR::RED << "Performance Test Name Not Found." << COLOR::END << endl;
    return;
  }

  PerformanceTest test = PerformanceTests[testName];

  milliseconds total_ms = std::chrono::duration_cast<milliseconds>(test.end - test.start);

  if (total_ms.count() < 10) {
    microseconds total_us = std::chrono::duration_cast<microseconds>(test.end - test.start);
    if (total_us.count() < 10) {
      nanoseconds total_ns = std::chrono::duration_cast<nanoseconds>(test.end - test.start);
      cout << "Performance Test Result for " << testName << ": " << endl << COLOR::BROWN <<
        std::setw(35) << std::right << total_ns.count() << " ns" << COLOR::END << endl;
    } else { 
      cout << "Performance Test Result for " << testName << ": " << endl << COLOR::BROWN <<
        std::setw(35) << std::right << total_us.count() << " us" << COLOR::END << endl;
    }
  } else {
    cout << "Performance Test Result for " << testName << ": " << endl << COLOR::BROWN <<
      std::setw(35) << std::right << total_ms.count() << " ms" << COLOR::END << endl;
  }
}


}
}

#else

#define TEST if (false)
#endif

#endif
