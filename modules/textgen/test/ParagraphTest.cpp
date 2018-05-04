#include <regression/tframe.h>
#include "DictionaryFactory.h"
#include "Paragraph.h"
#include "Sentence.h"

#include <newbase/NFmiSettings.h>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/locale.hpp>

using namespace std;
using namespace boost;

namespace ParagraphTest
{
//! Test structors
void structors(void)
{
  using namespace TextGen;

  {
    // Should succeed
    Paragraph p;
  }

  {
    // Should succeed
    Paragraph* p = new Paragraph();
    // Should succeed
    delete p;
  }

  {
    // Should succeed
    Paragraph p;
    // Should succeed
    Paragraph p2(p);
    // Should succeed
    Paragraph p3 = p;
    // Should succeed
    Paragraph p4;
    p4 = p;
  }

  TEST_PASSED();
}

//! Test empty()
void empty(void)
{
  using namespace TextGen;

  Paragraph p1;
  if (!p1.empty()) TEST_FAILED("paragraph should be empty after void constructor");

  Sentence s;
  s << "foobar";
  Paragraph p2;
  p2 << s;
  if (p2.empty()) TEST_FAILED("paragraph should not be empty after explicit construction");

  Paragraph p3(p2);
  if (p3.empty()) TEST_FAILED("paragraph should not be empty after copy construction");

  TEST_PASSED();
}

//! Test size()
void size(void)
{
  using namespace TextGen;

  Paragraph p1;
  if (p1.size() != 0) TEST_FAILED("paragraph size should be 0 after void constructor");

  Sentence s;
  s << "foobar";
  Paragraph p2;
  p2 << s;
  if (p2.size() != 1) TEST_FAILED("paragraph size should be 1 after explicit construction");

  Paragraph p3(p2);
  if (p3.size() != 1) TEST_FAILED("paragraph size should be 1 after copy constructor");

  TEST_PASSED();
}

//! Test operator<<
void appending(void)
{
  using namespace TextGen;

  Sentence s1;
  Sentence s2;
  Sentence s3;
  s1 << "a";
  s2 << "b";
  s3 << "c";

  Paragraph p1;
  Paragraph p2;
  p1 << s1;
  p2 << s2;

  p1 << p2;
  if (p1.size() != 2) TEST_FAILED("size after a << b is not 2");

  p1 << s3;
  if (p1.size() != 3) TEST_FAILED("size after ab << c is not 3");

  p1 << p1;
  if (p1.size() != 6)
  {
    cout << p1.size() << endl;
    TEST_FAILED("size after abc << abc is not 6");
  }

  Paragraph p;
  p << s1 << s2 << s3;
  if (p.size() != 3) TEST_FAILED("size after << b << c << d is not 3");

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

}  // namespace ParagraphTest

int main(void)
{
  boost::locale::generator generator;
  std::locale::global(generator(""));

  NFmiSettings::Init();
  NFmiSettings::Set("textgen::database", "textgen2");

  cout << endl << "Paragraph tester" << endl << "================" << endl;
  ParagraphTest::tests t;
  return t.run();
}
