//**********************************************************
// C++ Class Name : NFmiEditorControlPointManager 
// ---------------------------------------------------------
// Filetype: (SOURCE)
// Filepath: D:/projekti/GDPro/GDTemp/NFmiEditorControlPointManager.cpp 
// 
// 
// GDPro Properties 
// ---------------------------------------------------
//  - GD Symbol Type    : CLD_Class 
//  - GD Method         : UML ( 4.0 ) 
//  - GD System Name    : editori virityksi‰ 2000 syksy 
//  - GD View Type      : Class Diagram 
//  - GD View Name      : interpolation luokka 
// ---------------------------------------------------  
//  Author         : pietarin 
//  Creation Date  : Tues - Nov 7, 2000 
// 
//  Change Log     : 
// 
//**********************************************************
#ifdef _MSC_VER
#pragma warning(disable : 4786) // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta
#endif

#include "NFmiEditorControlPointManager.h"
#include "NFmiLocation.h"
#include "NFmiArea.h"
#include "NFmiSettings.h"
#include "NFmiFileString.h"
#include "NFmiFastQueryInfo.h"

#include <iterator>
#include <fstream>

using namespace std;
//--------------------------------------------------------
// Constructor/Destructor 
//--------------------------------------------------------
NFmiEditorControlPointManager::NFmiEditorControlPointManager (void)
:itsParamCount(0)
,itsParamIndex(-1)
,itsTimeDescriptor()
,itsParamBag()
,itsCPCount(0)
,itsCPIndex(-1)
,itsCPLocationVector(0)
,itsCPRelativeLocationVector(0)
,itsCPActivityVector(0)
,itsCPEnabledVector(0)
,itsCPChangeInTimeVector(0)
,itsCPChangeInTimeRelativeLocationMatrix()
,itsCPChangeInTimeLatLonLocationMatrix()
,itsCPMatrix()
,itsDummyChangeValueVector(200, 0.)
,itsDummyLatlon(NFmiPoint::gMissingLatlon)
,fMouseCaptured(false)
,itsArea(0)
,itsCPMovingInTimeHelpPoints(0)
,itsShowCPAllwaysOnTimeView(0)
,itsFilePath()
,itsName()
{
}
NFmiEditorControlPointManager::~NFmiEditorControlPointManager(void)
{
}

//--------------------------------------------------------
// Read 
//--------------------------------------------------------
std::istream& NFmiEditorControlPointManager::Read(istream& file)
{
	ReadCPs(file);
	return file;
}
//--------------------------------------------------------
// Write 
//--------------------------------------------------------
std::ostream& NFmiEditorControlPointManager::Write(ostream& file) const
{
	WriteCPs(file);

	return file;
}

// apufunktio, joka kirjoittaa ensin lukum‰‰r‰n tiedostoon kirjoittaa otukset jonoon erottimena space.
template<typename T>
static bool WriteVectorToFile(checkedVector<T>& theVector, ostream& file)
{
	file << theVector.size() << endl;
	std::copy(theVector.begin(), theVector.end(), ostream_iterator<T>(file, " "));
	file << endl;
	return !file.fail();
}

// apufunktio, joka lukee ensin tiedostosta lukum‰‰r‰n ja sitten niin monta otusta ja tallettaa ne vectoriin.
// vectorin koko muutetaan lukum‰‰r‰n suuruiseksi.
template<typename T>
static bool ReadVectorFromFile(checkedVector<T>& theVector, istream& file)
{
	int size = 0;
	file >> size;
	theVector.resize(size);
	for(int i=0; i<size; i++)
	  {
		T tmp;
		file >> tmp;
		theVector[i] = tmp;
	  }
	return !file.fail();
}

bool NFmiEditorControlPointManager::WriteBackup(ostream& file)
{
    if(itsTimeDescriptor.Size() == 0)
        return false;
	file << itsParamCount << endl;
	file << itsTimeDescriptor << endl;
	WriteCPs(file);
	WriteVectorToFile(itsCPActivityVector, file);
	WriteVectorToFile(itsCPEnabledVector, file);
	WriteVectorToFile(itsCPChangeInTimeVector, file);
	file << itsCPChangeInTimeLatLonLocationMatrix << endl;
	file << itsCPMatrix << endl;
	WriteVectorToFile(itsShowCPAllwaysOnTimeView, file);
	if(itsName.empty() == false)
	{
		NFmiString nameStr(itsName);
		file << nameStr << endl;
	}

	return !file.fail();
}

bool NFmiEditorControlPointManager::ReadBackup(istream& file)
{

	file >> itsParamCount;
	file >> itsTimeDescriptor;
	if(!ReadCPs(file))
		return false;

	if(!ReadVectorFromFile(itsCPActivityVector, file))
		return false;
	if(!ReadVectorFromFile(itsCPEnabledVector, file))
		return false;
	if(!ReadVectorFromFile(itsCPChangeInTimeVector, file))
		return false;
	file >> itsCPChangeInTimeLatLonLocationMatrix;
	if(file.fail())
		return false;
	file >> itsCPMatrix;
	if(file.fail())
		return false;
	if(!ReadVectorFromFile(itsShowCPAllwaysOnTimeView, file))
		return false;
	if(file.fail() == false)
	{ // yritet‰‰n lukea nimi lopusta, mutta jos ei onnistu, ei failaaa
		NFmiString nameStr;
		try
		{
			file >> nameStr;
		}
		catch(...)
		{}
		if(file.fail() == false)
			itsName = nameStr;
		return true;
	}

	return !file.fail();
}

bool NFmiEditorControlPointManager::ReadCPs(istream& file)
{
	int size = 0; 
	file >> size;
	itsCPLocationVector.resize(size);
	for(int i=0; i< size; i++)
		file >> itsCPLocationVector[i];
	itsCPCount = size;
	itsCPIndex = -1;

	return file.fail() == false;
}

