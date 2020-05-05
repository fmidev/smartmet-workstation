//© Ilmatieteenlaitos/Marko.
//Original 7.4.2004
//

#include "NFmiCrossSectionSystem.h"
#include "NFmiArea.h"
#include "NFmiQueryData.h"
#include "NFmiDrawParam.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiSettings.h"
#include "FmiCtrlKeys.h"
#include "NFmiDataStoringHelpers.h"
#include "SettingsFunctions.h"
#include "CtrlViewDocumentInterface.h"
#include "SpecialDesctopIndex.h"
#include "boost\math\special_functions\round.hpp"

double NFmiCrossSectionSystem::itsLatestVersionNumber = 1.0;
int NFmiCrossSectionSystem::itsNumberOfMainPoints = 3;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

NFmiCrossSectionSystem::NFmiCrossSectionSystem(int theMaxViewRowSize)
:itsCurrentVersionNumber(itsLatestVersionNumber)
,itsStartPointWinReg()
,itsMiddlePointWinReg()
,itsEndPointWinReg()
,itsMainXYPoints(3)
,itsMinorPoints()
,itsRouteTimes()
,itsActivatedMinorPointIndex(-1)
,itsWantedMinorPointCountWinReg(50)
,itsVerticalPointCountWinReg(50)
,fCrossSectionSystemActive(false)
,fUseTimeCrossSection(false)
,fUseRouteCrossSection(false)
,fUseObsAndForCrossSection(false)
,fTimeCrossSectionDirty(true)
,fCrossSectionViewOn(false)
,fShowHybridLevels(false)
,itsCrossSectionTimeControlTimeBag()
,itsCrossSectionDrawParam()
,itsMaxViewRowSize(theMaxViewRowSize)
,itsStartRowIndex(1)
,itsRowCount(1)
,itsCrossSectionMode(k2Point)
,itsStandardPressureLevelMode(kDontShow)
,itsCtrlViewDocumentInterface()
,itsObsForModeLocation()
,itsExtraRowInfos(theMaxViewRowSize)
,itsStartPointFillColor(1.f,0.f,0.f)
,itsEndPointFillColor(0.f,0.5f,1.f)
,itsMiddlePointFillColor(0.5f,0.9f,0.2f)
,itsAxisValuesDefaultWinReg(150, 1015)
,itsAxisValuesSpecialWinReg(1005, 1025)
,fShowParamWindowView(true)
,fShowTooltipOnCrossSectionView(true)
,itsTrueMapViewSizeInfo(CtrlViewUtils::kFmiCrossSectionView)
{
}

NFmiCrossSectionSystem::~NFmiCrossSectionSystem(void)
{
}

static NFmiPoint GetPointFromString(const std::string &thePointStr)
{
    std::stringstream input(thePointStr);
    NFmiPoint p;
    input >> p;
    return p;
}

static std::string GetStringFromPoint(const NFmiPoint &thePoint)
{
    std::stringstream output;
    output << thePoint;
    return output.str();
}

void NFmiCrossSectionSystem::InitializeFromSettings(const CrossSectionInitValuesWinReg &initValuesFromWinReg)
{
    StartPoint(::GetPointFromString(initValuesFromWinReg.itsStartPointStr));
    MiddlePoint(::GetPointFromString(initValuesFromWinReg.itsMiddlePointStr));
    EndPoint(::GetPointFromString(initValuesFromWinReg.itsEndPointStr));

	itsStartPointFillColor = SettingsFunctions::GetColorFromSettings("MetEditor::CrossSection::StartPointColor");
	itsMiddlePointFillColor = SettingsFunctions::GetColorFromSettings("MetEditor::CrossSection::MiddlePointColor");
	itsEndPointFillColor = SettingsFunctions::GetColorFromSettings("MetEditor::CrossSection::EndPointColor");

    itsAxisValuesDefaultWinReg = initValuesFromWinReg.itsAxisValuesDefault;
    itsAxisValuesSpecialWinReg = initValuesFromWinReg.itsAxisValuesSpecial;

    itsVerticalPointCountWinReg = initValuesFromWinReg.itsVerticalPointCount;
    itsWantedMinorPointCountWinReg = initValuesFromWinReg.itsWantedMinorPointCount;
}

// Tämä asettaa kaikki talletettavat asiat Windows rekistereihin.
void NFmiCrossSectionSystem::StoreSettings(CrossSectionInitValuesWinReg &storeValuesToWinReg)
{
    storeValuesToWinReg.itsStartPointStr = ::GetStringFromPoint(StartPoint());
    storeValuesToWinReg.itsMiddlePointStr = ::GetStringFromPoint(MiddlePoint());
    storeValuesToWinReg.itsEndPointStr = ::GetStringFromPoint(EndPoint());

    storeValuesToWinReg.itsAxisValuesDefault = itsAxisValuesDefaultWinReg;
    storeValuesToWinReg.itsAxisValuesSpecial = itsAxisValuesSpecialWinReg;

    storeValuesToWinReg.itsVerticalPointCount = itsVerticalPointCountWinReg;
    storeValuesToWinReg.itsWantedMinorPointCount = itsWantedMinorPointCountWinReg;
}


