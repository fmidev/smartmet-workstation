//**********************************************************
// C++ Class Name : NFmiProjectionCurvatureInfo 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: G:/siirto/marko/maskeja/generated/NFmiProjectionCurvatureInfo.cpp 
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
#include "NFmiProjectionCurvatureInfo.h"
#include "NFmiDrawingEnvironment.h"
#include "NFmiColor.h"
#include "NFmiArea.h"
#include "NFmiValueString.h"
#include "NFmiText.h"
#include "NFmiPolyline.h"
#include "NFmiVoidPtrList.h"
#include "NFmiSettings.h"
#include "SettingsFunctions.h"
#include <fstream>


using namespace std;

//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiProjectionCurvatureInfo::NFmiProjectionCurvatureInfo(void)
:itsInitFileName()
,itsDrawingMode(kNoDraw)
,itsLineSmoothness(kNormal)
,itsLineDensityModeLon(kModerate)
,itsLineDensityModeLat(kModerate)
,itsUsedFontSizeInRelativeUnit(0.01)
,fDrawPrimaryLines(true)
,itsPrimaryLineEnvi(new NFmiDrawingEnvironment())
,itsPrimaryLineStyle(0)
,itsPrimaryLineLabelEnvi(new NFmiDrawingEnvironment())
,fUsePrimaryLineLabel(false)
,itsPrimaryLineLabelPosition()
,fDrawSecondaryLines(false)
,itsSecondaryLineEnvi(new NFmiDrawingEnvironment())
,itsSecondaryLineStyle(0)
,itsSecondaryLineLabelEnvi(new NFmiDrawingEnvironment())
,fUseSecondaryLineLabel(false)
,itsSecondaryLineLabelPosition()
,itsCalculatedProjectionLines()
,itsCalculatedProjectionLineLabels()
,itsBaseNameSpace()
{
}

NFmiProjectionCurvatureInfo::NFmiProjectionCurvatureInfo(const NFmiProjectionCurvatureInfo &theInfo)
:itsInitFileName(theInfo.itsInitFileName)
,itsDrawingMode(theInfo.itsDrawingMode)
,itsLineSmoothness(theInfo.itsLineSmoothness)
,itsLineDensityModeLon(theInfo.itsLineDensityModeLon)
,itsLineDensityModeLat(theInfo.itsLineDensityModeLat)
,itsUsedFontSizeInRelativeUnit(theInfo.itsUsedFontSizeInRelativeUnit)
,fDrawPrimaryLines(theInfo.fDrawPrimaryLines)
,itsPrimaryLineEnvi(theInfo.itsPrimaryLineEnvi ? new NFmiDrawingEnvironment(*theInfo.itsPrimaryLineEnvi) : 0)
,itsPrimaryLineStyle(theInfo.itsPrimaryLineStyle)
,itsPrimaryLineLabelEnvi(theInfo.itsPrimaryLineLabelEnvi ? new NFmiDrawingEnvironment(*theInfo.itsPrimaryLineLabelEnvi) : 0)
,fUsePrimaryLineLabel(theInfo.fUsePrimaryLineLabel)
,itsPrimaryLineLabelPosition(theInfo.itsPrimaryLineLabelPosition)
,fDrawSecondaryLines(theInfo.fDrawSecondaryLines)
,itsSecondaryLineEnvi(theInfo.itsSecondaryLineEnvi ? new NFmiDrawingEnvironment(*theInfo.itsSecondaryLineEnvi) : 0)
,itsSecondaryLineStyle(theInfo.itsSecondaryLineStyle)
,itsSecondaryLineLabelEnvi(theInfo.itsSecondaryLineLabelEnvi ? new NFmiDrawingEnvironment(*theInfo.itsSecondaryLineLabelEnvi) : 0)
,fUseSecondaryLineLabel(theInfo.fUseSecondaryLineLabel)
,itsSecondaryLineLabelPosition(theInfo.itsSecondaryLineLabelPosition)
,itsCalculatedProjectionLines()
,itsCalculatedProjectionLineLabels()
,itsBaseNameSpace()
{
}

//--------------------------------------------------------
// ®NFmiProjectionCurvatureInfo 
//--------------------------------------------------------
NFmiProjectionCurvatureInfo::~NFmiProjectionCurvatureInfo(void)
{
	Destroy();
}