bool NFmiEditorControlPointManager::StoreCPs(void) const
{
	if(itsFilePath.empty() == false)
	{
		std::ofstream out(FilePath().c_str(), std::ios::binary);
		if(out)
		{
			WriteCPs(out); // t‰ss‰ talletetaan vain lat-lon pisteet
			out.close();
			return out.fail() == false;
		}
	}
	return false;
}

bool NFmiEditorControlPointManager::WriteCPs(std::ostream& file) const
{
	// talletetaan vain sijainnit tiedostoon
	size_t size = itsCPLocationVector.size();
	file << size << std::endl;
	for(size_t i=0; i< size; i++)
		file << *(const_cast<NFmiPoint*>(&itsCPLocationVector[i])) << std::endl;

	return true;
}

//--------------------------------------------------------
// Init 
//--------------------------------------------------------
bool NFmiEditorControlPointManager::Init(const NFmiTimeDescriptor& theTimes, const NFmiParamBag& theParams, const std::string& theCPFileName, bool /* fKeepOldValues */ , bool fKeepOldCPs, int theMaxAllowedTimeCount)
{
	using namespace std;
	// itsCPLocationVector:in voisi olla jo kunnossa!!! (Read() ensin)
	if(!fKeepOldCPs)
	{
		FilePath(theCPFileName);
		ifstream in(theCPFileName.c_str(), std::ios::binary);
		if(in)
		{
			if(ReadCPs(in) == false)
				return false;
		}
		else
		{
			itsCPCount = 0;
			itsCPIndex = -1;
			itsCPLocationVector = checkedVector<NFmiPoint>();
		}
	}


	itsParamBag = theParams;
	itsTimeDescriptor = theTimes;
	itsParamCount = CalcParamSize(itsParamBag);
	itsParamIndex = -1;
	InitRelativeLocations();
	itsCPActivityVector.resize(itsCPCount, false);
	itsCPEnabledVector.resize(itsCPCount, true);
	itsCPChangeInTimeVector.resize(itsCPCount, false);
	int timeSize = itsTimeDescriptor.Size();
	if(timeSize > theMaxAllowedTimeCount)
		timeSize = theMaxAllowedTimeCount;
	itsCPChangeInTimeRelativeLocationMatrix.Resize(itsCPCount, timeSize, NFmiPoint());
	itsCPChangeInTimeLatLonLocationMatrix.Resize(itsCPCount, timeSize, NFmiPoint());
	itsCPMatrix.Resize(itsParamCount, itsCPCount, NFmiEditorControlPoint(timeSize));
	itsCPMovingInTimeHelpPoints.resize(itsCPCount, ThreePoints());
	itsShowCPAllwaysOnTimeView.resize(itsCPCount, false);

	// viel‰ lopuksi pit‰‰ laitta aikasarjamuutos vectorit oikean kokoisiksi varmuuden vuoksi,
	// eli k‰yd‰‰n l‰pi kaikki controllipisteet ja tehd‰‰n muutosarvo taululle resize
	for(size_t j=0; j < itsCPMatrix.NY(); j++) // huom! loopituksessa haku [parIndex][cpindex]
		for(size_t i=0; i < itsCPMatrix.NX(); i++)
			itsCPMatrix[i][j].Resize(timeSize);

	return true;
}

// t‰ll‰ asetetaan uudet CP pisteet managerille ja nollataa muuten olion tila
bool NFmiEditorControlPointManager::Init(const checkedVector<NFmiPoint> &newCPs)
{
	itsCPLocationVector = newCPs;
	itsCPCount = static_cast<int>(itsCPLocationVector.size());
	itsCPIndex = -1;

	itsParamIndex = -1;
	InitRelativeLocations();
	itsCPActivityVector.resize(itsCPCount, false);
	itsCPEnabledVector.resize(itsCPCount, true);
	itsCPChangeInTimeVector.resize(itsCPCount, false);
	int timeSize = itsTimeDescriptor.Size();
	itsCPChangeInTimeRelativeLocationMatrix.Resize(itsCPCount, timeSize, NFmiPoint());
	itsCPChangeInTimeLatLonLocationMatrix.Resize(itsCPCount, timeSize, NFmiPoint());
	itsCPMatrix.Resize(itsParamCount, itsCPCount, NFmiEditorControlPoint(timeSize));
	itsCPMovingInTimeHelpPoints.resize(itsCPCount, ThreePoints());
	itsShowCPAllwaysOnTimeView.resize(itsCPCount, false);

	// viel‰ lopuksi pit‰‰ laitta aikasarjamuutos vectorit oikean kokoisiksi varmuuden vuoksi,
	// eli k‰yd‰‰n l‰pi kaikki controllipisteet ja tehd‰‰n muutosarvo taululle resize
	for(size_t j=0; j < itsCPMatrix.NY(); j++) // huom! loopituksessa haku [parIndex][cpindex]
		for(size_t i=0; i < itsCPMatrix.NX(); i++)
			itsCPMatrix[i][j].Resize(timeSize);

	return true;
}

// T‰m‰ liittyy CPManagerin vaihtoon (kun niit‰ on setti k‰ytˆss‰).
//  Eli otetaan kaikki ulkoiset tekij‰t theOther:ista k‰yttˆˆn: 
// area, times, params, griddingProperties etc.
// Mutta itse CP-pisteisiin ja niiden arvoihin ei kosketa!
bool NFmiEditorControlPointManager::Init(const NFmiEditorControlPointManager &theOther)
{
	CPGriddingProperties(theOther.CPGriddingProperties());
	Area(theOther.Area());
	return Init(theOther.TimeDescriptor(), theOther.ParamBag(), "", true, true);
}

bool NFmiEditorControlPointManager::SetZoomedAreaStationsAsControlPoints(checkedVector<boost::shared_ptr<NFmiFastQueryInfo>> &theInfos, boost::shared_ptr<NFmiArea> &theArea)
{
    checkedVector<NFmiPoint> addedControlPoints;
    for(auto &fastInfo : theInfos)
    {
        AddZoomedAreaStationsToCPVector(fastInfo, theArea, addedControlPoints);
    }
    return Init(addedControlPoints);
}