void NFmiCrossSectionSystem::StartPoint(const NFmiPoint &thePoint)
{
    itsStartPointWinReg = thePoint;
}

void NFmiCrossSectionSystem::MiddlePoint(const NFmiPoint &thePoint)
{
    itsMiddlePointWinReg = thePoint;
}

void NFmiCrossSectionSystem::EndPoint(const NFmiPoint &thePoint)
{
    itsEndPointWinReg = thePoint;
}

const NFmiPoint& NFmiCrossSectionSystem::StartPoint(void) const
{
    return itsStartPointWinReg;
}

const NFmiPoint& NFmiCrossSectionSystem::MiddlePoint(void) const
{
    return itsMiddlePointWinReg;
}
const NFmiPoint& NFmiCrossSectionSystem::EndPoint(void) const
{
    return itsEndPointWinReg;
}

const NFmiPoint& NFmiCrossSectionSystem::StartXYPoint(void) const
{
	return itsMainXYPoints[0];
}
void NFmiCrossSectionSystem::StartXYPoint(const NFmiPoint &thePoint)
{
	itsMainXYPoints[0] = thePoint;
}
const NFmiPoint& NFmiCrossSectionSystem::MiddleXYPoint(void) const
{
	return itsMainXYPoints[1];
}
void NFmiCrossSectionSystem::MiddleXYPoint(const NFmiPoint &thePoint)
{
	itsMainXYPoints[1] = thePoint;
}
const NFmiPoint& NFmiCrossSectionSystem::EndXYPoint(void) const
{
	return itsMainXYPoints[2];
}
void NFmiCrossSectionSystem::EndXYPoint(const NFmiPoint &thePoint)
{
	itsMainXYPoints[2] = thePoint;
}

checkedVector<NFmiPoint> NFmiCrossSectionSystem::MakeMainPointsVector(void) const
{
    checkedVector<NFmiPoint> points;
    points.push_back(itsStartPointWinReg);
    points.push_back(itsMiddlePointWinReg);
    points.push_back(itsEndPointWinReg);
    return points;
}

void NFmiCrossSectionSystem::SetMainPointsFromVector(const checkedVector<NFmiPoint> &theMainPointVector)
{
    if(theMainPointVector.size() > 0)
        itsStartPointWinReg = theMainPointVector[0];
    if(theMainPointVector.size() > 1)
        itsMiddlePointWinReg = theMainPointVector[1];
    if(theMainPointVector.size() > 2)
        itsEndPointWinReg = theMainPointVector[2];
}