NFmiProjectionCurvatureInfo& NFmiProjectionCurvatureInfo::operator=(const NFmiProjectionCurvatureInfo &theInfo)
{
	if(this != &theInfo)
	{
		Destroy();
		itsInitFileName = theInfo.itsInitFileName;
		itsDrawingMode = theInfo.itsDrawingMode;
		itsLineSmoothness = theInfo.itsLineSmoothness;
		itsLineDensityModeLon = theInfo.itsLineDensityModeLon;
		itsLineDensityModeLat = theInfo.itsLineDensityModeLat;
		fDrawPrimaryLines = theInfo.fDrawPrimaryLines;
		itsPrimaryLineEnvi = theInfo.itsPrimaryLineEnvi ? new NFmiDrawingEnvironment(*theInfo.itsPrimaryLineEnvi) : 0;
		itsPrimaryLineStyle = theInfo.itsPrimaryLineStyle;
		itsPrimaryLineLabelEnvi = theInfo.itsPrimaryLineLabelEnvi ? new NFmiDrawingEnvironment(*theInfo.itsPrimaryLineLabelEnvi) : 0;
		fUsePrimaryLineLabel = theInfo.fUsePrimaryLineLabel;
		itsPrimaryLineLabelPosition = theInfo.itsPrimaryLineLabelPosition;
		fDrawSecondaryLines = theInfo.fDrawSecondaryLines;
		itsSecondaryLineEnvi = theInfo.itsSecondaryLineEnvi ? new NFmiDrawingEnvironment(*theInfo.itsSecondaryLineEnvi) : 0;
		itsSecondaryLineStyle = theInfo.itsSecondaryLineStyle;
		itsSecondaryLineLabelEnvi = theInfo.itsSecondaryLineLabelEnvi ? new NFmiDrawingEnvironment(*theInfo.itsSecondaryLineLabelEnvi) : 0;
		fUseSecondaryLineLabel = theInfo.fUseSecondaryLineLabel;
		itsSecondaryLineLabelPosition = theInfo.itsSecondaryLineLabelPosition;
		// HUOM! ei saa kopioida viivoja eik‰ labeleita, pit‰‰ olla tyhj‰ lista sijoituksen j‰lkeen, ett‰ ne lasketaan uudestaan
        itsCalculatedProjectionLines.Clear(); 
        itsCalculatedProjectionLineLabels.clear();
		// itsBaseNameSpace pit‰‰ tarkistaa, jos kopioitavan objektin data on tyhj‰, s‰ilytet‰‰n nykyarvo (n‰yttˆmakrosta ladattuna se on tyhj‰)
		if(!theInfo.itsBaseNameSpace.empty())
			itsBaseNameSpace = theInfo.itsBaseNameSpace;
	}
	return *this;
}

void NFmiProjectionCurvatureInfo::Destroy(void)
{
	delete itsPrimaryLineEnvi;
	delete itsPrimaryLineLabelEnvi;
	delete itsSecondaryLineEnvi;
	delete itsSecondaryLineLabelEnvi;
    ClearLineData();
}
void NFmiProjectionCurvatureInfo::ClearLineData(void)
{
	itsCalculatedProjectionLines.Clear(true);
	itsCalculatedProjectionLineLabels.clear();
}

//--------------------------------------------------------
// Init 
//--------------------------------------------------------
void NFmiProjectionCurvatureInfo::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	itsDrawingMode = static_cast<DrawingMode>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::DrawingMode")));
	itsLineSmoothness = static_cast<LineSmoothness>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::LineSmoothness")));
	itsLineDensityModeLon = static_cast<DensityMode>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::LineDensityModeLon")));
	itsLineDensityModeLat = static_cast<DensityMode>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::LineDensityModeLat")));

	fDrawPrimaryLines = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::DrawPrimaryLines"));
	itsPrimaryLineEnvi->SetPenSize(SettingsFunctions::GetPointFromSettings(itsBaseNameSpace + "::PrimaryPenSize"));
	itsPrimaryLineEnvi->SetFrameColor(SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::PrimaryPenColor"));
	itsPrimaryLineStyle = NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::PrimaryLineStyle"));
	itsPrimaryLineLabelEnvi->SetFontSize(SettingsFunctions::GetPointFromSettings(itsBaseNameSpace + "::PrimaryLineLabelFontSize"));
	itsPrimaryLineLabelEnvi->SetFrameColor(SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::PrimaryLineLabelColor"));
	itsPrimaryLineLabelEnvi->SetFontType(static_cast<FmiFontType>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::PrimaryLineLabelFontType"))));
	fUsePrimaryLineLabel = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UsePrimaryLineLabel"));
	itsPrimaryLineLabelPosition = SettingsFunctions::GetPointFromSettings(itsBaseNameSpace + "::PrimaryLineLabelPosition");

	fDrawSecondaryLines = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::DrawSecondaryLines"));
	itsSecondaryLineEnvi->SetPenSize(SettingsFunctions::GetPointFromSettings(itsBaseNameSpace + "::SecondaryPenSize"));
	itsSecondaryLineEnvi->SetFrameColor(SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::SecondaryPenColor"));
	itsSecondaryLineStyle = NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::SecondaryLineStyle"));
	itsSecondaryLineLabelEnvi->SetFontSize(SettingsFunctions::GetPointFromSettings(itsBaseNameSpace + "::SecondaryLineLabelFontSize"));
	itsSecondaryLineLabelEnvi->SetFrameColor(SettingsFunctions::GetColorFromSettings(itsBaseNameSpace + "::SecondaryLineLabelColor"));
	itsSecondaryLineLabelEnvi->SetFontType(static_cast<FmiFontType>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::SecondaryLineLabelFontType"))));
	fUseSecondaryLineLabel = NFmiSettings::Require<bool>(std::string(itsBaseNameSpace + "::UseSecondaryLineLabel"));
	itsSecondaryLineLabelPosition = SettingsFunctions::GetPointFromSettings(itsBaseNameSpace + "::SecondaryLineLabelPosition");
}

//--------------------------------------------------------
// Store 
//--------------------------------------------------------
void NFmiProjectionCurvatureInfo::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DrawingMode"), NFmiStringTools::Convert(itsDrawingMode), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::LineSmoothness"), NFmiStringTools::Convert(itsLineSmoothness), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::LineDensityModeLon"), NFmiStringTools::Convert(itsLineDensityModeLon), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::LineDensityModeLat"), NFmiStringTools::Convert(itsLineDensityModeLat), true);

		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DrawPrimaryLines"), NFmiStringTools::Convert(fDrawPrimaryLines), true);
        SettingsFunctions::SetPointToSettings(itsBaseNameSpace + "::PrimaryPenSize", itsPrimaryLineEnvi->GetPenSize());
        SettingsFunctions::SetColorToSettings(itsBaseNameSpace + "::PrimaryPenColor", itsPrimaryLineEnvi->GetFrameColor());
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::PrimaryLineStyle"), NFmiStringTools::Convert(itsPrimaryLineStyle), true);
        SettingsFunctions::SetPointToSettings(itsBaseNameSpace + "::PrimaryLineLabelFontSize", itsPrimaryLineLabelEnvi->GetFontSize());
        SettingsFunctions::SetColorToSettings(itsBaseNameSpace + "::PrimaryLineLabelColor", itsPrimaryLineLabelEnvi->GetFrameColor());
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::PrimaryLineLabelFontType"), NFmiStringTools::Convert(itsPrimaryLineLabelEnvi->GetFontType()), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UsePrimaryLineLabel"), NFmiStringTools::Convert(fUsePrimaryLineLabel), true);
        SettingsFunctions::SetPointToSettings(itsBaseNameSpace + "::PrimaryLineLabelPosition", itsPrimaryLineLabelPosition);

		NFmiSettings::Set(std::string(itsBaseNameSpace + "::DrawSecondaryLines"), NFmiStringTools::Convert(fDrawSecondaryLines), true);
        SettingsFunctions::SetPointToSettings(itsBaseNameSpace + "::SecondaryPenSize", itsSecondaryLineEnvi->GetPenSize());
        SettingsFunctions::SetColorToSettings(itsBaseNameSpace + "::SecondaryPenColor", itsSecondaryLineEnvi->GetFrameColor());
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::SecondaryLineStyle"), NFmiStringTools::Convert(itsSecondaryLineStyle), true);
        SettingsFunctions::SetPointToSettings(itsBaseNameSpace + "::SecondaryLineLabelFontSize", itsSecondaryLineLabelEnvi->GetFontSize());
        SettingsFunctions::SetColorToSettings(itsBaseNameSpace + "::SecondaryLineLabelColor", itsSecondaryLineLabelEnvi->GetFrameColor());
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::SecondaryLineLabelFontType"), NFmiStringTools::Convert(itsSecondaryLineLabelEnvi->GetFontType()), true);
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseSecondaryLineLabel"), NFmiStringTools::Convert(fUseSecondaryLineLabel), true);
        SettingsFunctions::SetPointToSettings(itsBaseNameSpace + "::SecondaryLineLabelPosition", itsSecondaryLineLabelPosition);
	}
	else
		throw std::runtime_error("Error in NFmiProjectionCurvatureInfo::StoreToSettings, unable to store setting.");
}

