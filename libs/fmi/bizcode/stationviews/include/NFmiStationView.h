//© Ilmatieteenlaitos/software by Marko
//  Original 21.09.1998
//
//
//-------------------------------------------------------------------- NFmiStationView.h

#pragma once

#include "NFmiParameterName.h"
#include "NFmiColor.h"
#include "NFmiAreaView.h"
#include "NFmiDataMatrix.h"
#include "NFmiLocation.h"
#include "NFmiDataIdent.h"
#include "NFmiFastInfoUtils.h"
#include <unordered_map>

//_________________________________________________________ NFmiStationView

class NFmiToolBox;
class NFmiArea;
class NFmiFastQueryInfo;
class NFmiGrid;
class NFmiGriddingHelperInterface;
class NFmiHelpDataInfo;
class NFmiGriddingProperties;
class NFmiExtraMacroParamData;

class CRect;
class CDC;

#include <fstream>

template<typename T>
static void StoreMatrix(NFmiDataMatrix<T> &theMatrix, const std::string &theFileName)
{
	std::ofstream out(theFileName.c_str());
	if(out)
	{
		out << theMatrix;
	}
}

class NFmiStationView : public NFmiAreaView
{

public:
   typedef std::unordered_map<unsigned long, unsigned long> StationIdSeekContainer;
   typedef std::unordered_map<FmiParameterName, unsigned long> ParamIdSeekContainer;

   inline FmiParameterName ParameterName (void){ return itsParamId; } ;
   bool GetLocation (const NFmiPoint & thePoint, NFmiLocation & theLocation);
   NFmiStationView(int theMapViewDescTopIndex, boost::shared_ptr<NFmiArea> &theArea
				   ,NFmiToolBox * theToolBox
				   ,NFmiDrawingEnvironment * theDrawingEnvi
				   ,boost::shared_ptr<NFmiDrawParam> &theDrawParam
				   ,FmiParameterName theParamId
				   ,NFmiPoint theOffSet
				   ,NFmiPoint theSize
                   ,int viewGridRowNumber
                   ,int viewGridColumnNumber);
   virtual  ~NFmiStationView (void);
   void Draw (NFmiToolBox * theGTB);
   bool DrawAllSelectedStationsWithInvertStationRect(unsigned long theMaskType);
   bool LeftButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
   bool RightButtonUp (const NFmiPoint & thePlace, unsigned long theKey);
   bool IsActiveParam(void);
   bool IsEditedDataParamView(void);
   void DrawControlPointData(void);
   void SelectLocations(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiPoint& theLatLon
									 ,int theSelectionCombineFunction
									 ,unsigned long theMask
									 ,bool fMakeMTAModeAdd
									 ,bool fDoOnlyMTAModeAdd = false);
   std::string ComposeToolTipText(const NFmiPoint& theRelativePoint);
   NFmiPoint LatLonToViewPoint(const NFmiPoint& theLatLon);
   NFmiPoint ViewPointToLatLon(const NFmiPoint& theViewPoint);
   NFmiDataMatrix<float>& SpecialMatrixData(void) {return itsSpecialMatrixData;}
   void SpecialMatrixData(const NFmiDataMatrix<float> &theMatrix) {itsSpecialMatrixData = theMatrix;}
   // t‰m‰ on asemadatan griddaus funktio, jota voidaan k‰ytt‰‰ nyt staattisena funktiona
   static void GridStationData(NFmiGriddingHelperInterface *theGriddingHelper, const boost::shared_ptr<NFmiArea> &theArea, boost::shared_ptr<NFmiDrawParam> &theDrawParam, NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime, const NFmiGriddingProperties &griddingProperties);
   static long GetTimeInterpolationRangeInMinutes(const NFmiHelpDataInfo *theHelpDataInfo);
   static bool AllowNearestTimeInterpolation(long theTimeInterpolationRangeInMinutes);