// laskee  välipisteet pääpisteiden avulla käyttäen
// hyväkseen annettua areaa (josta laskuissa käytetään arean xy-maailmaa)
void NFmiCrossSectionSystem::CalcMinorPoints(const boost::shared_ptr<NFmiArea> &theArea)
{
	// alipisteitä on oltava väh. saman verran kuin pääpisteitä
	if(itsNumberOfMainPoints > static_cast<int>(itsWantedMinorPointCountWinReg))
	{
		itsMinorPoints = MakeMainPointsVector();
		return ;
	}
	checkedVector<NFmiPoint> xyPoints;
	int realMinorPointCount = itsWantedMinorPointCountWinReg;
	if(itsCrossSectionMode == k2Point)
	{
		xyPoints = checkedVector<NFmiPoint>(realMinorPointCount);
		// Poikkileikkausjana pitää jakaa ali pisteisiin xy-maailmassa
		NFmiPoint xy1(theArea->ToXY(StartPoint()));
		NFmiPoint xy2(theArea->ToXY(EndPoint()));
		double xDiff = (xy2.X() - xy1.X())/(realMinorPointCount - 1.0);
		double yDiff = (xy2.Y() - xy1.Y())/(realMinorPointCount - 1.0);
		for(int i=0 ; i < realMinorPointCount ; i++)
			xyPoints[i] = NFmiPoint(xy1.X() + i * xDiff, xy1.Y() + i * yDiff);
	}
	else
	{ // 3-piste moodissa tilanne toisella tavalla
		realMinorPointCount = itsWantedMinorPointCountWinReg;
		if(itsWantedMinorPointCountWinReg%2 == 0)
			realMinorPointCount++; // pitää olla pariton luku
		xyPoints = checkedVector<NFmiPoint>(realMinorPointCount);
		// Poikkileikkausjana pitää jakaa ali pisteisiin xy-maailmassa
		NFmiPoint xy1(theArea->ToXY(StartPoint()));
		NFmiPoint xy2(theArea->ToXY(MiddlePoint()));
		NFmiPoint xy3(theArea->ToXY(EndPoint()));
		double xDiff1 = (xy2.X() - xy1.X())/((realMinorPointCount - 1.0)/2.);
		double yDiff1 = (xy2.Y() - xy1.Y())/((realMinorPointCount - 1.0)/2.);
		int i=0;
		for(i=0 ; i < realMinorPointCount/2. ; i++)
			xyPoints[i] = NFmiPoint(xy1.X() + i * xDiff1, xy1.Y() + i * yDiff1);
		double xDiff2 = (xy3.X() - xy2.X())/((realMinorPointCount - 1.0)/2.);
		double yDiff2 = (xy3.Y() - xy2.Y())/((realMinorPointCount - 1.0)/2.);
		for(i=0 ; i < realMinorPointCount/2. ; i++)
			xyPoints[static_cast<int>(i+(realMinorPointCount-1)/2.)] = NFmiPoint(xy2.X() + i * xDiff2, xy2.Y() + i * yDiff2);
	}

	itsMinorPoints.resize(realMinorPointCount);
	checkedVector<NFmiPoint>::iterator xyIt = xyPoints.begin();
	checkedVector<NFmiPoint>::iterator minorIt = itsMinorPoints.begin();
	// konvertoidaan sitten lista xy-pisteitä halutuiksi latlon-pisteiksi
	for( ; minorIt != itsMinorPoints.end(); ++xyIt, ++minorIt)
		*minorIt = theArea->ToLatLon(*xyIt);

    itsCtrlViewDocumentInterface->UpdateCrossSectionMacroParamDataSize(); // Pitää päivittää macroParam laskuja varten poikkileikkaus hilakoot
}

// Jos annettu data (eli tämä on juuri ladattu metkun editoriin) on jonkinlaista level-dataa
// niin päivitystä tarvitaan. Mutta tyypin pitää olla editable, viewable tai hybrid, koska mm.
// havainnoissa on level-dataa ja päivitystä ei haluta
void NFmiCrossSectionSystem::CheckIfCrossSectionViewNeedsUpdate(NFmiQueryData *theData, NFmiInfoData::Type theType)
{
	if(theData)
	{
		if(theData->Info()->SizeLevels() > 1)
			if(theType == NFmiInfoData::kEditable || theType == NFmiInfoData::kViewable || theType == NFmiInfoData::kHybridData)
				fCrossSectionViewNeedsUpdate = true;
	}
}

void NFmiCrossSectionSystem::WantedMinorPointCount(int newValue)
{
	int oldValue = itsWantedMinorPointCountWinReg;
	if(newValue < itsNumberOfMainPoints)
		itsWantedMinorPointCountWinReg = itsNumberOfMainPoints;
	else
		itsWantedMinorPointCountWinReg = newValue;

	if(oldValue != itsWantedMinorPointCountWinReg)
        itsCtrlViewDocumentInterface->UpdateCrossSectionMacroParamDataSize();
}

int NFmiCrossSectionSystem::WantedMinorPointCount(void) const
{
    if(GetCrossMode() == k3Point)
    {
        if(itsMinorPoints.size() > 0)
            return static_cast<int>(itsMinorPoints.size());
    }
    return itsWantedMinorPointCountWinReg;
}

// aktivoi lähimmän ali-pisteen, mutta jos kyseinen piste jo aktivoitu, deaktivoi se
void NFmiCrossSectionSystem::ActivateNearestMinorPoint(const NFmiPoint &thePlace)
{
	int minDistIndex = -1;
	double tmpDist = 0; // etäisyydet metreissä
	double minDist = 9999999999.; // minimiksi aluksi joku iso luku
	checkedVector<NFmiPoint>::iterator it = itsMinorPoints.begin();
	NFmiLocation clickedLoc(thePlace);
	for( ; it != itsMinorPoints.end(); ++it)
	{
		NFmiLocation tmpLoc(*it);
		tmpDist = clickedLoc.Distance(tmpLoc);
		if(tmpDist < minDist)
		{
			minDist = tmpDist;
			minDistIndex = static_cast<int>(it - itsMinorPoints.begin());
		}
	}
	if(minDist < 1000 * 200) // minimi etäisyyden pitää olla alle halutun rajan (nyt 200km), että jokin piste aktivoidaan, muuten tehdään deaktivointi
	{
		if(itsActivatedMinorPointIndex == minDistIndex)
			itsActivatedMinorPointIndex = -1; // deaktivoidaan, jos jo valittuna
		else
			itsActivatedMinorPointIndex = minDistIndex;
	}
	else
		itsActivatedMinorPointIndex = -1; // deaktivoidaan, jos klikataan 'ohi'
}