bool NFmiProjectionCurvatureInfo::Init(const std::string& theFileName)
{
	ifstream in(theFileName.c_str(), std::ios::binary);
	if(in)
	{
		itsInitFileName = theFileName;
		in >> *this;
		in.close();
		return true;
	}
	return false;
}

bool NFmiProjectionCurvatureInfo::Store(const std::string& theFileName)
{
	ofstream out(theFileName.c_str(), std::ios::binary);
	if(out)
	{
		out << *this;
		out.close();
		return true;
	}
	return false;
}

void NFmiProjectionCurvatureInfo::Write(std::ostream& os) const
{
	os << static_cast<int>(itsDrawingMode) << endl;
	os << static_cast<int>(itsLineSmoothness) << endl;
	os << static_cast<int>(itsLineDensityModeLon) << endl;
	os << static_cast<int>(itsLineDensityModeLat) << endl;

	// Talletetaan sek‰ primary, ett‰ secondary viivojen ominaisuudet.

	// primary asetukset:
	os << endl;
	os	<< fDrawPrimaryLines << endl;
	os << const_cast<NFmiPoint&>(itsPrimaryLineEnvi->GetPenSize()) << NFmiColor(itsPrimaryLineEnvi->GetFrameColor()) << endl;
	os << itsPrimaryLineStyle << endl; 
	os << itsPrimaryLineLabelEnvi->GetFontWidth() << " " << itsPrimaryLineLabelEnvi->GetFontHeight() << " " << NFmiColor(itsPrimaryLineLabelEnvi->GetFrameColor()) << " " << static_cast<int>(itsPrimaryLineLabelEnvi->GetFontType()) << endl;
	os << fUsePrimaryLineLabel << endl;
	os << itsPrimaryLineLabelPosition << endl;

	// Secondary asetukset:
	os << endl;
	os	<< fDrawSecondaryLines << endl;
	os << const_cast<NFmiPoint&>(itsSecondaryLineEnvi->GetPenSize()) << NFmiColor(itsSecondaryLineEnvi->GetFrameColor()) << endl;
	os << itsSecondaryLineStyle << endl;
	os << itsSecondaryLineLabelEnvi->GetFontWidth() << " " << itsSecondaryLineLabelEnvi->GetFontHeight() << " " << NFmiColor(itsSecondaryLineLabelEnvi->GetFrameColor()) << " " << static_cast<int>(itsSecondaryLineLabelEnvi->GetFontType()) << endl;
	os << fUseSecondaryLineLabel << endl;
	os << itsSecondaryLineLabelPosition << endl;
}

