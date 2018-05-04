// NFmiModelProducerIndexViewWCTR.h: interface for the NFmiModelProducerIndexViewWCTR class.
//
// Lataus dialogissa (CFmiDataLoadDialog) on n�ytt�, joka n�ytt�� kullekin
// ajanhetkelle valitun tuottajan indeksin v�rin� aika-ruudukossa. Hiirell�
// voidaan muutella valintoja sallituissa rajoissa (onko kyseisen tuottajan datassa 
// tietylle ajanhetkelle dataa?). T�m� on suunniteltu ottamaan huomioon mm. aikalista.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiModelCombineStatusView.h"
#include "NFmiTimeDescriptor.h"
#include "NFmiPoint.h"
#include "NFmiDataMatrix.h"

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
	void SetDataFromDialog(	 checkedVector<boost::shared_ptr<NFmiQueryInfo> > &theQueryInfoVector
							,const NFmiTimeDescriptor& theLoadedDataTimeDescriptor
							,const checkedVector<int>& theSelectedProducerPriorityTable
							,const checkedVector<NFmiColor>& theProducerColorTable
							,int theActiveProducerIndex);

	void ActiveProducerIndex(int newValue){itsActiveProducerIndex = newValue;}
	const checkedVector<int>& ProducerIndexInTimeVector(void) const {return itsProducerIndexInTimeVector;};
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
	void FillNonMarkedRange(checkedVector<int>& theIndexVector, const NFmiPoint& theRange, int theIndex);
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

	checkedVector<boost::shared_ptr<NFmiQueryInfo> > itsQueryInfoVector; // t�ss� on ladattavien datojen tietoja
	checkedVector<int> itsProducerIndexInTimeVector;
	NFmiTimeDescriptor itsLoadedDataTimeDescriptor; // t�m�n mukaan elet��n ja tehd��n n�ytt�� (t�lle ajalle ladataan dataa)
	int itsActiveProducerIndex; // mill� tuottajalla t�ytet��n mahdollinen ei editoitava alue
	checkedVector<int> itsSelectedProducerPriorityTable; // radio buttonien asetukset
	checkedVector<NFmiColor> itsProducerColorTable; // eri tuottajille on eri v�rit
	checkedVector<NFmiPoint> itsSourceDataExistLimits; // t�h�n talletetaan eri datojen aika riitt�vyydet lopullisessa vectorissa
													// Eli esim. arvo 3, 15 tarkoittaa ett� kyseinen data riitt�� indekseille 3-15 (0:sta alkaa)

	// t�m� hoitelee normaalin aika-akselin
	NFmiStepTimeScale* itsTimeAxis;
	NFmiTimeScaleView* itsTimeView;
	NFmiRect itsTimeAxisRect;

	// t�m� hoitelee 0-168h akselin
	NFmiAxisView* itsHourValueView;	
	NFmiAxis* itsHourValueAxis;
	NFmiRect itsHourValueAxisRect;

	NFmiRect itsProducerColorIndexViewRect; // itseasiassa emo luokanb n�yt�n koko

	bool fProducerSelectionMouseCaptured; // nimi monimutkainen, ett� emojen vastaaviin captureihin ei mene sekaisin
};

