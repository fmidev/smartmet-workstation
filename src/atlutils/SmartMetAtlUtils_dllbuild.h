
// SmartMetAtlUtils_dllbuild.h

#ifdef SMARTMETATLUTILS_BUILD
#define SMARTMETATLUTILS_USAGE __declspec(dllexport)
#else
#define SMARTMETATLUTILS_USAGE __declspec(dllimport)
#endif
