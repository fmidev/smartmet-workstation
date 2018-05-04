#include <regression/tframe.h>
#include "BasicDictionary.h"

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace BasicDictionaryTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  {
    // Should succeed
    BasicDictionary dict;
  }

  {
    // Should succeed
    BasicDictionary* dict = new BasicDictionary();
    // Should succeed
    delete dict;
  }

  {
    // Should succeed
    BasicDictionary dict;
    // Should succeed
    BasicDictionary dict2(dict);
    // Should succeed
    BasicDictionary dict3 = dict;
    // Should succeed
    BasicDictionary dict4;
    dict4 = dict;
  }

  TEST_PASSED();
}

//! Test init()
void init(void)
{
  using namespace TextGen;
  BasicDictionary dict;

  // Should do nothing
  dict.init("foobar");
  if (!dict.empty()) TEST_FAILED("dict should be empty after init");

  TEST_PASSED();
}

//! Test language()
void language()
{
  using namespace TextGen;
  BasicDictionary dict;

  if (!dict.language().empty()) TEST_FAILED("language should be empty before init");

  dict.init("fi");
  if (dict.language() != "fi") TEST_FAILED("language should match init argument");

  TEST_PASSED();
}

//! Test insert()
void insert(void)
{
  using namespace TextGen;
  BasicDictionary dict;

  dict.insert("foo", "bar");
  if (dict.empty()) TEST_FAILED("dict should not be empty after insert");
  if (dict.size() != 1) TEST_FAILED("dict size should be 1 after 1 insert");
  dict.insert("bar", "foo");
  if (dict.size() != 2) TEST_FAILED("dict size should be 2 after2 inserts");
  dict.insert("foo", "foo");
  if (dict.size() != 2) TEST_FAILED("dict size should not change after duplicate insert");

  TEST_PASSED();
}

//! Test empty
void empty(void)
{
  using namespace TextGen;
  BasicDictionary dict;

  if (!dict.empty()) TEST_FAILED("empty() should return true after construction");
  dict.insert("foo", "bar");
  if (dict.empty()) TEST_FAILED("empty() should return false after insert");

  TEST_PASSED();
}

//! Test size
void size(void)
{
  using namespace TextGen;
  BasicDictionary dict;

  if (dict.size() != 0) TEST_FAILED("size() should return 0 after construction");
  dict.insert("foo", "bar");
  if (dict.size() != 1) TEST_FAILED("size() should return 1 after 1 insert");
  dict.insert("bar", "foo");
  if (dict.size() != 2) TEST_FAILED("size() should return 2 after 2 inserts");

  TEST_PASSED();
}

//! Test contains
void contains(void)
{
  using namespace TextGen;
  BasicDictionary dict;

  if (dict.contains("foo")) TEST_FAILED("contains() should return false for empty dictionary");
  dict.insert("foo", "bar");
  if (!dict.contains("foo")) TEST_FAILED("contains(foo) returned false after insert(foo,bar)");
  if (dict.contains("bar")) TEST_FAILED("contains(bar) returned true after insert(foo,bar)");

  TEST_PASSED();
}

//! Test find()
void find(void)
{
  using namespace TextGen;
  BasicDictionary dict;

  try
  {
    dict.find("foo");
    TEST_FAILED("find() should throw for empty dictionary");
  }
  catch (...)
  {
  }

  dict.insert("foo", "bar");
  if (dict.find("foo") != "bar") TEST_FAILED("find(foo) should return bar after insert(foo,bar)");

  try
  {
    dict.find("bar");
    TEST_FAILED("find(bar) should throw after insert(foo,bar)");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

//! The actual test driver
class tests : public tframe::tests
{
  //! Overridden message separator
  virtual const char* error_message_prefix() const { return "\n\t"; }
  //! Main test suite
  void test(void)
  {
    TEST(structors);
    TEST(init);
    TEST(language);
    TEST(insert);
    TEST(empty);
    TEST(size);
    TEST(contains);
    TEST(find);
  }

};  // class tests

}  // namespace BasicDictionaryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  cout << endl << "BasicDictionary tester" << endl << "======================" << endl;
  BasicDictionaryTest::tests t;
  return t.run();
}