bool NFmiEditorControlPointManager::SetZoomedAreaStationsAsControlPoints(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &theArea)
{
    checkedVector<NFmiPoint> addedControlPoints;
    AddZoomedAreaStationsToCPVector(theInfo, theArea, addedControlPoints);
    return Init(addedControlPoints);
}

void NFmiEditorControlPointManager::AddZoomedAreaStationsToCPVector(boost::shared_ptr<NFmiFastQueryInfo> &theInfo, boost::shared_ptr<NFmiArea> &theArea, checkedVector<NFmiPoint> &theAddedControlPointsInOut)
{
    if(!theInfo->IsGrid())
    {
        for(theInfo->ResetLocation(); theInfo->NextLocation(); )
        {
            const auto &latlon = theInfo->LatLonFast();
            if(theArea->IsInside(latlon))
            {
                theAddedControlPointsInOut.push_back(latlon);
            }
        }
    }
}

void NFmiEditorControlPointManager::FilePath(const std::string &newValue) 
{
	itsFilePath = newValue;
	NFmiFileString fileStr(newValue);
	itsName = fileStr.Header();
}

//--------------------------------------------------------
// Size 
//--------------------------------------------------------
// palautta CP lukum‰‰r‰n
int NFmiEditorControlPointManager::Size (void)
{
	return itsCPCount;
}
//--------------------------------------------------------
// ChangeValue 
//--------------------------------------------------------
// palautetaan sen CP:n arvo, mihin osoitetaan (ja on ilmeisesti aktiivinen)
double NFmiEditorControlPointManager::ChangeValue (void)
{
	return ChangeValue(itsCPIndex);
}
double NFmiEditorControlPointManager::ChangeValue (int theCPIndex)
{
	GetControlPoint(itsParamIndex, theCPIndex).TimeIndex(itsTimeDescriptor.Index());
	return GetControlPoint(itsParamIndex, theCPIndex).ChangeValue();
}

//--------------------------------------------------------
// ChangeValue 
//--------------------------------------------------------
// muutetaan arvo kaikille aktiivisille
void NFmiEditorControlPointManager::ChangeValue (double newValue)
{
	if(AreCPIndexiesGood(itsParamIndex, 0))
	{
		for(int i = itsCPCount - 1; i >= 0; i--)
		{
			if(itsCPActivityVector[i])
			{
				itsCPMatrix[itsParamIndex][i].TimeIndex(itsTimeDescriptor.Index());
				itsCPMatrix[itsParamIndex][i].ChangeValue(newValue); // miss‰ j‰rjestyksess‰ indeksit???
			}
		}
	}
}
// muutetaan arvo osoitetulle CP:lle
void NFmiEditorControlPointManager::CurrentCPChangeValue(double newValue)
{
	if(AreCPIndexiesGood(itsParamIndex, itsCPIndex))
	{
		itsCPMatrix[itsParamIndex][itsCPIndex].TimeIndex(itsTimeDescriptor.Index());
		itsCPMatrix[itsParamIndex][itsCPIndex].ChangeValue(newValue); // miss‰ j‰rjestyksess‰ indeksit???
	}
}

//--------------------------------------------------------
// Time 
//--------------------------------------------------------
bool NFmiEditorControlPointManager::Time (const NFmiMetTime& theTime)
{
	return itsTimeDescriptor.Time(theTime) == true;
}
//--------------------------------------------------------
// Time 
//--------------------------------------------------------
const NFmiMetTime& NFmiEditorControlPointManager::Time (void) const
{
	return itsTimeDescriptor.Time();
}
//--------------------------------------------------------
// Param 
//--------------------------------------------------------
bool NFmiEditorControlPointManager::Param (const NFmiDataIdent& theParam)
{
	int index = CalcParamIndex(theParam);
	if(index == -1)
		return false;
	itsParamIndex = index;
	return true;
}
//--------------------------------------------------------
// Param 
//--------------------------------------------------------
const NFmiDataIdent& NFmiEditorControlPointManager::Param (void) const
{
	return *itsParamBag.Current(false);
}
//--------------------------------------------------------
// LatLon 
//--------------------------------------------------------
void NFmiEditorControlPointManager::LatLon (const NFmiPoint& newLatLon, int theIndex)
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
		{
			if(itsCPChangeInTimeVector[itsCPIndex])
				itsCPChangeInTimeLatLonLocationMatrix[itsCPIndex][itsTimeDescriptor.Index()] = newLatLon;
			else
				itsCPLocationVector[itsCPIndex] = newLatLon;
		}
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
		{
			if(itsCPChangeInTimeVector[theIndex])
				itsCPChangeInTimeLatLonLocationMatrix[theIndex][itsTimeDescriptor.Index()] = newLatLon;
			else
				itsCPLocationVector[theIndex] = newLatLon;
		}
	}
}
//--------------------------------------------------------
// LatLon 
//--------------------------------------------------------
const NFmiPoint& NFmiEditorControlPointManager::LatLon(int theIndex) const
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
		{
			if(itsCPChangeInTimeVector[itsCPIndex])
				return itsCPChangeInTimeLatLonLocationMatrix[itsCPIndex][itsTimeDescriptor.Index()];
			else
				return itsCPLocationVector[itsCPIndex];
		}
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
		{
			if(itsCPChangeInTimeVector[theIndex])
				return itsCPChangeInTimeLatLonLocationMatrix[theIndex][itsTimeDescriptor.Index()];
			else
				return itsCPLocationVector[theIndex];
		}
	}
	return itsDummyLatlon;
}

