#include <regression/tframe.h>
#include "NullDictionary.h"

#include <iostream>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace NullDictionaryTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  {
    // Should succeed
    NullDictionary dict;
  }

  {
    // Should succeed
    NullDictionary* dict = new NullDictionary();
    // Should succeed
    delete dict;
  }

  {
    // Should succeed
    NullDictionary dict;
    // Should succeed
    NullDictionary dict2(dict);
    // Should succeed
    NullDictionary dict3 = dict;
    // Should succeed
    NullDictionary dict4;
    dict4 = dict;
  }

  TEST_PASSED();
}

//! Test init()
void init(void)
{
  using namespace TextGen;
  NullDictionary dict;

  // Should do nothing
  dict.init("foobar");
  if (!dict.empty()) TEST_FAILED("dict should be empty after init");

  TEST_PASSED();
}

//! Test language()
void language(void)
{
  using namespace TextGen;
  NullDictionary dict;

  if (!dict.language().empty()) TEST_FAILED("dict.language() should be empty before init");

  dict.init("fi");
  if (dict.language() != "fi") TEST_FAILED("dict.language() did not return init argument");

  TEST_PASSED();
}

//! Test insert()
void insert(void)
{
  using namespace TextGen;
  NullDictionary dict;

  try
  {
    dict.insert("foo", "bar");
    TEST_FAILED("insert() should always throw");
  }
  catch (...)
  {
  }

  TEST_PASSED();
}

//! Test empty
void empty(void)
{
  using namespace TextGen;
  NullDictionary dict;

  if (!dict.empty()) TEST_FAILED("empty() should return true after construction");

  TEST_PASSED();
}

//! Test size
void size(void)
{
  using namespace TextGen;
  NullDictionary dict;

  if (dict.size() != 0) TEST_FAILED("size() should return 0 after construction");

  TEST_PASSED();
}

//! Test contains
void contains(void)
{
  using namespace TextGen;
  NullDictionary dict;

  if (dict.contains("foo")) TEST_FAILED("contains() should return false for empty dictionary");

  TEST_PASSED();
}

//! Test find()
void find(void)
{
  using namespace TextGen;
  NullDictionary dict;

  try
  {
    dict.find("foo");
    TEST_FAILED("find() should throw for empty dictionary");
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

}  // namespace NullDictionaryTest

int main(void)
{
  cout << endl << "NullDictionary tester" << endl << "=====================" << endl;
  NullDictionaryTest::tests t;
  return t.run();
}
