//© Ilmatieteenlaitos/software by Marko
//  Original 08.08.2003
//
//
//-------------------------------------------------------------------- NFmiSatelView.h

#pragma once

#include "NFmiStationView.h"
#include "NFmiParameterName.h"
#include "NFmiSatelliteImageCacheHelpers.h"

//_________________________________________________________ NFmiSatelView

class NFmiToolBox;
class NFmiArea;

namespace Gdiplus
{
	class Bitmap;
}

class NFmiSatelView : public NFmiStationView
{

public:
	NFmiSatelView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
				,NFmiToolBox * theToolBox
				,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				,FmiParameterName theParamId
				,int theRowIndex
                ,int theColumnIndex);
    ~NFmiSatelView(void);
	void Draw (NFmiToolBox * theGTB) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
    static long ImagesOffsetInMinutes(const NFmiDataIdent &theDataIdent);

protected:
    void DrawImageOnDifferentProjection(boost::shared_ptr<NFmiArea> &theImageArea, NFmiImageHolder &theImageHolder);
    NFmiImageHolder GetImageFromCache();

	NFmiMetTime itsSatelImageTime; // otetaan talteen näytölle piirretyn kuvan aika
};

