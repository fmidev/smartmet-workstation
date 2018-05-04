//**********************************************************
// C++ Class Name : NFmiProjectionCurvatureInfo 
// ---------------------------------------------------------
// Filetype: (HEADER)
// Filepath: G:/siirto/marko/maskeja/generated/NFmiProjectionCurvatureInfo.h 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : duplicate2 newbase kes‰ 2001 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : loggeri 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Thur - Oct 11, 2001 
// 
//  Change Log     : 
// 
//**********************************************************

#pragma once

#include <string>
#include <list>
#include <memory>
#include "NFmiPoint.h"
#include "NFmiValueLineList.h"

class NFmiArea;
class NFmiText;
class NFmiDrawingEnvironment;

class NFmiProjectionCurvatureInfo 
{
public:
	typedef enum
	{
		kNoDraw			= 0,
		kOverMap		= 1,
		kOverEverything	= 2
	} DrawingMode; // Miten piirret‰‰n projektio viivat kartalle

	typedef enum
	{
		kRude			= 0,
		kNormal			= 1,
		kSmooth			= 2,
		kCustomSmooth	= 3
	} LineSmoothness; // Mill‰ tarkkuudella viivoja piirret‰‰n (hienoja k‰yri‰ monesta viivasta vai karkeita ja kulmikkaita)

	typedef enum
	{
		kScarce			= 0,
		kModerate		= 1,
		kDense			= 2,
		kCustomDensity	= 3
	} DensityMode; // Piirret‰‰nkˆ viivoja kuinka tihe‰sti 1 vai 5 asteen v‰lein (automaattisesti kuitenkin)

public:
	NFmiProjectionCurvatureInfo(void);
	NFmiProjectionCurvatureInfo(const NFmiProjectionCurvatureInfo &theInfo);
	~NFmiProjectionCurvatureInfo(void);
	NFmiProjectionCurvatureInfo& operator=(const NFmiProjectionCurvatureInfo &theInfo);

	void InitFromSettings(const std::string &theBaseNameSpace);
	void StoreToSettings(void);
	bool Init(const std::string& theFileName); // tarvitaan viewMacrojen talletuksissa
	bool Store(const std::string& theFileName); // tarvitaan viewMacrojen talletuksissa
	bool CalcProjectionLinesAndLabels(const NFmiArea* theArea, const NFmiPoint& theFontSize, double theUsedFontSizeInRelativeUnit);
	NFmiValueLineList& GetProjectionLines(void){return itsCalculatedProjectionLines;};
    std::list<std::shared_ptr<NFmiText>>& GetProjectionLineLabels(void){ return itsCalculatedProjectionLineLabels; };

	void InitFileName(const std::string& value) {itsInitFileName = value;}
	const std::string& InitFileName(void) const {return itsInitFileName;}
	void SetDrawingMode(DrawingMode value) {itsDrawingMode = value;}
	DrawingMode GetDrawingMode(void) const {return itsDrawingMode;}
	void SetLineSmoothness(LineSmoothness value) {itsLineSmoothness = value;}
	LineSmoothness GetLineSmoothness(void) const {return itsLineSmoothness;}
	void LineDensityModeLon(DensityMode value) {itsLineDensityModeLon = value;}
	DensityMode LineDensityModeLon(void) const {return itsLineDensityModeLon;}
	void LineDensityModeLat(DensityMode value) {itsLineDensityModeLat = value;}
	DensityMode LineDensityModeLat(void) const {return itsLineDensityModeLat;}
	void DrawPrimaryLines(bool value) {fDrawPrimaryLines = value;}
	bool DrawPrimaryLines(void) const {return fDrawPrimaryLines;}
	void PrimaryLineEnvi(const NFmiDrawingEnvironment& value);
	NFmiDrawingEnvironment* PrimaryLineEnvi(void) const {return itsPrimaryLineEnvi;}
	void PrimaryLineLabelEnvi(const NFmiDrawingEnvironment& value);
	NFmiDrawingEnvironment* PrimaryLineLabelEnvi(void) const {return itsPrimaryLineLabelEnvi;}
	void UsePrimaryLineLabel(bool value) {fUsePrimaryLineLabel = value;}
	bool UsePrimaryLineLabel(void) const {return fUsePrimaryLineLabel;}
	void PrimaryLineLabelPosition(const NFmiPoint& value) {itsPrimaryLineLabelPosition = value;}
	const NFmiPoint& PrimaryLineLabelPosition(void) const {return itsPrimaryLineLabelPosition;}
	void DrawSecondaryLines(bool value) {fDrawSecondaryLines = value;}
	bool DrawSecondaryLines(void) const {return fDrawSecondaryLines;}
	void SecondaryLineEnvi(const NFmiDrawingEnvironment& value);
	const NFmiDrawingEnvironment* SecondaryLineEnvi(void) const {return itsSecondaryLineEnvi;}
	void SecondaryLineLabelEnvi(const NFmiDrawingEnvironment& value);
	NFmiDrawingEnvironment* SecondaryLineLabelEnvi(void) const {return itsSecondaryLineLabelEnvi;}
	void UseSecondaryLineLabel(bool value) {fUseSecondaryLineLabel = value;}
	bool UseSecondaryLineLabel(void) const {return fUseSecondaryLineLabel;}
	void SecondaryLineLabelPosition(const NFmiPoint& value) {itsSecondaryLineLabelPosition = value;}
	const NFmiPoint& SecondaryLineLabelPosition(void) const {return itsSecondaryLineLabelPosition;}
	const std::string& BaseNameSpaceStr(void) const {return itsBaseNameSpace;}
	void BaseNameSpaceStr(const std::string &newValue) {itsBaseNameSpace = newValue;}

