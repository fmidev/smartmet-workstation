// NFmiModelProducerIndexView.h: interface for the NFmiModelProducerIndexView class.
//
// Lataus dialogissa (CFmiDataLoadDialog) on n�ytt�, joka n�ytt�� kullekin
// ajanhetkelle valitun tuottajan indeksin v�rin� aika-ruudukossa. Hiirell�
// voidaan muutella valintoja sallituissa rajoissa (onko kyseisen tuottajan datassa 
// tietylle ajanhetkelle dataa?).
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NFmiModelCombineStatusView.h"
#include "NFmiTimeBag.h"
#include "NFmiPoint.h"
#include "NFmiDataMatrix.h"

class NFmiFastQueryInfo;
class NFmiStepTimeScale;
class NFmiTimeScaleView;
class NFmiAxisView;	
class NFmiAxis;

class NFmiModelProducerIndexView : public NFmiModelCombineStatusView
{

public:
	virtual void Draw(NFmiToolBox * theGTB);
	NFmiModelProducerIndexView(NFmiToolBox * theToolBox
							 ,NFmiDrawingEnvironment * theDrawingEnvi
							 ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
							 ,const NFmiRect& theRect);
	virtual  ~NFmiModelProducerIndexView();
	virtual void Update(void);
	void SetDataFromDialog( checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theQueryInfoVector
							,const NFmiTimeBag& theLoadedDataTimeBag
							,const NFmiTimeBag& theNonModifieableDataTimeBag
							,const checkedVector<int>& theSelectedProducerPriorityTable
							,const checkedVector<NFmiColor>& theProducerColorTable
							,int theNonModifieableTimeRangeProducerIndex
							,int theActiveProducerIndex); // tuli hieman mega-luokan interface

	void ActiveProducerIndex(int newValue){itsActiveProducerIndex = newValue;}
	const checkedVector<int>& ProducerIndexInTimeVector(void) const {return itsProducerIndexInTimeVector;};
	bool IsProducerIndexInTimeVectorFilled(void);
	void SetExtraShortRangeData(bool useExtraData, const NFmiTimeBag& theExtraShortRangeTimeBag, NFmiTimeBag& theLoadedDataTimeBag); // joutuu antamaan my�s loadedtimebagin (huonon suunnittelun takia!!)

	bool LeftButtonDown(const NFmiPoint &thePlace, unsigned long theKey);
	bool LeftButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
	bool RightButtonUp(const NFmiPoint & thePlace, unsigned long theKey);
	bool MouseMove(const NFmiPoint & thePlace, unsigned long theKey);

protected:
	virtual NFmiRect CalcStatusGridViewSize(void);
	virtual NFmiColor CheckStatusBoxColor(int theTimeIndex);
	virtual bool IsViewDrawed(void);
	virtual NFmiTimeBag GetUsedTimeBag(void);

private:
	void DrawBackground(void);
	void UpdateHelperData(void);
	void InitProducerIndexInTimeVector(void);
	void CalcNonModifieableTimeRange(void);
	void CalcSourceDataExistLimits(void);
	void FillProducerIndexInTimeVector(void);
	NFmiPoint CalcTimeIndexRange(NFmiTimeBag& thePrimeTimeBag, NFmiTimeBag& theCheckedTimeBag, bool fCalcNormalRange);
	void CutWithNonEditableRange(NFmiPoint* theRange);
	void FillNonMarkedRange(checkedVector<int>& theIndexVector, const NFmiPoint& theRange, int theIndex);
	NFmiRect CalcTimeAxisRect(void);
	NFmiRect CalcHourValueAxisRect(void);
	void CreateTimeAxis(void);
	void CreateHourValueAxis(void);
	void DrawHourValueView(void);
	bool SetProducerIndex(int theIndex);
	int GetPlaceIndexInProducerColorIndexView(const NFmiPoint& thePlace);
	bool IndexInsideRange(const NFmiPoint& theRange, int theIndex);

	checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > itsQueryInfoVector; // t�ss� on ladattavien datojen tietoja
	checkedVector<int> itsProducerIndexInTimeVector;
	NFmiTimeBag itsLoadedDataTimeBag; // t�m�n mukaan elet��n ja tehd��n n�ytt�� (t�lle ajalle ladataan dataa)
	bool fUseNonModifieableDataTimeBag; // onko estoja datan editoinnille olemassa?
	NFmiTimeBag itsNonModifieableDataTimeBag; // t�ss� kerrotaan mm. kepan esto 12 ensimm�iselle tunnille
	NFmiPoint itsNonModifieableTimeRange; // jos fUseModifieableDataTimeBag=false, tulee arvoiksi -1 - -1 
	int itsNonModifieableTimeRangeProducerIndex; // mill� tuottajalla t�ytet��n mahdollinen ei editoitava alue
	int itsActiveProducerIndex; // mill� tuottajalla t�ytet��n mahdollinen ei editoitava alue
	checkedVector<int> itsSelectedProducerPriorityTable; // radio buttonien asetukset
	checkedVector<NFmiColor> itsProducerColorTable; // eri tuottajille on eri v�rit
	checkedVector<NFmiPoint> itsSourceDataExistLimits; // t�h�n talletetaan eri datojen aika riitt�vyydet lopullisessa vectorissa
													// Eli esim. arvo 3, 15 tarkoittaa ett� kyseinen data riitt�� indekseille 3-15 (0:sta alkaa)

	// Kepa-editorissa shortrange extra datan auto lataus kuvioiden hoitelu n�iden kautta
	bool fUseExtraShortRangeDataTimeBag;
	NFmiTimeBag itsExtraShortRangeDataTimeBag;
	NFmiPoint itsExtraShortRangeTimeRange;

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