	template<typename T>
	static void GetMinAndMaxValues(const NFmiDataMatrix<T> &theMatrix, T &theMin, T &theMax)
	{
		int nx = static_cast<int>(theMatrix.NX());
		int ny = static_cast<int>(theMatrix.NY());
		float tmp = 0;
		for(int j=0; j<ny; j++)
			for(int i=0; i<nx; i++)
			{
				tmp = theMatrix[i][j];
				if(tmp == kFloatMissing)
					continue;
				if(tmp > theMax)
					theMax = tmp;
				if(tmp < theMin)
					theMin = tmp;
			}
	}

protected:
   bool IsSpecialMatrixDataDraw(void) const;
   std::string GetToolTipValueStr(float theValue, boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   float GetSynopValueFromQ2Archive(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
   bool GetQ3ScriptData(NFmiDataMatrix<float> &theValues, NFmiGrid &theUsedGrid, const std::string &theUsedBaseUrlStr);
   bool GetArchiveDataFromQ3Server(NFmiDataMatrix<float> &theValues, NFmiGrid &theUsedGridOut, bool doToolTipCalculation);
   bool IsQ2ServerUsed(void);
   bool IsThisTimeExtrapolated(const NFmiMetTime &theMapTime, NFmiFastQueryInfo & theObsInfo);
   void DrawObsComparison(void);
   virtual std::string GetCompareObservationToolTipString(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
   virtual void SetupUsedDrawParam(void);
   virtual bool PrepareForStationDraw(void);
   bool SelectControlPointLocation(boost::shared_ptr<NFmiFastQueryInfo> &theInfo
										 ,int theSelectionCombineFunction
										 ,unsigned long theMask);
   NFmiRect CalcInvertStationRectSize(double theMinXSize, double theMinYSize, double theMaxXSize, double theMaxYSize, double sizeFactor);
   virtual void ModifyTextEnvironment(void);
   virtual NFmiPoint CalcFontSize(int theMinSize, int theMaxSize, bool fPrinting);
   const NFmiRect & GeneralStationRect (void);
   void DrawAllAccessoryStationData(void);
   void CalculateGeneralStationRect(void);
   virtual void DrawData (void);
   virtual void	DrawText2(void);
   virtual NFmiString GetPrintedText(float theValue);
   virtual void ModifyTextColor(float theValue);
   NFmiRect CurrentStationRect (double theSizeFactor);
   NFmiRect CurrentStationRect (void);
   virtual NFmiRect CurrentDataRect (void);
   float ToolTipValue(const NFmiPoint& theRelativePoint, boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
   virtual float InterpolatedToolTipValue(const NFmiMetTime &theUsedTime, const NFmiPoint& theLatlon, boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
   virtual std::string Value2ToolTipString(float theValue, int theDigitCount, FmiInterpolationMethod theInterpolationMethod, FmiParamType theParamType);
   void DrawWithIsolineView(const NFmiDataMatrix<float> &theMatrix, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   bool CanToolmasterBeUsed(void);
   void DrawMouseSelectionMarker(const NFmiPoint &theLatlon, bool fDrawBiggerMarker, CRect &theBiggerBaseMfcRect, CRect &theSmallerBaseMfcRect, CDC *theUsedDC);
   double MaximumFontSizeFactor(); // t‰m‰ avulla skaalataan maksimi fontti kokoa
   NFmiRect CalcSymbolRelativeRect(const NFmiPoint &theLatlon, double theSymbolSizeInMM);
   void AddLatestObsInfoToString(std::string &tooltipString);

// ********** NƒMƒ ovat osa koordinaatiokartta lasku virityksi‰ **************
   virtual float ViewFloatValue(void); // t‰m‰ hakee n‰ytett‰v‰n datan riippuen asetuksista
   virtual void SetMapViewSettings(boost::shared_ptr<NFmiFastQueryInfo> &theUsedInfo); // tarvittavat jutut optimointia varten
   boost::shared_ptr<NFmiFastQueryInfo> itsOriginalDataInfo; // ei omista, optimointia erotus piirtoon
// ********** NƒMƒ ovat osa koordinaatiokartta lasku virityksi‰ **************

   NFmiPoint itsObjectOffSet;
   NFmiPoint itsObjectSize;
   bool fDrawText;
   NFmiPoint itsFontSize; // lasketaan vain kerran piiron yhteydess‰!

protected:
   boost::shared_ptr<NFmiFastQueryInfo> GetNearestQ2SynopStation(const NFmiLocation &theWantedLocation);
   bool UseQ2ForSynopData(boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   bool GetQ2SynopData(unsigned long theStationId = 0, checkedVector<FmiParameterName> theWantedParamVector = checkedVector<FmiParameterName>());
   void MakeDrawedInfoVector(void);
   void MakeDrawedInfoVector(checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfoVector, boost::shared_ptr<NFmiDrawParam> &theDrawParam);
   void CalcMacroParamMatrix(NFmiDataMatrix<float> &theValues, NFmiGrid *theUsedGridOut);
   float CalcMacroParamTooltipValue(NFmiExtraMacroParamData &extraMacroParamData);
   virtual bool CalcViewFloatValueMatrix(NFmiDataMatrix<float> &theValues, int x1, int y1, int x2, int y2);
   void GridStationDataToMatrix(NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime);
   void GridStationDataFromQ2(NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime);
   NFmiColor GetColoredNumberColor(float theValue);
   void DrawInvertStationRect(NFmiRect &theRect);
   void DrawStationRect(void);
   void DrawStation(NFmiDrawingEnvironment &theStationPointEnvi);
   void SetStationPointDrawingEnvi(NFmiDrawingEnvironment &envi);
   virtual void DrawSymbols(void);
   NFmiPoint CalcUsedSpaceOutFactors(int theSpaceOutFactor);
   NFmiPoint CalcSymbolDrawedMacroParamSpaceOutGridSize(int theSpaceOutFactor, const NFmiDataMatrix<float> &probingValues);
   bool IsGridDataDrawnWithSpaceOutSymbols();
   virtual int GetApproxmationOfDataTextLength(std::vector<float> *sampleValues = nullptr);
   int CalcApproxmationOfDataTextLength(const std::vector<float> &sampleValues);
   std::vector<float> GetSampleDataForDataTextLengthApproxmation();
   virtual NFmiPoint GetSpaceOutFontFactor(void);
   NFmiPoint CurrentStationPosition (void);
   const NFmiPoint CurrentLatLon(void);
   const NFmiPoint CurrentLatLon(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
   bool IsStationDataMacroParam(void);
   NFmiColor GetSymbolColor(float theValue);
   virtual NFmiColor GetBasicParamRelatedSymbolColor(float theValue);
   std::string GetLocationTooltipString();
   bool IsAccessoryStationDataDrawn();
   boost::shared_ptr<NFmiFastQueryInfo> CreatePossibleSpaceOutMacroParamData();
   bool IsParamDrawn();
   bool IsSpaceOutDrawingUsed();
   void DoSpaceOutSymbolDraw(NFmiDrawingEnvironment &theStationPointEnvi);
   void DoNormalSymbolDraw(NFmiDrawingEnvironment &theStationPointEnvi);
   void DoSparseDataSymbolDraw(NFmiDrawingEnvironment &theStationPointEnvi);
   NFmiHelpDataInfo* GetHelpDataInfo(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
   void FillDataMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, const NFmiMetTime &theTime, bool fUseCropping, int x1, int y1, int x2, int y2);
   float CalcTimeInterpolatedValue(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &theTime);
   std::string GetPossibleMacroParamSymbolText(float value, const std::string &possibleSymbolTooltipFile);
   float GetMacroParamTooltipValueFromCache(const NFmiExtraMacroParamData& extraMacroParamData);
   std::string MakeMacroParamTotalTooltipString(boost::shared_ptr<NFmiFastQueryInfo> &usedInfo, const std::string &paramName);
   void SetupPossibleWindMetaParamData();
   bool GetDataFromLocalInfo() const;
   float GetSynopDataValueFromq2();
   float GetMacroParamSpecialCalculationsValue();
   float GetDifferenceToOriginalDataValue();
   float GetValueFromLocalInfo();
   bool GetCurrentDataMatrixFromQ2Server(NFmiDataMatrix<float> &theValues, NFmiGrid &usedGrid);
   bool IsStationDataGridded();
   void CalculateGriddedStationData(NFmiDataMatrix<float> &theValues, NFmiGrid &usedGrid);
   void CalculateDifferenceToOriginalDataMatrix(NFmiDataMatrix<float> &theValues, int x1, int y1, int x2, int y2, bool useCropping);
   void FinalFillDataMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, const NFmiMetTime &usedTime, bool useCropping, int x1, int y1, int x2, int y2);
   void FinalFillWindMetaDataMatrix(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, NFmiDataMatrix<float> &theValues, const NFmiMetTime &usedTime, bool useCropping, int x1, int y1, int x2, int y2, unsigned long wantedParamId);
   bool DataIsDrawable(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const NFmiMetTime &usedTime);

   NFmiRect itsGeneralStationRect;
   FmiParameterName itsParamId;
   checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > itsInfoVector; // t‰m‰ info vektori k‰yd‰‰n l‰pi kun piirret‰‰n dataa (aluksi useita infoja vain synop-data tapauksessa)
   checkedVector<boost::shared_ptr<NFmiFastQueryInfo> >::iterator itsInfoVectorIter;
   NFmiLocation itsNearestTooltipLocation; // multi synop infojen takia pient‰ virityst‰
   boost::shared_ptr<NFmiDrawParam> itsBackupDrawParamForDifferenceDrawing;  // laitetaan t‰h‰n talteen alkuper‰iset asetukset, jos piirto tapahtuu erotuksena, voidaan palauttaa alkuper‰inen t‰st‰
   bool fDoDifferenceDrawSwitch; // t‰h‰n talletetaan tieto onko k‰ytetty drawParam palautettava piirron j‰lkeen 

   bool fDoTimeInterpolation; // jos datalle voi tehd‰ aikainterpolaation piirrett‰ess‰
							// kartalle esim. symboleja, on t‰m‰ true (tarvitaan tieto, ettei vahingossa piirret‰ oikeasti puuttuvaa dataa)

   bool fDoShipDataLocations; // normaalisti asemadatassa paikka pyydet‰‰n suoraan datan asema tiedoista. Mutta
							  // esim. SHIP-havaintojen yhteydess‰ pit‰‰ sijainti katsoa lat-lon parametreist‰
							  // kulloisellakin ajan hetkell‰. T‰t‰  varten tehtiin metodi CurrentLatLon, joka osaa palauttaa
							  // datan oikean paikan myˆs ns. laiva tms havaintojen yhteydess‰.
   NFmiPoint itsCurrentShipLatlon;

   // Kun erikois tilanne esim. laskemacroParam, mutta piirr‰ se wind-barbilla
   // pit‰‰ k‰ytt‰‰ matriisia ja muita dataosia laskuissa.
   // ************************************************************************
   bool fUseMacroParamSpecialCalculations;
   NFmiDataMatrix<float> itsMacroParamSpecialCalculationsValues;
   // ************************************************************************

   // T‰m‰ on tehty siksi ett‰ kun erilaisia symboli/teksti datoja piirret‰‰n
   // eri karttan‰yttˆluokissa (paitsi NFmiIsolineView-luokka on poikkeus) voi olla harvennus 
   // tms. juttu p‰‰ll‰, joten kun n‰m‰ on kerran laskettu ja pyydet‰‰n arvoja halutulle pisteelle,
   // interpoloidaan arvot t‰st‰ matriisista.
   NFmiDataMatrix<float> itsQ2ServerDataValues;

   // Kun ollaan ollaan tarpeeksi menneisyydess‰ ajassa, jolle ei 
   // normaaleista synop datoista lˆydy arvoja, haetaan ne q2serverilt‰ sitten 
   // jos se on sallittu.
   // ************************************************************************
   bool fGetSynopDataFromQ2;
   NFmiDataMatrix<float> itsSynopDataValuesFromQ2;
   StationIdSeekContainer itsSynopDataFromQ2StationIndexies;
   ParamIdSeekContainer itsSynopPlotParamIndexies; // t‰m‰n avulla etsit‰‰n halutun parametrin arvo kun tehd‰‰n synop-plottausta q2-datoilla
   // ************************************************************************

   bool fGetCurrentDataFromQ2Server; // kun dataa piirret‰‰n, pit‰‰ olla jokin dataInfo k‰ytˆss‰. Jos haetaan dataa, joka on tarpeeksi kaukana menneisyydess‰, 
									// ettei sit‰ saada l‰hi-cachesta, pit‰‰ data yritt‰‰ hakea q2serverilt‰.
									// T‰m‰ arvo asetetaan NFmiStationView::MakeDrawedInfoVector -metodissa kohdalleen.

   float itsToolTipDiffValue1;
   float itsToolTipDiffValue2;

   // T‰h‰n asetetaann erikois data, joka halutaan piirt‰‰ esim. isoviivoina tai contoureina NFmiIsolineView-luokalla
   NFmiDataMatrix<float> itsSpecialMatrixData;

   // MacroParam laskuissa saattaa olla k‰ytˆss‰ calculationpoint:eja. 
   // Ne piirret‰‰n tekstin‰ ja niiden piirtoa ei saa harventaa spaceoutFactorilla.
   bool fUseCalculationPoints; 
   // MacroParam laskuja halutaan optimoida, jos ne piirret‰‰n harvennetussa symboli muodossa.
   // Jos t‰m‰ tapaus on piirrossa, ei saa en‰‰ alkaa laskemaan, tarvitseeko piirtoa harventaa edelleen.
   bool fUseAlReadySpacedOutData;

   // Rajoitettuihin aikainterpolaatioihin liittyvi‰ muuttujia
   long itsTimeInterpolationRangeInMinutes;
   bool fAllowNearestTimeInterpolation;
   NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage;
};