const NFmiPoint& NFmiCrossSectionSystem::ActivatedMinorPoint(void)
{
	static NFmiPoint dummy;
	if(itsActivatedMinorPointIndex >= 0 && itsActivatedMinorPointIndex < static_cast<int>(itsMinorPoints.size()))
		return itsMinorPoints[itsActivatedMinorPointIndex];
	else
		return dummy; // dummy arvo, ennen tämän funktion kutsua pitäisi tarkistaa, löytyykö aktivoitua pistettä ollenkaan
}

void NFmiCrossSectionSystem::CrossSectionDrawParam(boost::shared_ptr<NFmiDrawParam> &newValue)
{
	itsCrossSectionDrawParam = newValue;
}

// Paluuarvo kertoo tarvitaanko näytön päivitystä
bool NFmiCrossSectionSystem::ChangeStartRowIndex(bool fMoveUp)
{
	int oldIndex = itsStartRowIndex;
	if(fMoveUp)
		itsStartRowIndex--;
	else
		itsStartRowIndex++;
	if(itsStartRowIndex < 1 && itsRowCount == 1)
		itsStartRowIndex = itsMaxViewRowSize;
	else if(itsStartRowIndex < 1)
		itsStartRowIndex = 1;
	if(itsStartRowIndex > itsMaxViewRowSize && itsRowCount == 1)
		itsStartRowIndex = 1;
	else if(itsStartRowIndex > itsMaxViewRowSize - itsRowCount + 1)
		itsStartRowIndex = itsMaxViewRowSize - itsRowCount + 1;
	return oldIndex != itsStartRowIndex;
}

// paluu arvo kertoo tarvitaanko näytön päivitystä
bool NFmiCrossSectionSystem::RowCount(int newValue)
{
	int oldIndex = itsStartRowIndex;
	int oldCount = itsRowCount;
	itsRowCount = newValue;
	if(itsRowCount < 1)
		itsRowCount = 1;
	if(itsRowCount > itsMaxViewRowSize)
		itsRowCount = itsMaxViewRowSize;
	// jos rivien määrä vaikuttaa nyt alku rivin indeksiin, se tarkistetaan tässä
	if(itsStartRowIndex > itsMaxViewRowSize - itsRowCount + 1)
		itsStartRowIndex = itsMaxViewRowSize - itsRowCount + 1;

	itsTrueMapViewSizeInfo.onViewGridSizeChange(NFmiPoint(1, itsRowCount), true);
	return (oldCount != itsRowCount) || (oldIndex != itsStartRowIndex);
}

void NFmiCrossSectionSystem::NextCrossSectionMode(void)
{
	switch(itsCrossSectionMode)
	{
	case k2Point:
		itsCrossSectionMode = k3Point;
		break;
	default:
		itsCrossSectionMode = k2Point;
		break;
	}
}

void NFmiCrossSectionSystem::NextStandardPressureLevelMode(void)
{
	switch(itsStandardPressureLevelMode)
	{
	case kDontShow:
		itsStandardPressureLevelMode = kShowUnder;
		break;
	case kShowUnder:
		itsStandardPressureLevelMode = kShowOver;
		break;
	default:
		itsStandardPressureLevelMode = kDontShow;
		break;
	}
}

bool NFmiCrossSectionSystem::IsViewVisible(int theIndex)
{
	if(theIndex >= itsStartRowIndex && theIndex < itsStartRowIndex + itsRowCount)
		return true;
	return false;
}

const NFmiTimeBag& NFmiCrossSectionSystem::CrossSectionTimeControlTimeBag(void) const
{
	return itsCrossSectionTimeControlTimeBag;
}

void NFmiCrossSectionSystem::CrossSectionTimeControlTimeBag(const NFmiTimeBag &newTimeBag)
{
	if(!(itsCrossSectionTimeControlTimeBag == newTimeBag))
	{
		fTimeCrossSectionDirty = true;
		itsCrossSectionTimeControlTimeBag = newTimeBag;
	}
}

void NFmiCrossSectionSystem::CalcRouteTimes(void)
{
	itsRouteTimes.clear();
	const checkedVector<NFmiPoint>& points = MinorPoints();
	int ssize = static_cast<int>(points.size());
	int maxDiffInMinutes = CrossSectionTimeControlTimeBag().LastTime().DifferenceInMinutes(CrossSectionTimeControlTimeBag().FirstTime());
	for(int i=0; i<ssize; i++)
	{
		NFmiMetTime pTime(itsCrossSectionTimeControlTimeBag.FirstTime());
        int changeByMinutes = boost::math::iround(i * maxDiffInMinutes / (ssize - 1.));
		pTime.ChangeByMinutes(changeByMinutes);
		itsRouteTimes.push_back(pTime);
	}
}