void NFmiEditorControlPointManager::RelativePoint(const NFmiPoint& newPoint, int theIndex)
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
		{
			if(itsCPChangeInTimeVector[itsCPIndex])
				itsCPChangeInTimeRelativeLocationMatrix[itsCPIndex][itsTimeDescriptor.Index()] = newPoint;
			else
				itsCPRelativeLocationVector[itsCPIndex] = newPoint;
		}
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
		{
			if(itsCPChangeInTimeVector[theIndex])
				itsCPChangeInTimeRelativeLocationMatrix[theIndex][itsTimeDescriptor.Index()] = newPoint;
			else
				itsCPRelativeLocationVector[theIndex] = newPoint;
		}
	}
}

const NFmiPoint& NFmiEditorControlPointManager::RelativePoint(int theIndex) const
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
		{
			if(itsCPChangeInTimeVector[itsCPIndex])
				return itsCPChangeInTimeRelativeLocationMatrix[itsCPIndex][itsTimeDescriptor.Index()];
			else
				return itsCPRelativeLocationVector[itsCPIndex];
		}
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
		{
			if(itsCPChangeInTimeVector[theIndex])
				return itsCPChangeInTimeRelativeLocationMatrix[theIndex][itsTimeDescriptor.Index()];
			else
				return itsCPRelativeLocationVector[theIndex];
		}
	}
	return itsDummyLatlon;
}

void NFmiEditorControlPointManager::CPMovingInTimeHelpPoints(const ThreePoints& thePoints, int theIndex)
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
			itsCPMovingInTimeHelpPoints[itsCPIndex] = thePoints;
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
			itsCPMovingInTimeHelpPoints[theIndex] = thePoints;
	}
}

const NFmiEditorControlPointManager::ThreePoints& NFmiEditorControlPointManager::CPMovingInTimeHelpPoints(int theIndex) const
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
			return itsCPMovingInTimeHelpPoints[itsCPIndex];
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
			return itsCPMovingInTimeHelpPoints[theIndex];
	}

	static NFmiEditorControlPointManager::ThreePoints dummyPoints;
	return dummyPoints;
}

const NFmiPoint& NFmiEditorControlPointManager::ActiveCPLatLon (void) const
{
	for(int i = 0; i < itsCPCount; i++)
		if(itsCPActivityVector[i])
			return LatLon(i);
	return itsDummyLatlon;
}
//--------------------------------------------------------
// AddCP 
//--------------------------------------------------------
bool NFmiEditorControlPointManager::AddCP (const NFmiPoint& theLatLon)
{
	itsCPLocationVector.push_back(theLatLon);
	itsCPRelativeLocationVector.push_back(LatLonToRelative(theLatLon));
	itsCPActivityVector.push_back(false);
	itsCPEnabledVector.push_back(true);
	itsCPChangeInTimeVector.push_back(false);
	itsCPCount++;
	itsCPIndex = itsCPCount - 1; // laitetaan indeksi osoittamaan uutta pistett‰
	int timeSize = itsTimeDescriptor.Size();
	itsCPChangeInTimeRelativeLocationMatrix.Resize(itsCPCount, timeSize, NFmiPoint());
	itsCPChangeInTimeLatLonLocationMatrix.Resize(itsCPCount, timeSize, NFmiPoint());
	itsCPMatrix.Resize(itsParamCount, itsCPCount, NFmiEditorControlPoint(timeSize));
	itsCPMovingInTimeHelpPoints.resize(itsCPCount, ThreePoints());
	itsShowCPAllwaysOnTimeView.resize(itsCPCount, false);
	ActivateCP(itsCPIndex, true) ;

	return true;
}
//--------------------------------------------------------
// RemoveCP 
//--------------------------------------------------------
// Poistaa aktiivisen CP:n kaikilta parametreilta
bool NFmiEditorControlPointManager::RemoveCP (void)
{
	for(int i = itsCPCount - 1; i >= 0; i--)
	{
		if(itsCPActivityVector[i])
		{
			itsCPLocationVector.erase(itsCPLocationVector.begin()+i);
			if(itsCPRelativeLocationVector.size()) // t‰m‰ vektori saattaa olla viel‰ tyhj‰, jos ei ole dataa ladattuna editoriin, siksi pit‰‰ tarkistaa
				itsCPRelativeLocationVector.erase(itsCPRelativeLocationVector.begin()+i);
			itsCPActivityVector.erase(itsCPActivityVector.begin()+i);
			itsCPEnabledVector.erase(itsCPEnabledVector.begin()+i);
			itsCPChangeInTimeVector.erase(itsCPChangeInTimeVector.begin()+i);
			itsCPChangeInTimeRelativeLocationMatrix.RemoveColumn(i);
			itsCPChangeInTimeLatLonLocationMatrix.RemoveColumn(i);
			itsCPMatrix.RemoveRow(i); // en tied‰ miksi olen laittanut t‰h‰n cpindexin toisin p‰in kuin yll‰olevissa matriiseissa, mutta nyt pit‰‰ poistaa rivi eik‰ sarake
			itsCPMovingInTimeHelpPoints.erase(itsCPMovingInTimeHelpPoints.begin()+i);

			itsShowCPAllwaysOnTimeView.erase(itsShowCPAllwaysOnTimeView.begin()+i);
			itsCPCount--;
		}
	}

	if(itsCPCount == 0)
		itsCPIndex = -1;
	if(itsCPIndex >= itsCPCount)
		itsCPIndex = 0; // jos indeksi j‰i taulukon ulkopuolelle
	return true;
}
//--------------------------------------------------------
// EnableCP 
//--------------------------------------------------------
void NFmiEditorControlPointManager::EnableCP (bool newState)
{
	for(int i = 0; i < itsCPCount; i++)
		if(itsCPActivityVector[i])
			itsCPEnabledVector[i] = newState;
}
void NFmiEditorControlPointManager::EnableCP (int theCPIndex, bool newState)
{ // tehd‰‰n kun kerit‰‰n, pit‰‰ kai tehd‰ enable taulukko (eri kuin aktiivisuus!)
	if(AreCPIndexiesGood(0, theCPIndex))
		itsCPEnabledVector[theCPIndex] = newState;
}
bool NFmiEditorControlPointManager::IsEnabledCP(void)
{ // palauttaa 1. aktiivisen pisteen arvo
	if(AreCPIndexiesGood(0, itsCPIndex))
		return itsCPEnabledVector[itsCPIndex];
	else
		return false;
}