void NFmiProjectionCurvatureInfo::Read(std::istream& is)
{
	int tmp = 0, tmp2 = 0;
	NFmiPoint tmpPoint;
	NFmiColor tmpColor;

	is >> tmp;
	itsDrawingMode = (DrawingMode)tmp;
	is >> tmp;
	itsLineSmoothness = (LineSmoothness)tmp;
	is >> tmp;
	itsLineDensityModeLon = (DensityMode)tmp;
	is >> tmp;
	itsLineDensityModeLat = (DensityMode)tmp;

	// Luetaan sek‰ primary, ett‰ secondary viivojen ominaisuudet.

	// Luetaan primary asetukset:
	is >> fDrawPrimaryLines;
	is >> tmpPoint;
	itsPrimaryLineEnvi->SetPenSize(tmpPoint);
	is >> tmpColor;
	itsPrimaryLineEnvi->SetFrameColor(tmpColor.GetRGB());
	is >> itsPrimaryLineStyle;
	is >> tmp;
	is >> tmp2;
	itsPrimaryLineLabelEnvi->SetFontSize(NFmiPoint(tmp, tmp2));
	is >> tmpColor;
	itsPrimaryLineLabelEnvi->SetFrameColor(tmpColor.GetRGB());
	is >> tmp;
	itsPrimaryLineLabelEnvi->SetFontType((FmiFontType)tmp);
	is >> fUsePrimaryLineLabel;
	is >> itsPrimaryLineLabelPosition;

	// Luetaan Secondary asetukset:
	is >> fDrawSecondaryLines;
	is >> tmpPoint;
	itsSecondaryLineEnvi->SetPenSize(tmpPoint);
	is >> tmpColor;
	itsSecondaryLineEnvi->SetFrameColor(tmpColor.GetRGB());
	is >> itsSecondaryLineStyle;
	is >> tmp;
	is >> tmp2;
	itsSecondaryLineLabelEnvi->SetFontSize(NFmiPoint(tmp, tmp2));
	is >> tmpColor;
	itsSecondaryLineLabelEnvi->SetFrameColor(tmpColor.GetRGB());
	is >> tmp;
	itsSecondaryLineLabelEnvi->SetFontType((FmiFontType)tmp);
	is >> fUseSecondaryLineLabel;
	is >> itsSecondaryLineLabelPosition;
}

//--------------------------------------------------------
// Set-metodeja, jotka piti piilottaa cpp:hen
//--------------------------------------------------------
void NFmiProjectionCurvatureInfo::PrimaryLineEnvi(const NFmiDrawingEnvironment& value) 
{
	*itsPrimaryLineEnvi = value;
}
void NFmiProjectionCurvatureInfo::PrimaryLineLabelEnvi(const NFmiDrawingEnvironment& value) 
{
	*itsPrimaryLineLabelEnvi = value;
}
void NFmiProjectionCurvatureInfo::SecondaryLineEnvi(const NFmiDrawingEnvironment& value) 
{
	*itsSecondaryLineEnvi = value;
}
void NFmiProjectionCurvatureInfo::SecondaryLineLabelEnvi(const NFmiDrawingEnvironment& value) 
{
	*itsSecondaryLineLabelEnvi = value;
}

//--------------------------------------------------------
// GetLines 
//--------------------------------------------------------
// Palauttaa yhdess‰ rys‰yksess‰ prim‰‰ri ja sekund‰‰ri viivat halutulle alueelle, k‰ytt‰en haluttuja piirtoominaisuuksia.
bool NFmiProjectionCurvatureInfo::CalcProjectionLinesAndLabels(const NFmiArea* theArea, const NFmiPoint& theFontSize, double theUsedFontSizeInRelativeUnit)
{
	bool status = false;
	itsUsedFontSizeInRelativeUnit = theUsedFontSizeInRelativeUnit;
	ClearLineData();
	if(itsDrawingMode != kNoDraw)
	{
		itsPrimaryLineLabelEnvi->SetFontSize(theFontSize);
		status = GetPrimaryLines(theArea, itsCalculatedProjectionLines, itsCalculatedProjectionLineLabels);
	}
	return status;
}

static bool gsLineIntersection(const NFmiPoint& P1, const NFmiPoint& P2, const NFmiPoint& P3, const NFmiPoint& P4, NFmiPoint* PI)
{
	double denominator = ((P4.Y() - P3.Y()) * (P2.X() - P1.X())) - ((P4.X() - P3.X()) * (P2.Y() - P1.Y()));
	if(denominator == 0.)
		return false;
	double numerator1 = ((P4.X() - P3.X()) * (P1.Y() - P3.Y())) - ((P4.Y() - P3.Y()) * (P1.X() - P3.X()));
	double numerator2 = ((P2.X() - P1.X()) * (P1.Y() - P3.Y())) - ((P2.Y() - P1.Y()) * (P1.X() - P3.X()));
	double u1 = numerator1/denominator;
	double u2 = numerator2/denominator;
	if(u1 >= 0. && u1 <= 1. && u2 >= 0. && u2 <= 1.) // viiva segmentit leikkaavat
	{
		double x = P1.X() + u1*(P2.X() - P1.X());
		double y = P1.Y() + u1*(P2.Y() - P1.Y());
		*PI = NFmiPoint(x, y);
		return true;
	}
	return false;
}