// kun poikkileikkaus näytössä on nyt 3 eri moodia, pitää se päätellä jotenkin kahden booleanin avulla
NFmiCrossSectionSystem::CrossMode NFmiCrossSectionSystem::GetCrossMode(void) const
{
	if(fUseObsAndForCrossSection)
		return kObsAndFor;
	else if(fUseRouteCrossSection)
		return kRoute; // route optio on dominoiva
	else if(fUseTimeCrossSection)
		return kTime;
	else
		return kNormal;
}

void NFmiCrossSectionSystem::VerticalPointCount(int newValue)
{
	int oldValue = itsVerticalPointCountWinReg;
	itsVerticalPointCountWinReg = newValue;
	if(itsVerticalPointCountWinReg < 3)
		itsVerticalPointCountWinReg = 3;
	if(oldValue != itsVerticalPointCountWinReg)
        itsCtrlViewDocumentInterface->UpdateCrossSectionMacroParamDataSize();
}

// vähän mousewheel käsittelyä tänne, koska tätä kutsutaan parista eri view-luokasta
bool NFmiCrossSectionSystem::MouseWheel(const NFmiPoint & /* thePlace */ , unsigned long theKey, short theDelta)
{
	if(theKey & CtrlViewUtils::kCtrlKey)
	{ // vertikaali pisteiden säätö
		if(theDelta > 0)
			VerticalPointCount(VerticalPointCount()+1); // lisätään yksi vert. piste
		else
			VerticalPointCount(VerticalPointCount()-1);  // vähennetään yksi vert. piste
	}
	else if(theKey & CtrlViewUtils::kShiftKey)
	{ // vaakapisteiden määrän säätö
		// 2-piste moodissa yksi piste lisää ja 3-piste moodissa 2 lisää (tai vähemmän)
		int changeValue = CrossSectionMode() == NFmiCrossSectionSystem::k2Point ? 1 : 2;
		if(theDelta > 0)
			WantedMinorPointCount(WantedMinorPointCount()+changeValue);
		else
			WantedMinorPointCount(WantedMinorPointCount()-changeValue);
	}
	else
	{
		bool status = false;
		if(theDelta > 0)
			status = ChangeStartRowIndex(true);
		else
			status = ChangeStartRowIndex(false);
		return status;
	}
	return true;
}

int NFmiCrossSectionSystem::LowestVisibleRowIndex(void)
{
	return itsStartRowIndex + itsRowCount - 1;
}

NFmiCrossSectionSystem::ExtraRowInfo& NFmiCrossSectionSystem::GetRowInfo(int theRowIndex)
{
	if(theRowIndex >= 0 && theRowIndex < static_cast<int>(itsExtraRowInfos.size()))
		return itsExtraRowInfos[theRowIndex];
	else
		throw std::runtime_error("Error in NFmiCrossSectionSystem::GetRowInfo, invalid row index given, error in application...");
}

static std::string GetCrossSectionViewPressureLevelsStr(const checkedVector<NFmiCrossSectionSystem::ExtraRowInfo> &theExtraRowInfos)
{
	std::string str;
	size_t ssize = theExtraRowInfos.size();
	for(size_t i = 0; i<ssize; i++)
	{
		if(i != 0)
			str += ",";

		str += NFmiStringTools::Convert<double>(theExtraRowInfos[i].itsLowerEndOfPressureAxis);
		str += ",";
		str += NFmiStringTools::Convert<double>(theExtraRowInfos[i].itsUpperEndOfPressureAxis);
	}

	return str;
}

static void SetCrossSectionViewPressureLevels(NFmiCrossSectionSystem &theCrossSectionSystem, const std::string &thePressureLevelsStr)
{
	std::vector<float> pressureLevels = NFmiStringTools::Split<std::vector<float> >(thePressureLevelsStr, ",");
	int maxSize = static_cast<int>(pressureLevels.size()/2); // näin moneen riviin limit löytyy maksimissaan
	for(int i=0; i<maxSize; i++)
	{
		if(i < theCrossSectionSystem.MaxViewRowSize())
		{
			float maxPressure = pressureLevels[i*2];
			float minPressure = pressureLevels[i*2+1];
			NFmiCrossSectionSystem::ExtraRowInfo &extraRowInfo = theCrossSectionSystem.GetRowInfo(i);
			extraRowInfo.itsLowerEndOfPressureAxis = maxPressure;
			extraRowInfo.itsUpperEndOfPressureAxis = minPressure;
		}
	}
}

