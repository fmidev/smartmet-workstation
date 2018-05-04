// ======================================================================
/*!
 * \file NFmiViewingConstants.h
 * \brief nview ja ngram kirjastoissa tarvittavia enumeraatioita.
 *
 * \todo Siirr‰ n‰m‰ johonkin kirjastoon, miss‰ niit‰ oikeasti
 * k‰ytet‰‰nkin.
 */
// ======================================================================

#pragma once

//! Viivatyyppien enumeraatio, tyylit kopioitu WinGDI:st‰

enum FmiLineStyle
{
  kSolid          =  0,		//!< Viiva tyyliin -------
  kDash           =  1,		//!< Viiva tyyliin - - - -
  kDot            =  2,		//!< Viiva tyyliin .......
  kDashDot        =  3,		//!< Viiva tyyliin _._._._
  kDashDotDot     =  4,		//!< Viiva tyyliin _.._.._
  kLongDash       =  5		//!< Viiva tyyliin -- -- --
};


//! Erilaisten n‰kymien enumeraatio

enum FmiViewType
{
  kLineView,
  kBarView,
  kBarIntervalView,
  kNumberView,
  kSimpleWeatherView,
  kWeatherView,
  kWindBarbView
};