bool NFmiEditorControlPointManager::ShowCPAllwaysOnTimeView(void)
{
	if(AreCPIndexiesGood(0, itsCPIndex))
		return itsShowCPAllwaysOnTimeView[itsCPIndex];
	else
		return false;
}
void NFmiEditorControlPointManager::ShowCPAllwaysOnTimeView(bool newValue)
{
	if(AreCPIndexiesGood(0, itsCPIndex))
		itsShowCPAllwaysOnTimeView[itsCPIndex] = newValue;
}
void NFmiEditorControlPointManager::ShowAllCPsAllwaysOnTimeView(bool newValue)
{
	for(int i=0; i<itsCPCount; i++)
		itsShowCPAllwaysOnTimeView[i] = newValue;
}
//--------------------------------------------------------
// MoveCP 
//--------------------------------------------------------
// Siirt‰‰ 1. aktiivisen CP:n paikkaa.
void NFmiEditorControlPointManager::MoveCP (const NFmiPoint& newLatLon)
{
	for(int i = 0; i < itsCPCount; i++)
	{
		if(itsCPActivityVector[i])
		{
			LatLon(newLatLon, i);
			if(itsArea)
			{
				NFmiPoint p(itsArea->ToXY(newLatLon));
				p.Y(1 - p.Y());
				RelativePoint(p, i);
			}
			break;
		}
	}
}
//--------------------------------------------------------
// ActivateCP 
//--------------------------------------------------------
// Aktivoidaan CP:t‰ muokattaviksi, haetaan l‰hin annettua pistett‰
// keepOld:in avulla voidaan
// kontrolloida, lis‰t‰‰nkˆ aktiivinen jo aktiivisten listaan vai
// tuleeko siit‰ ainoa aktiivinen CP.
void NFmiEditorControlPointManager::ActivateCP(const NFmiPoint& theLatLon, bool newState)
{
    if(FindNearestCP(theLatLon))
    {
        ResetActivityVector();
        if(AreCPIndexiesGood(0, itsCPIndex))
            itsCPActivityVector[itsCPIndex] = newState;
    }
}

void NFmiEditorControlPointManager::ActivateCP(int theCPIndex, bool newState)
{
    ResetActivityVector();
    if(AreCPIndexiesGood(0, theCPIndex))
        itsCPActivityVector[theCPIndex] = newState;
}

void NFmiEditorControlPointManager::ResetActivityVector()
{
    itsCPActivityVector.swap(std::vector<bool>(itsCPActivityVector.size(), false));
}

bool NFmiEditorControlPointManager::IsActivateCP(void)
{
	if(AreCPIndexiesGood(0, itsCPIndex))
		return itsCPActivityVector[itsCPIndex];
	else
		return false;
}

bool NFmiEditorControlPointManager::IsNearestPointActivateCP(const NFmiPoint& theLatLon)
{
	int oldIndex = itsCPIndex;
	bool status = FindNearestCP(theLatLon);
	if(status)
	{
		if(!itsCPActivityVector[itsCPIndex])
			status = false;
	}
	itsCPIndex = oldIndex;
	return status;
}

bool NFmiEditorControlPointManager::FindNearestCP(const NFmiPoint& theLatLon, bool fChangeIndex)
{
	double minDist = 999999999.;
	double dist = minDist;
	bool status = false;
	NFmiLocation theLoc(theLatLon);
	for(int i = 0; i < itsCPCount; i++)
	{
		dist = theLoc.Distance(LatLon(i));
		if(minDist > dist)
		{
			minDist = dist;
			if(fChangeIndex)
				itsCPIndex = i;
			status = true;
		}
	}
	return status;
}

//--------------------------------------------------------
// ActivateAllCPs 
//--------------------------------------------------------
void NFmiEditorControlPointManager::ActivateAllCPs (bool newState)
{
	for(int i=0; i< itsCPCount; i++)
		itsCPActivityVector[i] = newState;
}
//--------------------------------------------------------
// ChangeValues 
//--------------------------------------------------------
// Palauttaa enabloitujen CP:n x ja y koordinaatit, vektorissa sek‰ niiden
// muutos arvot z-vektorissa ja lis‰ksi aktiivisten pisteiden lukum‰‰r‰.
// pit‰‰ tehd‰ enabled-vector!!!! Nyt palauttaa kaikkien CP:en arvot.
bool NFmiEditorControlPointManager::ChangeValues (std::vector<float>& xValues, std::vector<float>& yValues, std::vector<float>& zValues, int& theArraySize)
{
	bool status = false;
	theArraySize = 0;
	for(int i=0; i< itsCPCount; i++)
	{
		if(itsCPEnabledVector[i])
		{
			status = true;
			xValues.push_back(float(RelativePoint(i).X()));
			yValues.push_back(float(RelativePoint(i).Y()));
			
			zValues.push_back(float(ChangeValue(i)));
			theArraySize++;
		}
	}
	return status;
}

// palauttaa 1. aktiivisen CP muutos arvot currentille parametrille
checkedVector<double>& NFmiEditorControlPointManager::ActiveCPChangeValues(void)
{
	for(int i = 0; i < itsCPCount; i++)
		if(itsCPActivityVector[i])
			return GetControlPoint(itsParamIndex, i).ChangeValues();
	return itsDummyChangeValueVector;
}

bool NFmiEditorControlPointManager::AreCPIndexiesGood(int theParamIndex, int theCPIndex) const
{
	if(theParamIndex >= 0 && theParamIndex < itsCPMatrix.NX() && theCPIndex >= 0 && theCPIndex < itsCPMatrix.NY())
		return true;
	else 
		return false;
}