static FmiDirection gsFindIntersectionPoint(const NFmiPoint& lineP1, const NFmiPoint& lineP2, const NFmiPoint& blP, const NFmiPoint& brP, const NFmiPoint& tlP, const NFmiPoint& trP, NFmiPoint* intersectionP)
{
	if(gsLineIntersection(lineP1, lineP2, blP, brP, intersectionP))
		return kBottom;
	if(gsLineIntersection(lineP1, lineP2, blP, tlP, intersectionP))
		return kLeft;
	if(gsLineIntersection(lineP1, lineP2, tlP, trP, intersectionP))
		return kTop;
	if(gsLineIntersection(lineP1, lineP2, trP, brP, intersectionP))
		return kRight;
	return kCenter; // kCenter = ei lˆytynyt leikkausta
}

static bool IsGlobalArea(const NFmiArea* theArea)
{
	if(theArea->ClassId() == kNFmiLatLonArea)
	{
		if(theArea->PacificView())
		{
			const NFmiPoint globalBL(0, -90);
			const NFmiPoint globalTR(360, 90);
			if(theArea->BottomLeftLatLon() == globalBL && theArea->TopRightLatLon() == globalTR)
				return true;
		}
		else
		{
			const NFmiPoint globalBL(-180, -90);
			const NFmiPoint globalTR(180, 90);
			if(theArea->BottomLeftLatLon() == globalBL && theArea->TopRightLatLon() == globalTR)
				return true;
		}
	}
	return false;
}

//--------------------------------------------------------
// GetPrimaryLines 
//--------------------------------------------------------
// Laskee annetulle alueelle prim‰‰ri projektiviivat. Tulos on tallessa parametrina annetussa theCalculatedLines-oliossa. Huom! ne lis‰t‰‰n 
// listaan (muista tyhjent‰‰ v‰lill‰!).
bool NFmiProjectionCurvatureInfo::GetPrimaryLines(const NFmiArea* theArea, NFmiValueLineList &theCalculatedLines, std::list<std::shared_ptr<NFmiText>> &theLineLabels)
{

	double minLon = 0;
	double maxLon = 0;
	double minLat = 0;
	double maxLat = 0;
	GetExtremeLatLonValues(theArea, minLon, maxLon, minLat, maxLat);

	double longitudeLineAdvance = 0;
	double longitudeAlongLineAdvance = 0;
	double latitudeLineAdvance = 0;
	double latitudeAlongLineAdvance = 0;
	CalcLatitudeAdvances(minLat, maxLat, &latitudeLineAdvance, &latitudeAlongLineAdvance);
	CalcLongitudeAdvances(minLon, maxLon, &longitudeLineAdvance, &longitudeAlongLineAdvance);
	if(longitudeLineAdvance	&& longitudeAlongLineAdvance &&	latitudeLineAdvance &&	latitudeAlongLineAdvance)
	{
		// koko maailman kartan aloitus arvot eiv‰t saa menn‰ alueen ulkopuolelle, mutta kaikkien loppuarvojen pit‰‰ menn‰ hieman ulos
		bool globalMap = ::IsGlobalArea(theArea);
		double leftLon = globalMap ? minLon : (int)(minLon/longitudeLineAdvance)*longitudeLineAdvance;
		if(!globalMap && minLon < 0.)
			leftLon = round(minLon/longitudeLineAdvance)*longitudeLineAdvance - longitudeLineAdvance;
		double rightLon = (int)(maxLon/longitudeLineAdvance)*longitudeLineAdvance + 1*longitudeLineAdvance; // + 2*longitudeLineAdvance on ns. varman p‰‰lle juttu, muuten lopetuspiste jouduttaisiin iteroimaan
		if(maxLon < 0.)
			rightLon = round(maxLon/longitudeLineAdvance)*longitudeLineAdvance + 1*longitudeLineAdvance; // + 2*longitudeLineAdvance on ns. varman p‰‰lle juttu, muuten lopetuspiste jouduttaisiin iteroimaan
		double downLat = globalMap ? minLat : (int)(minLat/latitudeLineAdvance)*latitudeLineAdvance;
		if(!globalMap && minLat < 0.)
			downLat = round(minLat/latitudeLineAdvance)*latitudeLineAdvance - latitudeLineAdvance;
		double upLat = (int)(maxLat/latitudeLineAdvance)*latitudeLineAdvance + 1*latitudeLineAdvance; // + 2*longitudeLineAdvance on ns. varman p‰‰lle juttu, muuten lopetuspiste jouduttaisiin iteroimaan
		if(maxLat < 0.)
			upLat = round(maxLat/latitudeLineAdvance)*latitudeLineAdvance + 1*latitudeLineAdvance; // + 2*longitudeLineAdvance on ns. varman p‰‰lle juttu, muuten lopetuspiste jouduttaisiin iteroimaan

		MakeLongitudeLines(theArea, leftLon, rightLon, downLat, upLat, longitudeLineAdvance, latitudeAlongLineAdvance, theCalculatedLines, theLineLabels);

        // T‰m‰ try-catch -lauseke est‰‰ mystisen poikkeuksen MakeLatitudeLines -metodin kutsusta, joka esiintyy VAIN Visual C++ 2013 k‰‰nt‰j‰n x64 Release version ajossa.
        // Koodi toiminut ongelmitta ainakin VC++2003,2005,2008,2010,2012 versioilla kaikissa 32/64 ja debug/release -buildeissa.
        // Ongelma esiintyi ainakin SmartMetin normaali skandi kartan versiolla ja kartan projektio viivojen piirto keskeytyi (alun perin myˆs datan piirto kartalle keskeytyi).
        // Jos laitoin try-catch -lausekkeen MakeLatitudeLines -metodin sis‰lle (kattamaan koko metodin alusta loppuun), poikkeus lensi.
        // JOKO kyse on Visual C++ 2013 k‰‰nt‰j‰n bugista tai omassa koodissa on bugi, joka aiheuttaa poikkeuksen, kun metodia kutsutaan (en vain ymm‰rr‰ mik‰ se on).
        try
        {
            MakeLatitudeLines(theArea, leftLon, rightLon, downLat, upLat, latitudeLineAdvance, longitudeAlongLineAdvance, theCalculatedLines, theLineLabels);
        }
        catch(...)
        {
        }
	}
	return true;
}