	void Write(std::ostream& os) const; // tarvitaan viewMacrojen talletuksissa
	void Read(std::istream& is); // tarvitaan viewMacrojen talletuksissa
private:
    void CreateLineLabel2List(double theLabelValue, FmiDirection theDirection, const NFmiPoint& theEdgePoint, std::list<std::shared_ptr<NFmiText>> &theLineLabels, bool fLatitudeLabelUsed);
    void MakeLatitudeLines(const NFmiArea* theArea, double theStartLon, double theEndLon, double theStartLat, double theEndLat, double theDegreeAdvance, double theRelLineAdvance, NFmiValueLineList &theCalculatedLines, std::list<std::shared_ptr<NFmiText>> &theLineLabels);
    void MakeLongitudeLines(const NFmiArea* theArea, double theStartLon, double theEndLon, double theStartLat, double theEndLat, double theDegreeAdvance, double theRelLineAdvance, NFmiValueLineList &theCalculatedLines, std::list<std::shared_ptr<NFmiText>> &theLineLabels);
	double GetLineSmoothnessFactor(LineSmoothness theLineSmoothness);
	void GetLineDensityLimits(DensityMode theLineDensityMode, int* theLowerLimit, int* theUpperLimit);
	void GetExtremeLatLonValues(const NFmiArea* theArea, double& theMinLon, double& theMaxLon, double& theMinLat, double& theMaxLat);
	void CalcLatitudeAdvances(double theMinLat, double theMaxLat, double* theLatitudeLineAdvance, double* theLatitudeAlongLineAdvance);
	void CalcLongitudeAdvances(double theMinLon, double theMaxLon, double* theLongitudeLineAdvance, double* theLongitudeAlongLineAdvance);
	void Destroy(void);
	void ClearLineData(void);
    bool GetPrimaryLines(const NFmiArea* theArea, NFmiValueLineList &theCalculatedLines, std::list<std::shared_ptr<NFmiText>> &theLineLabels);

	std::string itsInitFileName; // mist‰ tiedostosta on olio initialisoitu
	
	DrawingMode itsDrawingMode; // Miten piirret‰‰n projektio viivat kartalle:
	LineSmoothness itsLineSmoothness; // Mill‰ tarkkuudella viivoja piirret‰‰n (hienoja k‰yri‰ monesta viivasta vai karkeita ja kulmikkaita):
	DensityMode itsLineDensityModeLon; // Piirret‰‰nkˆ viivoja kuinka tihe‰sti 1 vai 5 asteen v‰lein (automaattisesti kuitenkin):
	DensityMode itsLineDensityModeLat; // Piirret‰‰nkˆ viivoja kuinka tihe‰sti 1 vai 5 asteen v‰lein (automaattisesti kuitenkin):
	double itsUsedFontSizeInRelativeUnit; // t‰ll‰ mitalla siirret‰‰n label paikkoja

	// Projektio viivat jaetaan kahteen luokkaan: primary ja secondary.

	// primary asetukset:
	bool fDrawPrimaryLines;
	NFmiDrawingEnvironment* itsPrimaryLineEnvi;
	int itsPrimaryLineStyle; // yhten‰inen vai katkoviiva 0=yht. ja 1 = katko (ei toolboxissa tietoa)
	NFmiDrawingEnvironment* itsPrimaryLineLabelEnvi;
	bool fUsePrimaryLineLabel;
	NFmiPoint itsPrimaryLineLabelPosition;

	// primary asetukset:
	bool fDrawSecondaryLines;
	NFmiDrawingEnvironment* itsSecondaryLineEnvi;
	int itsSecondaryLineStyle; // yhten‰inen vai katkoviiva 0=yht. ja 1 = katko (ei toolboxissa tietoa)
	NFmiDrawingEnvironment* itsSecondaryLineLabelEnvi;
	bool fUseSecondaryLineLabel;
	NFmiPoint itsSecondaryLineLabelPosition;

	NFmiValueLineList itsCalculatedProjectionLines; // lasketaan viivat t‰nne talteen // omistaa, tuhoaa!!!
	std::list<std::shared_ptr<NFmiText>> itsCalculatedProjectionLineLabels; // lasketaan viivoihin liittyv‰t labelit tanne talteen  // omistaa, tuhoaa!!!

	std::string itsBaseNameSpace;
};
inline std::ostream& operator<<(std::ostream& os, const NFmiProjectionCurvatureInfo& item){item.Write(os); return os;}
inline std::istream& operator>>(std::istream& is, NFmiProjectionCurvatureInfo& item){item.Read(is); return is;}

