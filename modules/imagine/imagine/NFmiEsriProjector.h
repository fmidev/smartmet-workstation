// ======================================================================
//
// Abstract base class, from which any actual projector should be derived.
//
// History:
//
// 25.09.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include <newbase/NFmiDef.h>

namespace Imagine
{
class NFmiEsriPoint;  // introduce projector argument type
class NFmiEsriBox;

class _FMI_DLL NFmiEsriProjector
{
 public:
  virtual ~NFmiEsriProjector(void){};
  NFmiEsriProjector() {}
  virtual NFmiEsriPoint operator()(const NFmiEsriPoint& thePoint) const = 0;
  virtual void SetBox(const NFmiEsriBox& theBox) const = 0;
};

}  // namespace Imagine


// ======================================================================
