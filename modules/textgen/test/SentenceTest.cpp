#include <regression/tframe.h>
#include "DictionaryFactory.h"
#include "Integer.h"
#include "Delimiter.h"
#include "Sentence.h"

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace SentenceTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  {
    // Should succeed
    Sentence s;
  }

  {
    // Should succeed
    Sentence* s = new Sentence();
    // Should succeed
    delete s;
  }

  {
    // Should succeed
    Sentence s;
    // Should succeed
    Sentence s2(s);
    // Should succeed
    Sentence s3 = s;
    // Should succeed
    Sentence s4;
    s4 = s;
  }

  TEST_PASSED();
}

//! Test empty()
void empty(void)
{
  using namespace TextGen;

  Sentence s1;
  if (!s1.empty()) TEST_FAILED("sentence should be empty after void constructor");

  Sentence s2;
  s2 << "foobar";
  if (s2.empty()) TEST_FAILED("sentence should not be empty after adding phrase");

  Sentence s3(s2);
  if (s3.empty()) TEST_FAILED("sentence should not be empty after copy construction");

  TEST_PASSED();
}

//! Test size()
void size(void)
{
  using namespace TextGen;

  Sentence s1;
  if (s1.size() != 0) TEST_FAILED("sentence size should be 0 after void constructor");

  Sentence s2;
  s2 << "foobar";
  if (s2.size() != 1) TEST_FAILED("sentence size should be 1 after adding a phrase");

  Sentence s3(s2);
  if (s3.size() != 1) TEST_FAILED("sentence size should be 1 after copy constructor");

  TEST_PASSED();
}

//! Test operator<<
void appending(void)
{
  using namespace TextGen;

  {
    Sentence s1;
    Sentence s2;
    s1 << "a";
    s2 << "b";

    s1 << s2;
    if (s1.size() != 2) TEST_FAILED("size after a << b is not 2");

    s1 << string("c");
    if (s1.size() != 3) TEST_FAILED("size after ab << c is not 3");

    s1 << "d";
    if (s1.size() != 4) TEST_FAILED("size after abc << d is not 4");

    s1 << s1;
    if (s1.size() != 8) TEST_FAILED("size after abcd << abcd is not 8");

    Sentence s;
    s << "a";
    s << "b"
      << "c"
      << "d"
      << "e";
    if (s.size() != 5) TEST_FAILED("size after a << b << c << d << e is not 5");
  }

  {
    Integer num(1);
    Sentence s1;
    Sentence s2;
    s1 << num;
    s2 << 2;

    s1 << s2;
    if (s1.size() != 2) TEST_FAILED("size after 1 << 2 is not 2");

    s1 << 3;
    if (s1.size() != 3) TEST_FAILED("size after 12 << 3 is not 3");

    s1 << s1;
    if (s1.size() != 6) TEST_FAILED("size after 123 << 123 is not 6");

    Sentence s;
    s << 1 << 2 << 3 << 4 << 5;
    if (s.size() != 5) TEST_FAILED("size after << 1 << 2 << 3 << 4 << 5 is not 5");
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
    TEST(empty);
    TEST(size);
    TEST(appending);
  }

};  // class tests

}  // namespace SentenceTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  cout << endl << "Sentence tester" << endl << "===============" << endl;
  SentenceTest::tests t;
  return t.run();
}
