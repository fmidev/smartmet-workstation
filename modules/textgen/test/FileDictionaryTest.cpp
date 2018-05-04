#include <regression/tframe.h>
#include "FileDictionary.h"
#include <calculator/TextGenError.h>
#include <calculator/Settings.h>
#include <newbase/NFmiSettings.h>
#include <iostream>
#include <string>

#include <boost/locale.hpp>

using namespace std;

namespace FileDictionaryTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  {
    // Should succeed
    FileDictionary dict;
  }

  {
    // Should succeed
    FileDictionary* dict = new FileDictionary();
    // Should succeed
    delete dict;
  }

  {
    // Should succeed
    FileDictionary dict;
    // Should succeed
    FileDictionary dict2(dict);
    // Should succeed
    FileDictionary dict3 = dict;
    // Should succeed
    FileDictionary dict4;
    dict4 = dict;
  }

  TEST_PASSED();
}

//! Test init() method

void init(void)
{
  using namespace TextGen;
  FileDictionary dict;

  // Should succeed
  dict.init("fi");
  dict.init("en");

  // Should throw since Russian is not active
  try
  {
    dict.init("ru");
    TEST_FAILED("init('ru') should have failed");
  }
  catch (const TextGenError& e)
  {
  }
  TEST_PASSED();
}

//! Test language()
void language(void)
{
  using namespace TextGen;
  FileDictionary dict;

  if (!dict.language().empty()) TEST_FAILED("language should be empty before init");

  dict.init("fi");
  if (dict.language() != "fi") TEST_FAILED("language should match init argument");

  TEST_PASSED();
}

//! Test empty() method
void empty(void)
{
  using namespace TextGen;
  FileDictionary dict;

  if (!dict.empty()) TEST_FAILED("empty() should return true before init()");

  dict.init("fi");
  if (dict.empty()) TEST_FAILED("empty() should return true after init()");

  TEST_PASSED();
}

//! Test size() method
void size(void)
{
  using namespace TextGen;
  FileDictionary dict;

  if (dict.size() != 0) TEST_FAILED("size() should return 0 before init()");

  dict.init("fi");
  if (dict.size() == 0) TEST_FAILED("size() should return > 0 after init(fi)");

  TEST_PASSED();
}

//! Test contains() method
void contains(void)
{
  using namespace TextGen;
  FileDictionary dict;

  if (dict.contains("foobar")) TEST_FAILED("contains(foobar) should have failed");

  dict.init("fi");
  if (dict.contains("foobar")) TEST_FAILED("contains(foobar) should have failed");
  if (!dict.contains("sama")) TEST_FAILED("contains(sama) should have succeeded");

  TEST_PASSED();
}

//! Test find() method
void find(void)
{
  using namespace TextGen;
  FileDictionary dict;

  try
  {
    dict.find("foobar");
    TEST_FAILED("find() should throw before init()");
  }
  catch (const TextGenError& e)
  {
  }

  dict.init("fi");

  try
  {
    dict.find("foobar");
    TEST_FAILED("find(foobar) should have thrown");
  }
  catch (const TextGenError& e)
  {
  }

  if (dict.find("sama") != "sama") TEST_FAILED("find(sama) should have returned sama");

  dict.init("en");
  if (dict.find("sama") != "the same") TEST_FAILED("find(sama) should have returned the same");

  TEST_PASSED();
}

//! Test insert
void insert(void)
{
  using namespace TextGen;
  FileDictionary dict;

  try
  {
    dict.insert("foo", "bar");
    TEST_FAILED("insert(foo,bar) should throw even before init()");
  }
  catch (const TextGenError& e)
  {
  }

  dict.init("fi");

  try
  {
    dict.insert("foo", "bar");
    TEST_FAILED("insert(foo,bar) should have thrown after init(fi)");
  }
  catch (const TextGenError& e)
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
    TEST(empty);
    TEST(size);
    TEST(contains);
    TEST(find);
    TEST(insert);
  }

};  // class tests

}  // namespace FileDictionaryTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");
  Settings::set(NFmiSettings::ToString());

  cout << endl << "FileDictionary tester" << endl << "=====================" << endl;
  FileDictionaryTest::tests t;
  return t.run();
}