void NFmiProjectionCurvatureInfo::MakeLongitudeLines(const NFmiArea* theArea, double theStartLon, double theEndLon, double theStartLat, double theEndLat, double theDegreeAdvance, double theRelLineAdvance, NFmiValueLineList &theCalculatedLines, std::list<std::shared_ptr<NFmiText>> &theLineLabels)
{
	// n‰ist‰ muodostetaan reuna viivat leikkausten laskuun!!!!
	NFmiPoint bl(theArea->BottomLeft());
	NFmiPoint br(theArea->BottomRight());
	NFmiPoint tr(theArea->TopRight());
	NFmiPoint tl(theArea->TopLeft());

	NFmiPolyline* polyLine = 0;
	NFmiPoint currentPoint;
	NFmiPoint lastPoint; // t‰m‰n avulla lasketaan ruudun yli menv‰t viivojen leikkaukset
	NFmiPoint intersectionPoint;
	bool globalMap = ::IsGlobalArea(theArea);
	if(globalMap == false)
		theStartLat -= theRelLineAdvance;
	for(double i = theStartLon; i<=theEndLon; i+=theDegreeAdvance) // lasketaan ensin longitude viivat
	{
		bool foundPointInside = false;
		bool outsideAfterFoundPointInside = false;
		for(double j = theStartLat; j<= theEndLat+theRelLineAdvance; j+=theRelLineAdvance)
		{
			NFmiPoint p(i, j);
			currentPoint = theArea->ToXY(p);
			if(theArea->IsInside(p))
			{
				if(!foundPointInside)
				{
					foundPointInside = true;
					polyLine = new NFmiPolyline(theArea->XYArea(), 0, itsPrimaryLineEnvi);
					// laita leikkaus piste mukaan ensimm‰iseksi
					if(j != theStartLat)
					{
						FmiDirection dir = gsFindIntersectionPoint(lastPoint, currentPoint, bl, br, tl, tr, &intersectionPoint);
						if(dir != kCenter)
						{
							polyLine->AddPoint(intersectionPoint);
							CreateLineLabel2List(i, dir, intersectionPoint, theLineLabels, false);
						}
					}
					else
						CreateLineLabel2List(i, kBottom, currentPoint, theLineLabels, false);
				}
				// pakko lis‰t‰ global-kartta erikoisuus, jos ollaan tarpeeksi l‰hell‰ maailman kartan yl‰puolella, on piste sis‰ll‰ (epsilon tarkastus), vaikka piste onkin ulkona, t‰llˆin lis‰t‰‰n labeli jo t‰ss‰
				if(globalMap && j > 90)
					CreateLineLabel2List(i, kUp, currentPoint, theLineLabels, false);
				polyLine->AddPoint(currentPoint);
			}
			else if(foundPointInside && !outsideAfterFoundPointInside) // nyt kun menn‰‰n taas ruudun ulkopuolelle, pit‰‰ viimeinen viiva clipata
			{
				outsideAfterFoundPointInside = true;
				FmiDirection dir = gsFindIntersectionPoint(lastPoint, currentPoint, bl, br, tl, tr, &intersectionPoint);
				if(dir != kCenter)
				{
					polyLine->AddPoint(intersectionPoint);
					CreateLineLabel2List(i, dir, intersectionPoint, theLineLabels, false);
				}
			}
			lastPoint = currentPoint;
		}
		if(foundPointInside)
			theCalculatedLines.PolyAdd(polyLine);
	}
}