void NFmiCrossSectionSystem::Write(std::ostream& os) const
{
	os << "// NFmiCrossSectionSystem::Write..." << std::endl;

	os << "// version number" << std::endl;
	itsCurrentVersionNumber = itsLatestVersionNumber; // aina kirjoitetaan viimeisellä versio numerolla
	os << itsCurrentVersionNumber << std::endl;

	os << "// Container<MainPoints>" << std::endl;
    NFmiDataStoringHelpers::WriteContainer(MakeMainPointsVector(), os, std::string(""));

	os << "// ActivatedMinorPointIndex + WantedMinorPointCount + VerticalPointCount" << std::endl;
	os << itsActivatedMinorPointIndex << " " << itsWantedMinorPointCountWinReg << " " << itsVerticalPointCountWinReg << std::endl;

	os << "// CrossSectionSystemActive + UseTimeCrossSection + UseRouteCrossSection" << std::endl;
	os << fCrossSectionSystemActive << " " << fUseTimeCrossSection << " " << fUseRouteCrossSection << std::endl;

	os << "// UseObsAndForCrossSection + TimeCrossSectionDirty + CrossSectionViewOn + ShowHybridLevels" << std::endl;
	os << fUseObsAndForCrossSection << " " << fTimeCrossSectionDirty << " " << fCrossSectionViewOn << " " << fShowHybridLevels << std::endl;

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	os << "// selected Route-Start-Time with utc hour + minute + day shift to current day" << std::endl;
	NFmiDataStoringHelpers::WriteTimeWithOffsets(usedViewMacroTime, CrossSectionTimeControlTimeBag().FirstTime(), os);

	os << "// selected Route-End-Time with utc hour + minute + day shift to current day" << std::endl;
	NFmiDataStoringHelpers::WriteTimeWithOffsets(usedViewMacroTime, CrossSectionTimeControlTimeBag().LastTime(), os);

	os << "// itsMaxViewRowSize + itsStartRowIndex + itsRowCount + itsCrossSectionMode  + itsStandardPressureLevelMode" << std::endl;
	os << itsMaxViewRowSize << " " << itsStartRowIndex << " " << itsRowCount << " " << itsCrossSectionMode  << " " << itsStandardPressureLevelMode << std::endl;

	os << "// ObsForModeLocation" << std::endl;
	os << itsObsForModeLocation;


	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	// Kun tulee uusia muuttujia, tee tähän extradatan täyttöä, jotta se saadaan talteen tiedopstoon siten että
	// edelliset versiot eivät mene solmuun vaikka on tullut uutta dataa.

	// *** 1. uusi tallennus itsCrossSectionTimeBag talletetaan aikasiirtoina ja siitä tehdään yksi aika-stringi ***
	std::string crossSectionTimeBagOffSetStr = NFmiDataStoringHelpers::GetTimeBagOffSetStr(usedViewMacroTime, CrossSectionTimeControlTimeBag());
	extraData.Add(crossSectionTimeBagOffSetStr);

	// *** 2. uusi tallennus jokaisen rivin paineasteikon alku ja loppu päät talletetaan ja siitä tehdään yksi stringi ***
	std::string crossSectionViewPressureLevelsStr = ::GetCrossSectionViewPressureLevelsStr(ExtraRowInfos());
	extraData.Add(crossSectionViewPressureLevelsStr);

	os << "// possible extra data" << std::endl;
	os << extraData;

	if(os.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Write failed");
}

void NFmiCrossSectionSystem::Read(std::istream& is)
{ // toivottavasti olet poistanut kommentit luettavasta streamista!!

	is >> itsCurrentVersionNumber;
	if(itsCurrentVersionNumber > itsLatestVersionNumber)
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed the version number war newer than program can handle.");

	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");
    checkedVector<NFmiPoint> mainPoints;
    NFmiDataStoringHelpers::ReadContainer(mainPoints, is);
	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");
    SetMainPointsFromVector(mainPoints);

	is >> itsActivatedMinorPointIndex >> itsWantedMinorPointCountWinReg >> itsVerticalPointCountWinReg;
	is >> fCrossSectionSystemActive >> fUseTimeCrossSection >> fUseRouteCrossSection;
	fCrossSectionViewNeedsUpdate = true;
	is >> fUseObsAndForCrossSection >> fTimeCrossSectionDirty >> fCrossSectionViewOn >> fShowHybridLevels;
	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");

	NFmiMetTime usedViewMacroTime = NFmiDataStoringHelpers::GetUsedViewMacroTime();
	NFmiMetTime dummyTime; // erilliset itsRouteStartTime ja itsRouteEndTime on poistettu käytöstä, luetaan vain dummy arvot pois
	NFmiDataStoringHelpers::ReadTimeWithOffsets(usedViewMacroTime, dummyTime, is);
	NFmiDataStoringHelpers::ReadTimeWithOffsets(usedViewMacroTime, dummyTime, is);

	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");
	int tmpValue1 = 0;
	int tmpValue2 = 0;
	is >> itsMaxViewRowSize >> itsStartRowIndex >> itsRowCount >> tmpValue1  >> tmpValue2;
	itsCrossSectionMode = static_cast<NFmiCrossSectionSystem::Mode>(tmpValue1);
	itsStandardPressureLevelMode = static_cast<NFmiCrossSectionSystem::PressureMode>(tmpValue2);

	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");
	is >> itsObsForModeLocation;

	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");
	NFmiDataStoringHelpers::NFmiExtraDataStorage extraData; // lopuksi vielä mahdollinen extra data
	is >> extraData;
	// Tässä sitten otetaaan extradatasta talteen uudet muuttujat, mitä on mahdollisesti tullut
	// eli jos uusia muutujia tai arvoja, käsittele tässä.

	// *** 1. uusi tallennus itsCrossSectionTimeBag talletetaan aikasiirtoina ja siitä tehdään yksi aika-stringi ***
	if(extraData.itsStringValues.size() >= 1)
	{
		CrossSectionTimeControlTimeBag(NFmiDataStoringHelpers::GetTimeBagOffSetFromStr(usedViewMacroTime, extraData.itsStringValues[0]));
	}
	// *** 2. uusi tallennus jokaisen rivin paineasteikon alku ja loppu päät talletetaan ja siitä tehdään yksi stringi ***
	if(extraData.itsStringValues.size() >= 2)
	{
		std::string crossSectionViewPressureLevelsStr = extraData.itsStringValues[1];
		::SetCrossSectionViewPressureLevels(*this, crossSectionViewPressureLevelsStr);
	}
	if(is.fail())
		throw std::runtime_error("NFmiCrossSectionSystem::Read failed");

	itsCurrentVersionNumber = itsLatestVersionNumber; // aina jatketaan viimeisellä versio numerolla
}

void NFmiCrossSectionSystem::Init(const NFmiCrossSectionSystem &theData)
{
    itsStartPointWinReg = theData.itsStartPointWinReg;
    itsMiddlePointWinReg = theData.itsMiddlePointWinReg;
    itsEndPointWinReg = theData.itsEndPointWinReg;
	itsActivatedMinorPointIndex = theData.itsActivatedMinorPointIndex;
	itsWantedMinorPointCountWinReg = theData.itsWantedMinorPointCountWinReg;
	itsVerticalPointCountWinReg = theData.itsVerticalPointCountWinReg;
	fCrossSectionSystemActive = theData.fCrossSectionSystemActive;
	fCrossSectionViewNeedsUpdate = true;
	fUseTimeCrossSection = theData.fUseTimeCrossSection;
	fUseRouteCrossSection = theData.fUseRouteCrossSection;
	fUseObsAndForCrossSection = theData.fUseObsAndForCrossSection;
	fTimeCrossSectionDirty = true;
	fCrossSectionViewOn = theData.fCrossSectionViewOn;
	fShowHybridLevels = theData.fShowHybridLevels;
	if(theData.itsCrossSectionTimeControlTimeBag.GetSize() > 1) // vanhoissa makroissa timebagin koko voi olla 1, sitä ei sijoiteta käyttöön
		itsCrossSectionTimeControlTimeBag = theData.itsCrossSectionTimeControlTimeBag;
	itsCrossSectionDrawParam = theData.itsCrossSectionDrawParam;

	itsMaxViewRowSize = theData.itsMaxViewRowSize;
	itsStartRowIndex = theData.itsStartRowIndex;
	itsRowCount = theData.itsRowCount;
	itsCrossSectionMode = theData.itsCrossSectionMode;
	itsStandardPressureLevelMode = theData.itsStandardPressureLevelMode;

	if(itsCtrlViewDocumentInterface == nullptr)
		itsCtrlViewDocumentInterface = theData.itsCtrlViewDocumentInterface;

	itsObsForModeLocation = theData.itsObsForModeLocation;
	fDragWholeCrossSection = false;
	itsExtraRowInfos = theData.itsExtraRowInfos;
}

void NFmiCrossSectionSystem::GetStartAndEndTimes(NFmiMetTime &theStartTimeOut, NFmiMetTime &theEndTimeOut, const NFmiMetTime &theCurrentTime, bool fShowTrajectories) const
{
	theStartTimeOut = NFmiMetTime::gMissingTime;
	theEndTimeOut = NFmiMetTime::gMissingTime;
	bool obsForMode = GetCrossMode() == NFmiCrossSectionSystem::kObsAndFor;
	if(GetCrossMode() == NFmiCrossSectionSystem::kTime || obsForMode || fShowTrajectories)
	{
		theStartTimeOut = CrossSectionTimeControlTimeBag().FirstTime();
		theEndTimeOut = CrossSectionTimeControlTimeBag().LastTime();
	}
	else if(GetCrossMode() == NFmiCrossSectionSystem::kNormal)
	{
		theStartTimeOut = theCurrentTime;
		theEndTimeOut = theCurrentTime;
	}
	else if(GetCrossMode() == NFmiCrossSectionSystem::kRoute)
	{
		const checkedVector<NFmiMetTime> &times = RouteTimes();
		if(times.size() > 0)
		{
			theStartTimeOut = times[0];
			theEndTimeOut = times[times.size()-1];
		}
	}
}

// Säädetään kaikki aikaa liittyvät jutut parametrina annettuun aikaan, että SmartMet säätyy ladattuun CaseStudy-dataan mahdollisimman hyvin.
void NFmiCrossSectionSystem::SetCaseStudyTimes(const NFmiMetTime &theCaseStudyTime)
{
	itsCrossSectionTimeControlTimeBag.SetNewStartTime(theCaseStudyTime);

	if(itsRouteTimes.size() > 0)
	{
		long diffInMinutes = theCaseStudyTime.DifferenceInMinutes(itsRouteTimes[0]);
		for(size_t i = 0; i < itsRouteTimes.size(); i++)
			itsRouteTimes[i].ChangeByMinutes(diffInMinutes);
	}
}

void NFmiCrossSectionSystem::SetCrossSectionDefaultAxisToFirstVisibleView(void)
{
	itsExtraRowInfos[itsStartRowIndex - 1].itsLowerEndOfPressureAxis = itsAxisValuesDefaultWinReg.itsLowerEndOfPressureAxis;
	itsExtraRowInfos[itsStartRowIndex - 1].itsUpperEndOfPressureAxis = itsAxisValuesDefaultWinReg.itsUpperEndOfPressureAxis;
}

void NFmiCrossSectionSystem::SetCrossSectionDefaultAxisAll(void)
{
	for(size_t i = 0; i < itsExtraRowInfos.size(); i++)
	{
		itsExtraRowInfos[i].itsLowerEndOfPressureAxis = itsAxisValuesDefaultWinReg.itsLowerEndOfPressureAxis;
		itsExtraRowInfos[i].itsUpperEndOfPressureAxis = itsAxisValuesDefaultWinReg.itsUpperEndOfPressureAxis;
	}
}

void NFmiCrossSectionSystem::SetCrossSectionSpecialAxisToFirstVisibleView(void)
{
	itsExtraRowInfos[itsStartRowIndex - 1].itsLowerEndOfPressureAxis = itsAxisValuesSpecialWinReg.itsLowerEndOfPressureAxis;
	itsExtraRowInfos[itsStartRowIndex - 1].itsUpperEndOfPressureAxis = itsAxisValuesSpecialWinReg.itsUpperEndOfPressureAxis;
}

void NFmiCrossSectionSystem::SetCrossSectionSpecialAxisAll(void)
{
	for(size_t i = 0; i < itsExtraRowInfos.size(); i++)
	{
		itsExtraRowInfos[i].itsLowerEndOfPressureAxis = itsAxisValuesSpecialWinReg.itsLowerEndOfPressureAxis;
		itsExtraRowInfos[i].itsUpperEndOfPressureAxis = itsAxisValuesSpecialWinReg.itsUpperEndOfPressureAxis;
	}
}

// Ottaa 1. näkyvän rivin akseli arvot ja laittaa ne default-akseli arvoiksi.
// Talettaa samalla myös settings arvot ja tekee NFmiSettings -save:n levylle.
void NFmiCrossSectionSystem::SaveCrossSectionDefaultAxisValues(void)
{
	// itsStartRowIndex - 1, koska itsStartRowIndex alkaa 1:stä ja vektorin indeksit 0:sta
	itsAxisValuesDefaultWinReg = itsExtraRowInfos[itsStartRowIndex - 1];
}

// Ottaa 1. näkyvän rivin akseli arvot ja laittaa ne special-akseli arvoiksi.
// Talettaa samalla myös settings arvot ja tekee NFmiSettings -save:n levylle.
void NFmiCrossSectionSystem::SaveCrossSectionSpecialAxisValues(void)
{
	// itsStartRowIndex - 1, koska itsStartRowIndex alkaa 1:stä ja vektorin indeksit 0:sta
	itsAxisValuesSpecialWinReg = itsExtraRowInfos[itsStartRowIndex - 1];
}
