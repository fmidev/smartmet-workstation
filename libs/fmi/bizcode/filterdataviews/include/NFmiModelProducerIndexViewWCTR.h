// NFmiModelProducerIndexViewWCTR.h: interface for the NFmiModelProducerIndexViewWCTR class.
//
// Lataus dialogissa (CFmiDataLoadDialog) on näyttö, joka näyttää kullekin
// ajanhetkelle valitun tuottajan indeksin värinä aika-ruudukossa. Hiirellä
// voidaan muutella valintoja sallituissa rajoissa (onko kyseisen tuottajan datassa 
// tietylle ajanhetkelle dataa?). Tämä on suunniteltu ottamaan huomioon mm. aikalista.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiModelCombineStatusView.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiPoint.h"

class NFmiQueryInfo;
class NFmiStepTimeScale;
class NFmiTimeScaleView;
class NFmiAxisView;	
class NFmiAxis;

class NFmiModelProducerIndexViewWCTR : public NFmiModelCombineStatusView
{

public:
	void Draw(NFmiToolBox * theGTB);
	NFmiModelProducerIndexViewWCTR(NFmiToolBox * theToolBox
							 ,NFmiDrawingEnvironment * theDrawingEnvi
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,const NFmiRect& theRect);
	virtual  ~NFmiModelProducerIndexViewWCTR();
	void Update(void);
	void SetDataFromDialog(	 std::vector<boost::shared_ptr<NFmiQueryInfo> > &theQueryInfoVector
							,const NFmiTimeDescriptor& theLoadedDataTimeDescriptor
							,const std::vector<int>& theSelectedProducerPriorityTable
							,const std::vector<NFmiColor>& theProducerColorTable
							,int theActiveProducerIndex);

	void ActiveProducerIndex(int newValue){itsActiveProducerIndex = newValue;}
	const std::vector<int>& ProducerIndexInTimeVector(void) const {return itsProducerIndexInTimeVector;};
	bool IsProducerIndexInTimeVectorFilled(void);
	bool SelectSecondProducerFromThisTimeOn(int theTimeIndex);

	bool LeftButtonDown(const NFmiPoint &thePlace, unsigned long theKey);
	bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
	bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
	bool MouseMove(const NFmiPoint & thePlace, unsigned long theKey);

protected:
	virtual NFmiRect CalcStatusGridViewSize(void);
	virtual NFmiColor CheckStatusBoxColor(int theTimeIndex);
	virtual bool IsViewDrawed(void);
    const NFmiTimeBag& MaximalCoverageTimeBag();
private:
	void DrawBlendingArea(void);
	void DrawBackground(void);
	void UpdateHelperData(void);
	void InitProducerIndexInTimeVector(void);
	void CalcSourceDataExistLimits(void);
	void FillProducerIndexInTimeVector(void);
	NFmiPoint CalcTimeIndexRange(NFmiTimeDescriptor& thePrimeTimeDescriptor, NFmiTimeDescriptor& theCheckedTimeDescriptor);
	void FillNonMarkedRange(std::vector<int>& theIndexVector, const NFmiPoint& theRange, int theIndex);
	NFmiRect CalcTimeAxisRect(void);
	NFmiRect CalcHourValueAxisRect(void);
	void CreateTimeAxis(void);
	void CreateHourValueAxis(void);
	void DrawHourValueView(void);
	bool SetProducerIndex(int theIndex);
	bool SetProducerIndex(int theIndex, int theProducerIndex);
	int GetPlaceIndexInProducerColorIndexView(const NFmiPoint& thePlace);
	bool IndexInsideRange(const NFmiPoint& theRange, int theIndex);
	double Time2Value(const NFmiMetTime& theTime);
	NFmiMetTime Value2Time(const NFmiPoint& thePoint);

	std::vector<boost::shared_ptr<NFmiQueryInfo> > itsQueryInfoVector; // tässä on ladattavien datojen tietoja
	std::vector<int> itsProducerIndexInTimeVector;
	NFmiTimeDescriptor itsLoadedDataTimeDescriptor; // tämän mukaan eletään ja tehdään näyttöä (tälle ajalle ladataan dataa)
	int itsActiveProducerIndex; // millä tuottajalla täytetään mahdollinen ei editoitava alue
	std::vector<int> itsSelectedProducerPriorityTable; // radio buttonien asetukset
	std::vector<NFmiColor> itsProducerColorTable; // eri tuottajille on eri värit
	std::vector<NFmiPoint> itsSourceDataExistLimits; // tähän talletetaan eri datojen aika riittävyydet lopullisessa vectorissa
													// Eli esim. arvo 3, 15 tarkoittaa että kyseinen data riittää indekseille 3-15 (0:sta alkaa)

	// tämä hoitelee normaalin aika-akselin
	NFmiStepTimeScale* itsTimeAxis;
	NFmiTimeScaleView* itsTimeView;
	NFmiRect itsTimeAxisRect;

	// tämä hoitelee 0-168h akselin
	NFmiAxisView* itsHourValueView;	
	NFmiAxis* itsHourValueAxis;
	NFmiRect itsHourValueAxisRect;

	NFmiRect itsProducerColorIndexViewRect; // itseasiassa emo luokanb näytön koko

	bool fProducerSelectionMouseCaptured; // nimi monimutkainen, että emojen vastaaviin captureihin ei mene sekaisin
};