void NFmiProjectionCurvatureInfo::MakeLatitudeLines(const NFmiArea* theArea, double theStartLon, double theEndLon, double theStartLat, double theEndLat, double theDegreeAdvance, double theRelLineAdvance, NFmiValueLineList &theCalculatedLines, std::list<std::shared_ptr<NFmiText>> &theLineLabels)
{
    // n‰ist‰ muodostetaan reuna viivat leikkausten laskuun!!!!
    NFmiPoint bl(theArea->BottomLeft());
    NFmiPoint br(theArea->BottomRight());
    NFmiPoint tr(theArea->TopRight());
    NFmiPoint tl(theArea->TopLeft());

    NFmiPolyline* polyLine = 0;
    NFmiPoint currentPoint;
    NFmiPoint lastPoint; // t‰m‰n avulla lasketaan ruudun yli menv‰t viivojen leikkaukset
    NFmiPoint intersectionPoint;
    bool globalMap = ::IsGlobalArea(theArea);
    if(globalMap == false)
        theStartLon -= theRelLineAdvance;
    for(double j = theStartLat; j <= theEndLat; j += theDegreeAdvance) // lasketaan ensin longitude viivat
    {
        bool foundPointInside = false;
        bool outsideAfterFoundPointInside = false;
        for(double i = theStartLon; i <= theEndLon + theRelLineAdvance; i += theRelLineAdvance)
        {
            NFmiPoint p(i, j);
            currentPoint = theArea->ToXY(p);
            if(theArea->IsInside(p) && currentPoint.X() < theArea->XYArea().Right())
            {
                if(!foundPointInside)
                {
                    foundPointInside = true;
                    outsideAfterFoundPointInside = false;
                    if(polyLine)
                        theCalculatedLines.PolyAdd(polyLine); // laitetaan jo syntynyt viiva talteen ja sitten luodaan uusi uutta p‰tk‰‰ varten
                    polyLine = new NFmiPolyline(theArea->XYArea(), 0, itsPrimaryLineEnvi);
                    // laita leikkaus piste mukaan ensimm‰iseksi
                    if(i != theStartLon)
                    {
                        FmiDirection dir = gsFindIntersectionPoint(lastPoint, currentPoint, bl, br, tl, tr, &intersectionPoint);
                        if(dir != kCenter)
                        {
                            polyLine->AddPoint(intersectionPoint);
                            CreateLineLabel2List(j, dir, intersectionPoint, theLineLabels, true);
                        }
                    }
                    else
                        CreateLineLabel2List(j, kLeft, currentPoint, theLineLabels, true);
                }
                polyLine->AddPoint(currentPoint);
            }
            else if(foundPointInside && !outsideAfterFoundPointInside) // nyt kun menn‰‰n taas ruudun ulkopuolelle, pit‰‰ viimeinen viiva clipata
            {
                outsideAfterFoundPointInside = true;
                foundPointInside = false;
                FmiDirection dir = gsFindIntersectionPoint(lastPoint, currentPoint, bl, br, tl, tr, &intersectionPoint);
                if(dir != kCenter)
                {
                    polyLine->AddPoint(intersectionPoint);
                    CreateLineLabel2List(j, dir, intersectionPoint, theLineLabels, true);
                }
            }
            lastPoint = currentPoint;
        }
        if(polyLine && polyLine->GetPoints().size())
        {
            theCalculatedLines.PolyAdd(polyLine);
            polyLine = 0;
        }
    }
}

void NFmiProjectionCurvatureInfo::CreateLineLabel2List(double theLabelValue, FmiDirection theDirection, const NFmiPoint& theEdgePoint, std::list<std::shared_ptr<NFmiText>> &theLineLabels, bool fLatitudeLabelUsed)
{
    double moveUnit = itsUsedFontSizeInRelativeUnit;
    // Pit‰‰ fiksata label stringiin arvo, jos kyseess‰ longitude ja ollaan v‰lill‰ 180 > x <= 360
    if(fLatitudeLabelUsed == false && NFmiArea::IsPacificLongitude(theLabelValue))
    {
        NFmiLongitude lon(theLabelValue, false);
        theLabelValue = lon.Value();
    }
    NFmiString str(NFmiValueString::GetStringWithMaxDecimalsSmartWay(theLabelValue, 2));
    str += "∞";
    NFmiPoint labelPos(theEdgePoint);
    NFmiPoint move;
    int labelCharCount = str.GetLen();
    double minusSignMove = 0.;
    if(theLabelValue < 0)
        minusSignMove = -moveUnit;
    double longitudeXMove = moveUnit*labelCharCount*0.2;
    double latitudeRightXMove = moveUnit*labelCharCount*1.0;

    switch(theDirection)
    {
    case kBottom:
        if(fLatitudeLabelUsed)
            move = NFmiPoint(move.X(), move.Y() - moveUnit * 3);
        else
            move = NFmiPoint(move.X() + longitudeXMove, move.Y() - moveUnit*1.7);
        break;
    case kLeft:
        if(fLatitudeLabelUsed)
            move = NFmiPoint(move.X() + latitudeRightXMove*0.5, move.Y() - moveUnit / 2.);
        else
            move = NFmiPoint(move.X() + latitudeRightXMove*0.1, move.Y() - moveUnit);
        break;
    case kRight:
        if(fLatitudeLabelUsed)
            move = NFmiPoint(move.X() - latitudeRightXMove, move.Y() - moveUnit / 2.);
        else
            move = NFmiPoint(move.X() - latitudeRightXMove*0.7, move.Y() - moveUnit);
        break;
    case kTop:
        if(fLatitudeLabelUsed)
            move = NFmiPoint(move.X() - (minusSignMove + moveUnit*0.5), move.Y() + moveUnit / 0.9);
        else
            move = NFmiPoint(move.X() + longitudeXMove, move.Y() + moveUnit / 20.);
        break;
    }
    labelPos += move;
    theLineLabels.push_back(std::make_shared<NFmiText>(labelPos, str, false, nullptr, itsPrimaryLineLabelEnvi));
}