NFmiEditorControlPoint& NFmiEditorControlPointManager::GetControlPoint(int theParamIndex, int theCPIndex)
{
	if(AreCPIndexiesGood(theParamIndex, theCPIndex))
		return itsCPMatrix[theParamIndex][theCPIndex];
	else
		return itsDummyControlPoint;
}

// currentin CP:n muutos arvot
checkedVector<double>& NFmiEditorControlPointManager::CPChangeValues(void)
{
	return GetControlPoint(itsParamIndex, itsCPIndex).ChangeValues();
}

//--------------------------------------------------------
// ResetTime 
//--------------------------------------------------------
bool NFmiEditorControlPointManager::ResetTime (void)
{
	itsTimeDescriptor.Reset();
	return true;
}
//--------------------------------------------------------
// NextTime 
//--------------------------------------------------------
bool NFmiEditorControlPointManager::NextTime (void)
{
	return itsTimeDescriptor.Next();
}

bool NFmiEditorControlPointManager::ResetCP(void)
{
	itsCPIndex = -1;
	return true;
}

bool NFmiEditorControlPointManager::NextCP(void)
{
	itsCPIndex++;
	if(itsCPIndex >= itsCPCount)
		return false;
	else
		return true;
}

const int g_missingCpIndex = -1;

// Etsi ensimm‰inen aktiivinen CP-piste ja palauta sen indeksi.
// Jos aktiivista CP:t‰ ei ole, palauta g_missingCpIndex.
int NFmiEditorControlPointManager::GetActiveCpIndex() const
{
    for(auto cpIndex = 0; cpIndex < itsCPActivityVector.size(); cpIndex++)
    {
        if(itsCPActivityVector[cpIndex])
            return cpIndex;
    }
    return g_missingCpIndex;
}

bool NFmiEditorControlPointManager::ActivateNextCP()
{
    auto activeCpIndex = GetActiveCpIndex();
    if(activeCpIndex == g_missingCpIndex)
        return ActivateFirstCp(); // Yritet‰‰n aktivoida 1. CP-piste
    if(activeCpIndex >= itsCPCount - 1)
        return false; // Viimeinen CP-piste oli jo aktiivinen, j‰tet‰‰n homma siihen
    else 
    {
        // Deaktivoidaan lˆydetty piste, ja aktivoidaan sit‰ seuraava
        ActivateCP(activeCpIndex + 1, true);
        return true;
    }
}

bool NFmiEditorControlPointManager::ActivatePreviousCP()
{
    auto activeCpIndex = GetActiveCpIndex();
    if(activeCpIndex == g_missingCpIndex)
        return ActivateFirstCp(); // Yritet‰‰n aktivoida 1. CP-piste
    if(activeCpIndex <= 0)
        return false; // Viimeinen CP-piste oli jo aktiivinen, j‰tet‰‰n homma siihen
    else
    {
        // Deaktivoidaan lˆydetty piste, ja aktivoidaan sit‰ edellinen
        ActivateCP(activeCpIndex - 1, true);
        return true;
    }
}

bool NFmiEditorControlPointManager::ActivateFirstCp()
{
    if(itsCPCount)
    {
        ActivateCP(0, true);
        return true;
    }
    else
        return false;
}

// Calculate direction from point1 to point2.
// Result is in degrees (0 -> 360), 'north' is 0 and goes clockwise (east 90, south 180, west 270).
// If points are the same, result is 0.
static double CalcDirection(const NFmiPoint &point1, const NFmiPoint &point2)
{
    auto x = point2.X() - point1.X();
    auto y = point2.Y() - point1.Y();
    if(x == 0 && y == 0)
        return 0;
    auto origDirection = atan2(x, y) * 180 / kPii;
    if(origDirection < 0)
        return 360 + origDirection;
    else
        return origDirection;
}

static double GetAngleTowardsDirection(ControlPointAcceleratorActions direction)
{
    switch(direction)
    {
    case ControlPointAcceleratorActions::Up:
        return 0;
    case ControlPointAcceleratorActions::Down:
        return 180;
    case ControlPointAcceleratorActions::Left:
        return 270;
    default: // default on Right case
        return 90;
    }
}

// Lasketaan kerroin currentin CP pisteen suunnalle suhteessa haluttuun suuntaan.
// Jos suunta on t‰ydellinen, saadaan kertoimeksi tasan 1, mutta jos suunnissa on eroa, voi se kasvaa kohti 2:sta (jos l‰hestyt‰‰n 180 asteen heittoa)
static double CalcDirectionalFactor(const NFmiPoint &activeCpRelativePoint, const NFmiPoint &currentCpRelativePoint, ControlPointAcceleratorActions direction)
{
    const double angleDivider = 90.;
    auto directionAngle = ::GetAngleTowardsDirection(direction);
    auto directionToCurrentPoint = ::CalcDirection(activeCpRelativePoint, currentCpRelativePoint);
    if(std::fabs(directionAngle - directionToCurrentPoint) > 180.)
    {
        directionToCurrentPoint -= 360.; // Pit‰‰ varmistaa ett‰ erotusta ei lasketa ympyr‰n pitk‰‰ kaarta pitkin
        return 1. + (std::fabs(directionToCurrentPoint - directionAngle) / angleDivider);
    }
    else
        return 1. + (std::fabs(directionAngle - directionToCurrentPoint)/ angleDivider);
}

