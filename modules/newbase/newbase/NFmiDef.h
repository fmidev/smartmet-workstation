// ======================================================================
/*!
 * \file NFmiDef.h
 * \brief Some globally essential definitions.
 */
// ======================================================================

#pragma once

//! FMI_DLL definition for MSVC linking
#if defined _MSC_VER || defined __BORLANDC__

#ifdef BUILD_FMIDLL
#define _FMI_DLL __declspec(dllexport)
#elif defined USE_FMIDLL
#define _FMI_DLL __declspec(dllimport)
#else
#define _FMI_DLL
#endif

#else

#define _FMI_DLL

#endif

//! Directory separator is system dependent

#ifdef UNIX
const unsigned char kFmiDirectorySeparator = '/';
#else
const unsigned char kFmiDirectorySeparator = '\\';
#endif

// ======================================================================