void NFmiProjectionCurvatureInfo::GetExtremeLatLonValues(const NFmiArea* theArea, double& theMinLon, double& theMaxLon, double& theMinLat, double& theMaxLat)
{
	NFmiPoint bl(theArea->BottomLeftLatLon());
	NFmiPoint br(theArea->BottomRightLatLon());
	NFmiPoint tr(theArea->TopRightLatLon());
	NFmiPoint tl(theArea->TopLeftLatLon());

	double xyTop = theArea->XYArea().Top();
	double xyBottom = theArea->XYArea().Bottom();
	double xyLeft = theArea->XYArea().Left();
	double xyRight = theArea->XYArea().Right();
	NFmiPoint bmXY((xyLeft + xyRight)/2., xyBottom);
	NFmiPoint bm(theArea->ToLatLon(bmXY));
	NFmiPoint tmXY((xyLeft + xyRight)/2., xyTop);
	NFmiPoint tm(theArea->ToLatLon(tmXY));
	NFmiPoint lmXY(xyLeft, (xyBottom + xyTop)/2.);
	NFmiPoint lm(theArea->ToLatLon(lmXY));
	NFmiPoint rmXY(xyRight, (xyBottom + xyTop)/2.);
	NFmiPoint rm(theArea->ToLatLon(rmXY));

	theMinLon = FmiMin(FmiMin(bl.X(), tl.X()), lm.X());
	theMaxLon = FmiMax(FmiMax(tr.X(), br.X()), rm.X());
	theMinLat = FmiMin(FmiMin(bl.Y(), br.Y()), bm.Y());
	theMaxLat = FmiMax(FmiMax(tr.Y(), tl.Y()), tm.Y());
	if(theMinLon > theMaxLon)
		throw runtime_error("NFmiProjectionCurvatureInfo::GetExtremeLatLonValues - minLon > maxLon");
	if(theMinLat > theMaxLat)
		throw runtime_error("NFmiProjectionCurvatureInfo::GetExtremeLatLonValues - minLat > maxLat");
}

// taulukko sis‰lt‰‰ lat/lon viivastojen sallitut etenemis nopeudet, eli mill‰ v‰lill‰ viivoja voi piirt‰‰
static const double legalAdvances[] = {90., 60., 30., 15., 10., 5., 2., 1., 0.5 /*, 0.2, 0.1 */ , 0}; // huom! 0 on taulukon lopetus merkki!!!
void NFmiProjectionCurvatureInfo::CalcLatitudeAdvances(double theMinLat, double theMaxLat, double* theLatitudeLineAdvance, double* theLatitudeAlongLineAdvance)
{
	double diff = theMaxLat - theMinLat;
	int divValue = 0;
	int lowerLimit = 0;
	int upperLimit = 0;
	GetLineDensityLimits(itsLineDensityModeLat, &lowerLimit, &upperLimit);
	double smoothnessFactor = GetLineSmoothnessFactor(itsLineSmoothness);

	for(int i=0; i<100; i++) // 100 on vain varmistus, ett‰ ei j‰‰d‰ ikiluuppiin, lopetus tulee viimeist‰‰n, kun taulukossa tullaan kohtaan, jonka arvo on 0
	{
		if(legalAdvances[i])
		{
			divValue = (int)(diff/legalAdvances[i]);
			if(divValue >= lowerLimit && divValue <= upperLimit)
			{
				*theLatitudeLineAdvance = legalAdvances[i];
				*theLatitudeAlongLineAdvance = diff/smoothnessFactor;
				break;
			}
			else if(divValue >= 0)
			{
				*theLatitudeLineAdvance = legalAdvances[i];
				*theLatitudeAlongLineAdvance = diff/smoothnessFactor;
			}
		}
		else
			break;
	}
}

void NFmiProjectionCurvatureInfo::CalcLongitudeAdvances(double theMinLon, double theMaxLon, double* theLongitudeLineAdvance, double* theLongitudeAlongLineAdvance)
{
	double diff = theMaxLon - theMinLon;
	int divValue = 0;
	int lowerLimit = 0;
	int upperLimit = 0;
	GetLineDensityLimits(itsLineDensityModeLon, &lowerLimit, &upperLimit);
	double smoothnessFactor = GetLineSmoothnessFactor(itsLineSmoothness);

	for(int i=0; i<100; i++) // 100 on vain varmistus, ett‰ ei j‰‰d‰ ikiluuppiin, lopetus tulee viimeist‰‰n, kun taulukossa tullaan kohtaan, jonka arvo on 0
	{
		if(legalAdvances[i])
		{
			divValue = (int)(diff/legalAdvances[i]);
			if(divValue > lowerLimit && divValue < upperLimit)
			{
				*theLongitudeLineAdvance = legalAdvances[i];
				*theLongitudeAlongLineAdvance = diff/smoothnessFactor;
				break;
			}
			else if(divValue >= 0)
			{
				*theLongitudeLineAdvance = legalAdvances[i];
				*theLongitudeAlongLineAdvance = diff/smoothnessFactor;
			}
		}
		else
			break;
	}
}

// t‰m‰ laskee sen, mill‰ askelv‰lill‰ projektio viivoja piirret‰‰n (riippuen k‰ytetyst‰ viivan tiheys arvosta)
void NFmiProjectionCurvatureInfo::GetLineDensityLimits(DensityMode theLineDensityMode, int* theLowerLimit, int* theUpperLimit)
{
	switch(theLineDensityMode)
	{
	case kScarce:
		*theLowerLimit = 1;
		*theUpperLimit = 100;
		break;
	case kDense:
		*theLowerLimit = 6;
		*theUpperLimit = 100;
		break;
//	case kCustomDensity:
//		break;
	case kModerate:
	default:
		*theLowerLimit = 3;
		*theUpperLimit = 100;
		break;
	}
}

double NFmiProjectionCurvatureInfo::GetLineSmoothnessFactor(LineSmoothness theLineSmoothness)
{
	double factor = 1.;
	switch(theLineSmoothness)
	{
	case kRude:
		factor = 15;
		break;
	case kSmooth:
		factor = 150;
		break;
//	case kCustomSmooth:
//		break;
	case kNormal:
	default:
		factor = 50;
		break;
	}
	return factor;
}
