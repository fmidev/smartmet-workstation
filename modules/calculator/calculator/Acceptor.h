// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Acceptor
 */
// ======================================================================

#ifndef TEXTGEN_ACCEPTOR_H
#define TEXTGEN_ACCEPTOR_H

namespace TextGen
{
class Acceptor
{
 public:
  Acceptor() {}
  virtual ~Acceptor() {}
  virtual bool accept(float theValue) const = 0;
  virtual Acceptor* clone() const = 0;

};  // class Acceptor
}  // namespace TextGen

#endif  // TEXTGEN_ACCEPTOR_H

// ======================================================================
