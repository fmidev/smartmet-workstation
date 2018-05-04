// ======================================================================
/*!
 * \file
 * \brief Implementation of class MessageLoggerStream
 */
// ======================================================================
/*!
 * \class MessageLoggerStream
 *
 * \brief Output stream services for MessageLogger.
 *
 * Represents a stream that holds a message until it's full,
 * and then, calls an Event function - onNewMessage
 *
 * A message is full when the stream buffer is flushed.
 * For instance, when std::endl is written to a stream.
 *
 * Usage: Derive your class from this, and implement the PROTECTED
 * onNewMessage function
 *
 * All code copied from CUJ February 2003, code by John Torjo.
 *
 */
// ======================================================================

#include "MessageLoggerStream.h"
