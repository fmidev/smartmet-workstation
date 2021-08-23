// NFmiModelCombineStatusView.h: interface for the NFmiModelCombineStatusView class.
//
// N�ytt�� filterdialogissa mallien yhdist�misty�ss� ty�stett�v�n datan 
// statuksen eri aika-askeleille. K�yt�nn�ss� ruudukko eri v�rej�, ruutuja
// on yht� monta kuin on ty�stett�v�n datan aika-askeleita. Punainen v�ri
// kertoo, ett� data on vasta alustettu puuttuvalla. Oranssi kertoo, ett�
// vain osassa parametreista on dataa, vihre� kertoo, ett� kaikilla 
// parametreilla on jotain dataa (siin� aika-askeleessa).
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiZoomView.h"
#include "NFmiColor.h"

class NFmiTimeBag;

class NFmiModelCombineStatusView : public NFmiZoomView
{

public:
   void Draw(NFmiToolBox * theGTB) override;
   NFmiModelCombineStatusView(NFmiToolBox * theToolBox
							 ,NFmiDrawingEnvironment * theDrawingEnvi
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,const NFmiRect& theRect);
	~NFmiModelCombineStatusView();
	void Update(void) override;

	bool LeftButtonDown(const NFmiPoint &thePlace, unsigned long theKey) override;
	bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool MouseMove(const NFmiPoint & thePlace, unsigned long theKey) override;

protected:
	virtual bool IsViewDrawed(void);
	virtual NFmiRect CalcStatusGridViewSize(void); // t�m� tarvitaan lapsessa
	virtual NFmiTimeBag GetUsedTimeBag(void); // t�m� tarvitaan lapsessa
	virtual NFmiColor CheckStatusBoxColor(int theTimeIndex);

	int itsGridXSize; // kuinka monta laatikkoa on aikahilassa
	boost::shared_ptr<NFmiFastQueryInfo> itsCheckedInfo;
};

