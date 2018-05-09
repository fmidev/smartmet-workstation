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
							,NFmiDrawingEnvironment * theDrawingEnvi
							,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							,FmiParameterName theParamId
							,int theRowIndex
                            ,int theColumnIndex);
    virtual ~NFmiWmsView(void) = default;
	void Draw (NFmiToolBox * theGTB);
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);

protected:
    int itsRowIndex; // monennessako riviss� n�ytt�ruudukossa t�m� n�ytt� sijaitsee
    int itsColIndex;
	
	double itsScreenPixelSizeInMM; // t�h�n pit�� saada talteen pikselin koko mm:eiss� silloin kun ei olla printtaamassa, 
								// koska viivan paksuu on annettu ruudulla n�kyviss� pikselikoossa.

    long itsLatestParam = -1;
    long itsLatestProducer = -1;

};

#endif // DISABLE_CPPRESTSDK

