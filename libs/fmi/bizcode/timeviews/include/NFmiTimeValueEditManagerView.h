#pragma once

#include "NFmiCtrlView.h"
#include "NFmiRect.h"

class NFmiCtrlViewList;
class NFmiToolBox;
class NFmiTimeSerialView;
class NFmiStepTimeScale;
class NFmiTimeScaleView;
class NFmiTimeBag;
class NFmiTimeControlView;


class NFmiTimeValueEditManagerView : public NFmiCtrlView
{

 public:
	NFmiTimeValueEditManagerView (const NFmiRect & theRect
								,NFmiToolBox * theToolBox);
	~NFmiTimeValueEditManagerView(void);
	virtual void Update (const NFmiRect & theRect, NFmiToolBox * theToolBox = 0) override;
	virtual void Update (void) override;
	void Draw (NFmiToolBox* theToolBox) override;
	virtual void DrawData (void);
	virtual void DrawBase (void);
	void MaxStationShowed (const unsigned int & newCount);
	bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool LeftDoubleClick(const NFmiPoint& thePlace, unsigned long theKey) override;
	bool RightButtonUp(const NFmiPoint& thePlace, unsigned long theKey) override;
	bool LeftButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool RightButtonDown (const NFmiPoint & thePlace, unsigned long theKey) override;
	bool MouseMove (const NFmiPoint &thePlace, unsigned long theKey) override;
    bool MouseWheel(const NFmiPoint &thePlace, unsigned long theKey, short theDelta) override;
	bool MiddleButtonUp(const NFmiPoint & thePlace, unsigned long theKey) override;
	bool ChangeTimeSeriesValues(void);
	void ResetAllModifyFactorValues(void);
	NFmiCtrlView* ActiveView(void);
	std::string ComposeToolTipText(const NFmiPoint& theRelativePoint) override;
	bool TimeScaleUpdated(void) const {return fTimeScaleUpdated;}
	void TimeScaleUpdated(bool newValue) {fTimeScaleUpdated = newValue;}
	bool AutoAdjustValueScales(bool fJustActive);
	std::string MakeCsvDataString() override;

 protected:
	void UpdateTimeSerialViews(void);
	NFmiRect CalcUpperTimeAxisRect(void);
	void CreateTimeAxisView(const NFmiTimeBag& theTimeBag);
	void DrawTimeAxisView(void);
	NFmiRect CalcListViewRect(int theIndex);
	void DrawBackground (void);
	virtual NFmiTimeSerialView* CreateTimeSerialView(boost::shared_ptr<NFmiDrawParam> &theDrawParam, int index);
	double CalcTimeAxisHeight(void);
	int CalcRowIndex(const NFmiPoint& thePlace);
	void DoNeededTimeSerialMacroParamInfoUpdates();

	//laura siirsi seuraavat protectediksi
	NFmiCtrlViewList *itsViewList;
	int itsViewCount; // ei tarvita välttämättä, koska luku saadaan drawparamlist:ilta

 private:
	bool FindView(NFmiCtrlViewList* theViewList, boost::shared_ptr<NFmiDrawParam> &theDrawParam);


	int itsSymbolViewCount;
// nykyään managerissa on ylhäällä kaikille aikasarjanäytöille yhteinen aikasarja-akseli
	NFmiStepTimeScale *itsTimeAxis;
	NFmiTimeControlView *itsUpperTimeView;

	bool fTimeScaleUpdated; // tämän avulla voidaan pakottaa tietyissä tapauksissa karttanäyttö päivittymään, kun ollaan säädetty aikakontrolli ikkunaa
};