template<typename Comparison>
static int FindBestSuitedCpTowardsDirection(NFmiEditorControlPointManager &cpManager, int activeCpIndex, const Comparison &compare, ControlPointAcceleratorActions direction)
{
    const auto &activeCpRelativePoint = cpManager.RelativePoint(activeCpIndex);
    int closestCpIndex = g_missingCpIndex;
    double minRelativeDistance = 9999999999.;
    for(int cpIndex = 0; cpIndex < cpManager.CPCount(); cpIndex++)
    {
        if(cpIndex != activeCpIndex)
        {
            const auto &currentCpRelativePoint = cpManager.RelativePoint(cpIndex);
            if(compare(activeCpRelativePoint, currentCpRelativePoint))
            {
                // Et‰isyys laskuissa otetaan huomioon pisteiden et‰isyys ja 'hyv‰' suunta
                auto directionalFactor = ::CalcDirectionalFactor(activeCpRelativePoint, currentCpRelativePoint, direction);
                auto currentDistance = activeCpRelativePoint.Distance(currentCpRelativePoint) * directionalFactor;
                if(currentDistance < minRelativeDistance)
                {
                    closestCpIndex = cpIndex;
                    minRelativeDistance = currentDistance;
                }
            }
        }
    }
    return closestCpIndex;
}

static std::function<bool(const NFmiPoint &, const NFmiPoint &)> GetTowardsComparisonFunction(ControlPointAcceleratorActions direction)
{
    switch(direction)
    {
    case ControlPointAcceleratorActions::Up:
        return [](const NFmiPoint &activePoint, const NFmiPoint &currentPoint) {return activePoint.Y() < currentPoint.Y(); };
    case ControlPointAcceleratorActions::Down:
        return [](const NFmiPoint &activePoint, const NFmiPoint &currentPoint) {return activePoint.Y() > currentPoint.Y(); };
    case ControlPointAcceleratorActions::Left:
        return [](const NFmiPoint &activePoint, const NFmiPoint &currentPoint) {return activePoint.X() > currentPoint.X(); };
    default: // default on Right case
        return [](const NFmiPoint &activePoint, const NFmiPoint &currentPoint) {return activePoint.X() < currentPoint.X(); };
    }
}

bool NFmiEditorControlPointManager::ActivateCPToward(ControlPointAcceleratorActions direction)
{
    auto activeCpIndex = GetActiveCpIndex();
    if(activeCpIndex == g_missingCpIndex)
        return ActivateFirstCp(); // Yritet‰‰n aktivoida 1. CP-piste
    else
    {
        auto closestCpIndex = ::FindBestSuitedCpTowardsDirection(*this, activeCpIndex, ::GetTowardsComparisonFunction(direction), direction);
        if(closestCpIndex != g_missingCpIndex)
        {
            ActivateCP(closestCpIndex, true);
            return true;
        }
        else
            return false;
    }
}

bool NFmiEditorControlPointManager::MakeControlPointAcceleratorAction(ControlPointAcceleratorActions action)
{
    switch(action)
    {
    case ControlPointAcceleratorActions::Next:
        return ActivateNextCP();
    case ControlPointAcceleratorActions::Previous:
        return ActivatePreviousCP();
    default:
        return ActivateCPToward(action);
    }
}

//--------------------------------------------------------
// ClearAllChangeValues 
//--------------------------------------------------------
// Nollaa haluttujen CP:n muutosvektorit.
// method: 0=kaikki, 1=vain aktiivinen param, 2=aktiivinen param ja CP
void NFmiEditorControlPointManager::ClearAllChangeValues (int clearMethod, double theClearValue)
{
	switch(clearMethod)
	{
	case 0:
		ClearAllChangeValues(theClearValue);
		break;
	case 1:
		ClearIndexedParamChangeValues(theClearValue, itsParamIndex);
		break;
	case 2:
		ClearIndexedParamIndexedCPChangeValues(theClearValue, itsParamIndex, itsCPIndex);
		break;
	}
}
void NFmiEditorControlPointManager::ClearAllChangeValues(double newValue)
{
	for(int i = 0; i < itsParamCount; i++)
		ClearIndexedParamChangeValues(newValue, i);
}

void NFmiEditorControlPointManager::ClearIndexedParamChangeValues(double newValue, int paramIndex)
{
	for(int i = 0; i < itsCPCount; i++)
		ClearIndexedParamIndexedCPChangeValues(newValue, paramIndex, i);
}

void NFmiEditorControlPointManager::ClearIndexedParamIndexedCPChangeValues(double theValue, int theParamIndex, int theCPIndex)
{
	GetControlPoint(theParamIndex, theCPIndex).ClearValues(theValue);
}

//--------------------------------------------------------
// ControlPoint 
//--------------------------------------------------------
const NFmiEditorControlPoint& NFmiEditorControlPointManager::ControlPoint (void)
{
	return GetControlPoint(itsParamIndex, itsCPIndex); // 1. parInd ja 2. CPInd
}
//--------------------------------------------------------
// ParamBag 
//--------------------------------------------------------
const NFmiParamBag& NFmiEditorControlPointManager::ParamBag (void) const
{
	return itsParamBag;
}
//--------------------------------------------------------
// TimeBag 
//--------------------------------------------------------

const NFmiTimeDescriptor& NFmiEditorControlPointManager::TimeDescriptor(void) const
{
	return itsTimeDescriptor;
}

// laskee annetun parambagin kaikkien parametrien lukum‰‰r‰n (aliparametrit
// otetaan mukaa siten ett‰ ne lasketaan ,mutta niiden yliparametri j‰‰ laskuista)
// esim. TotalWind lis‰‰ lukum‰‰r‰‰ nyt 3:lla (totalwind sin‰ns‰ 0, mutta suunta,
// nopeus ja windvector = 3)
int NFmiEditorControlPointManager::CalcParamSize(NFmiParamBag& theParams)
{
	int size = 0;
	for(theParams.Reset(); theParams.Next(false); )
		size++;

	return size;
}
int NFmiEditorControlPointManager::CalcParamIndex(const NFmiDataIdent& theParam)
{
	int index = -1;
	for(itsParamBag.Reset(); itsParamBag.Next(false); )
	{
		index++;
		if(*itsParamBag.Current(false) == theParam)
			break;
	}
	if(index >= itsParamCount)
		index = -1;
	return index;
}

