// NFmiModelCombineStatusView.h: interface for the NFmiModelCombineStatusView class.
//
// Näyttää filterdialogissa mallien yhdistämistyössä työstettävän datan 
// statuksen eri aika-askeleille. Käytännössä ruudukko eri värejä, ruutuja
// on yhtä monta kuin on työstettävän datan aika-askeleita. Punainen väri
// kertoo, että data on vasta alustettu puuttuvalla. Oranssi kertoo, että
// vain osassa parametreista on dataa, vihreä kertoo, että kaikilla 
// parametreilla on jotain dataa (siinä aika-askeleessa).
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
	virtual NFmiRect CalcStatusGridViewSize(void); // tämä tarvitaan lapsessa
	virtual NFmiTimeBag GetUsedTimeBag(void); // tämä tarvitaan lapsessa
	virtual NFmiColor CheckStatusBoxColor(int theTimeIndex);

	int itsGridXSize; // kuinka monta laatikkoa on aikahilassa
	boost::shared_ptr<NFmiFastQueryInfo> itsCheckedInfo;
};

