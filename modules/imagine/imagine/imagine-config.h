/*
* imagine-config.h
*
* Defines whether Cairo or non-cairo (self made) rendering is used.
*
* This is in a config file (and not Makefile) so that other projects using
* Imagine will get the headers right automatically.
*/
#pragma once

#ifdef IMAGINE_WITH_CAIRO
#error "Do NOT define IMAGINE_WITH_CAIRO in a Makefile"
#endif

// Comment this out for original (self made) rendering
// #define IMAGINE_WITH_CAIRO

// Having this typedef makes many either-or places shorter (could also be
// made a class of its own, but this is how it ended in summer-08) --AKa
//
#ifdef IMAGINE_WITH_CAIRO
class ImagineXr;
typedef ImagineXr ImagineXr_or_NFmiImage;
#else
namespace Imagine
{
class NFmiImage;
}
typedef Imagine::NFmiImage ImagineXr_or_NFmiImage;
#endif

