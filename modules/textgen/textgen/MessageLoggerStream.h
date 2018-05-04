// ======================================================================
/*!
 * \file
 * \brief Interface of class MessageLoggerStream
 */
// ======================================================================

#ifndef MESSAGELOGGERSTREAM_H
#define MESSAGELOGGERSTREAM_H

#include <ostream>
#include <sstream>
#include <memory>
#include <cassert>
#include <stdexcept>

#ifndef UNIX
#pragma warning(disable : 4512)  // poistaa VC++ k‰‰nt‰j‰n C4512 "assignment operator could not be
                                 // generated" varoituksen
#endif

// forward declaration(s)

template <class char_type = char, class CharTraits = std::char_traits<char_type> >
class MessageLoggerStream;

template <class char_type = char, class CharTraits = std::char_traits<char_type> >
class MessageLoggerStreambuf : public std::basic_streambuf<char_type, CharTraits>
{
 private:
  friend class MessageLoggerStream<char, CharTraits>;

  typedef std::basic_streambuf<char_type, CharTraits> streambuf_type;
  typedef MessageLoggerStream<char_type, CharTraits> ostream_type;
  typedef std::basic_streambuf<char_type, CharTraits> base_class;

  typedef typename CharTraits::int_type int_type;
  typedef typename CharTraits::pos_type pos_type;
  typedef typename CharTraits::off_type off_type;

  enum
  {
    _BADOFF = -1  // bad offset - for positioning functions
  };

 protected:
  // only for output, not for input
  virtual int_type pbackfail(int_type = CharTraits::eof())
  {
    throw std::runtime_error("MessageLoggerStream::pbackfail not available");
    return 0;
  }

  // only for output, not for input
  virtual std::streamsize showmanyc()
  {
    throw std::runtime_error("MessageLoggerStream::showmanyc not available");
    return 0;
  }

  // only for output, not for input
  virtual int_type underflow()
  {
    throw std::runtime_error("MessageLoggerStream::underflow not available");
    return 0;
  }

  // only for output, not for input
  virtual int_type uflow()
  {
    throw std::runtime_error("MessageLoggerStream::uflow not available");
    return 0;
  }

  // only for output, not for input
  virtual std::streamsize xsgetn(char_type* /* _S */, std::streamsize /* _N */)
  {
    throw std::runtime_error("MessageLoggerStream::xsgetn not available");
    return 0;
  }

  // we don't allow positioning
  virtual pos_type seekoff(off_type,
                           std::ios_base::seekdir,
                           std::ios_base::openmode = std::ios_base::in | std::ios_base::out)
  {
    throw std::runtime_error("MessageLoggerStream::seekoff not available");
    return 0;
  }

  // we don't allow positioning
  virtual pos_type seekpos(pos_type,
                           std::ios_base::openmode = std::ios_base::in | std::ios_base::out)
  {
    throw std::runtime_error("MessageLoggerStream::seekpos not available");
    return 0;
  }

  // output functions

  // called to write out from the internal buffer, into the external buffer
  virtual int sync()
  {
    itsOwnerStream->onNewMessage(GetStreamBuffer().str());
    itsStreamBuffer.reset(new StringStream);
    return 0;
  }

  virtual streambuf_type* setbuf(char_type* buffer, std::streamsize n)
  {
    // note: this function MUST be called
    // before working with this stream buffer

    // we don't use a buffer - we forward everything
    assert(buffer == NULL && n == 0);
    this->setp(NULL, NULL);

    return this;
  }

  // write the characters from the buffer
  // to their real destination
  virtual int_type overflow(int_type nChar = CharTraits::eof())
  {
    if (CharTraits::not_eof(nChar))
    {
      GetStreamBuffer() << char_type(nChar);
    }
    return CharTraits::not_eof(nChar);
  }

  virtual std::streamsize xsputn(const char_type* S, std::streamsize N)
  {
    GetStreamBuffer().write(S, N);
    return N;
  }

 public:
  MessageLoggerStreambuf() : itsStreamBuffer(new StringStream) {}
 private:
  typedef std::basic_ostringstream<char_type> StringStream;

  StringStream& GetStreamBuffer()
  {
    // allocate the buffer first !!!
    assert(itsStreamBuffer.get());
    return *itsStreamBuffer;
  }

 private:
  // holds the Message, until it's flushed
  std::auto_ptr<StringStream> itsStreamBuffer;

  // the Message Handler Stream - where we write into
  ostream_type* itsOwnerStream;
};

template <class char_type, class CharTraits>
class MessageLoggerStream : public std::basic_ostream<char_type, CharTraits>
{
  typedef MessageLoggerStreambuf<char_type, CharTraits> handler_streambuf_type;
  friend class MessageLoggerStreambuf<char_type, CharTraits>;

  typedef std::basic_ostream<char_type, CharTraits> base_class;

 protected:
  typedef std::basic_string<char_type> string_type;

  MessageLoggerStream() : base_class(NULL), itsStreamBuf()
  {
    itsStreamBuf.itsOwnerStream = this;
    this->init(&itsStreamBuf);
    itsStreamBuf.pubsetbuf(NULL, 0);
  }

  ~MessageLoggerStream() {}
 protected:
  virtual void onNewMessage(const string_type& strNewMessage) = 0;

 public:
  // our stream buffer
  handler_streambuf_type itsStreamBuf;
};

#endif  // MESSAGELOGGERSTREAM_H

// ======================================================================
