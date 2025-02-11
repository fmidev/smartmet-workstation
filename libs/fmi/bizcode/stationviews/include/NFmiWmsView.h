#pragma once
#ifndef DISABLE_CPPRESTSDK

#include "NFmiStationView.h"
#include "NFmiParameterName.h"
#include "xmlliteutils/XMLite.h"

class NFmiToolBox;
class NFmiArea;
namespace Gdiplus
{
	class PointF;
    class Bitmap;
    class GraphicsPath;
    class Font;
}

class NFmiWmsView : public NFmiStationView
{

public:
    NFmiWmsView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
							,NFmiToolBox * theToolBox
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,int theRowIndex
                            ,int theColumnIndex);
    ~NFmiWmsView(void) = default;
	void Draw (NFmiToolBox * theGTB) override;
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;

protected:
	double itsScreenPixelSizeInMM; // t‰h‰n pit‰‰ saada talteen pikselin koko mm:eiss‰ silloin kun ei olla printtaamassa, 
								// koska viivan paksuu on annettu ruudulla n‰kyviss‰ pikselikoossa.

};

#endif // DISABLE_CPPRESTSDK