// laskee latlon pisteist‰ vastaavan relative point vektorin itsArean avulla
bool NFmiEditorControlPointManager::InitRelativeLocations(void)
{
	bool returnStatus = false;
	if(itsArea)
	{
		itsCPRelativeLocationVector.resize(itsCPCount);
		for(int i=0; i< itsCPCount; i++)
		{
			returnStatus = true;
			itsCPRelativeLocationVector[i] = LatLonToRelative(itsCPLocationVector[i]);
		}
	}
	return returnStatus;
}

NFmiPoint NFmiEditorControlPointManager::LatLonToRelative(const NFmiPoint& theLatLon)
{
	if(itsArea)
	{
		NFmiPoint p(itsArea->ToXY(theLatLon));
		p.Y(1 - p.Y());
		return p;
	}
	return NFmiPoint();
}

NFmiPoint NFmiEditorControlPointManager::RelativeToLatLon(const NFmiPoint& thePoint)
{
	if(itsArea)
	{
		NFmiPoint p(thePoint);
		p.Y(1 - p.Y()); // ?
		NFmiPoint latlon(itsArea->ToLatLon(p));
		return latlon;
	}
	return NFmiPoint();
}

// Palauttaa tiedon, siirtyykˆ annetun indeksin osoittama CP paikkaa ajassa?
// jos annettu indeksi on -1, k‰ytt‰‰ itsCPIndex:i‰
bool NFmiEditorControlPointManager::IsCPMovingInTime(int theIndex)
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
			return itsCPChangeInTimeVector[itsCPIndex];
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
			return itsCPChangeInTimeVector[theIndex];
	}

	return false;
}

// Asettaa tiedon, siirtyykˆ annetun indeksin osoittaman CP:n paikka ajassa?
// jos annettu indeksi on -1, k‰ytt‰‰ itsCPIndex:i‰
void NFmiEditorControlPointManager::CPMovingInTime(bool newState, int theIndex)
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
			itsCPChangeInTimeVector[itsCPIndex] = newState;
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
			itsCPChangeInTimeVector[theIndex] = newState;
	}
}

// sis‰‰n annetaan relatiiviset pisteet, pit‰‰ laske latlon pisteet itse!
// jos annettu indeksi on -1, k‰ytt‰‰ itsCPIndex:i‰
// P‰ivitt‰‰ samalla latlon-taulukon?
void NFmiEditorControlPointManager::SetInTimeMovingCPRelativeLocations(const checkedVector<NFmiPoint>& theRelativeVector, int theIndex)
{
	int timeSize = itsTimeDescriptor.Size();
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
		{
			for(int i = 0; i < timeSize; i++)
			{
				itsCPChangeInTimeRelativeLocationMatrix[itsCPIndex][i] = theRelativeVector[i];
				itsCPChangeInTimeLatLonLocationMatrix[itsCPIndex][i] = RelativeToLatLon(theRelativeVector[i]);
			}
		}
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
		{
			for(int i = 0; i < timeSize; i++)
			{
				itsCPChangeInTimeRelativeLocationMatrix[theIndex][i] = theRelativeVector[i];
				itsCPChangeInTimeLatLonLocationMatrix[theIndex][i] = RelativeToLatLon(theRelativeVector[i]);
			}
		}
	}
}

// jos annettu indeksi on -1, k‰ytt‰‰ itsCPIndex:i‰
const checkedVector<NFmiPoint>& NFmiEditorControlPointManager::GetInTimeMovingCPRelativeLocations(int theIndex) const
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
			return itsCPChangeInTimeRelativeLocationMatrix[itsCPIndex];
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
			return itsCPChangeInTimeRelativeLocationMatrix[theIndex];
	}

	static const checkedVector<NFmiPoint> dummyPoints;
	return dummyPoints;
}

// jos annettu indeksi on -1, k‰ytt‰‰ itsCPIndex:i‰
const NFmiPoint& NFmiEditorControlPointManager::StartingRelativeLocation(int theIndex) const
{
	if(theIndex == -1)
	{
		if(AreCPIndexiesGood(0, itsCPIndex))
			return itsCPRelativeLocationVector[itsCPIndex];
	}
	else
	{
		if(AreCPIndexiesGood(0, theIndex))
			return itsCPRelativeLocationVector[theIndex];
	}

	return itsDummyLatlon;
}



std::ostream& operator<<(std::ostream& os, const NFmiEditorControlPointManager& item)
{
	item.Write(os); 
	return os;
}
std::istream& operator>>(std::istream& is, NFmiEditorControlPointManager& item)
{
	item.Read(is); 
	return is;
}

// ==========================================================================================

NFmiCPGriddingProperties::NFmiCPGriddingProperties(bool isToolMasterAvailable)
:fToolMasterAvailable(isToolMasterAvailable)
,itsFunction(kFmiXuGriddingFastLocalFitCalc) // asetetaan defaulttina joku hyv‰ algoritmi, jos toolmaster ei ole k‰ytˆss‰, NFmiCPGriddingProperties::Function palauttaa kFmiMarkoGriddingFunction:in joka tapauksessa
,itsBaseNameSpace()
{}

void NFmiCPGriddingProperties::InitFromSettings(const std::string &theBaseNameSpace)
{
	itsBaseNameSpace = theBaseNameSpace;

	itsFunction = static_cast<FmiGriddingFunction>(NFmiSettings::Require<int>(std::string(itsBaseNameSpace + "::Function")));
}

void NFmiCPGriddingProperties::StoreToSettings(void)
{
	if(itsBaseNameSpace.empty() == false)
	{
		NFmiSettings::Set(std::string(itsBaseNameSpace + "::Function"), NFmiStringTools::Convert(itsFunction), true);
	}
	else
		throw std::runtime_error("Error in NFmiCPGriddingProperties::StoreToSettings, unable to store setting.");
}
